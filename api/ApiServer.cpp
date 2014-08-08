#include "ApiServer.h"

#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/Http/ResponseContinuation>

#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>

#include <time.h>
#include "json.h"

#include "PeersHandler.h"
#include "IdentityHandler.h"
#include "WallHandler.h"
#include "ServiceControlHandler.h"
#include <retroshare/rsservicecontrol.h>
#include "rswall.h"
#include "JsonStream.h"

/*
data types in json       http://json.org/
string (utf-8 unicode)
number (int and float)
object (key value pairs, key must be a string)
true
false
null

data types in lua        http://www.lua.org/pil/2.html
nil
boolean
number  (double)
string  (8-bit)
table   (key value pairs, keys can be anything except nil)

data types in QML       http://qt-project.org/doc/qt-5/qtqml-typesystem-basictypes.html
bool
string
real/double
int
list
object types?

QML has many more types with special meaning like date


C++ delivers
std::string
bool
int
(double? i don't know)
enum
bitflags
raw binary data

objects
std::vector
std::list

different types of ids/hashes
-> convert to/from string with a generic operator
-> the operator signals ok/fail to the stream


*/

/*

data types to handle:
- bool
- string
- bitflags
- enums

containers:
- arrays: collection of objects or values without name, usually of the same type
- objects: objects and values with names

careful: the json lib has many asserts, so retroshare will stop if the smalles thing goes wrong
-> check type of json before usage

there are two possible implementations:
- with a virtual base class for the serialisation targets
  - better documentation of the interface
- with templates

*/

// other http server library: libmicrohttpd

/*

the general idea is:
want output in many different formats, while the retrival of the source data is always the same

get, put

ressource adress like
.org.retroshare.api.peers

generic router from adress to the ressource handler object

data formats for input and output:
- json
- lua
- QObject
- maybe just a typed c++ object

rest inspired / resource based interface
- have resources with adresses
- requests:
  - put
  - get
- request has parameters
- response:
  - returncode:
    - ok
    - not modified
    - error
  - data = object or list of objects

want to have a typesafe interface at the top?
probably not, a system with generic return values is enough
this interface is for scripting languages which don't have types

the interface at the top should look like this:
template <class RequestDataFormatT, class ResponseDataFormatT>
processRequest(const RequestMeta& req, const RequestDataFormatT& reqData,
               ResponseMeta& respMeta, ResponseDataFormatT& respData);

idea: pass all the interfaces to the retroshare core to this function,
or have this function as part of an object

the processor then applies all members of the request and response data to the data format like this:
reqData << "member1" << member1
        << "member2" << member2 ... ;

these operators have to be implemented for common things like boolean, int, std::string, std::vector, std::list ...
request data gets only deserialised
response data gets only serialised

response and request meta contains things like resource address, method and additional parameters

want generic resource caching mechanism
- on first request a request handler is created
- request handler is stored with its input data
- if a request handler for a given resource adress and parameters exists
  then the request handler is asked if the result is still valid
  if yes the result from the existing handler is used
- request handler gets deleted after timeout
- can have two types of resource handlers: static handlers and dynamic handlers
  - static handlers don't get deleted, because they don't contain result data
  - dynamic handlers contain result data, and thus get deleted after a while

it is even possible to implement a resource-changed check at the highest level
this allows to compute everything on the server side and only send chanes to the client
the different resource providers don't have to implement a resource changed check then
a top level change detector will poll them
of course this does not wokr with a deep resource tree with millions of nodes

for this we have the dynamic handlers,
they are created on demand and know how to listen for changes which affect them

*/

// new_api is the old api now
// the new new api namespace is called resource_api
namespace new_api{
/*
{
    "m1": [1, 2, 3],
    "m2": [{"p1":1, "p2":2},{"p1":1, "p2":2}]
}
*/

class Request
{
public:
    std::string mMethod; // "GET" or "PUT"
    std::string mAdress; // for example "org.retrohare.api.peers"
    std::string mStateCode; // if this code is present, then don't return a full result if the data did not change
};

class Response
{
public:
    int mReturnCode; // something like "200 OK", "304 Not Modified", "404 Not Found"
    std::string mStateCode;
    std::string mDebugString; // a humand readable error message
};


class JsonResponse
{
public:
    class Array: public json::Array {};
    class Object: public json::Object {};

    void operator=(const Array& array)
    {
        mRootType = ROOT_TYPE_ARRAY;
        mArray = array;
    }
    void operator=(const Object& object)
    {
        mRootType = ROOT_TYPE_OBJECT;
        mObject = object;
    }
    std::string toStdString()
    {
        if(mRootType == ROOT_TYPE_ARRAY)
        {
            return json::Serialize(mArray);
        }
        else if(mRootType == ROOT_TYPE_OBJECT)
        {
            return json::Serialize(mObject);
        }
        else
        {
            return "";
        }
    }

    enum RootType { ROOT_TYPE_UNDEFINED, ROOT_TYPE_ARRAY, ROOT_TYPE_OBJECT };
    RootType mRootType;
    Array mArray;
    Object mObject;
};

class PeersHandler
{
public:
    PeersHandler(RsPeers* peers): mPeers(peers) {}

    template <class InputT, class OutputT>
    void handleRequest(Request& req, /*InputT& reqData,*/ Response& resp, OutputT& respData)
    {
        if(req.mMethod == "GET")
        {
            typename OutputT::Array result;
            std::list<RsPeerId> sslIds;
            mPeers->getFriendList(sslIds);
            for(std::list<RsPeerId>::iterator lit = sslIds.begin(); lit != sslIds.end(); lit++)
            {
                typename OutputT::Object peer;
                peer["name"] = mPeers->getPeerName(*lit);
                peer["online"] = mPeers->isOnline(*lit);
                result.push_back(peer);
            }
            respData = result;
            resp.mStateCode = 200;
        }
    }

    RsPeers* mPeers;
};

class ChatlobbiesHandler
{
public:
    ChatlobbiesHandler(RsMsgs* msgs): mMsgs(msgs) {}

    template <class InputT, class OutputT>
    void handleRequest(Request& req, InputT& reqData, Response& resp, OutputT& respData)
    {
        if(req.mMethod == "GET")
        {
            typename OutputT::Array result;
            // subscribed lobbies
            std::list<ChatLobbyInfo> slobbies;
            mMsgs->getChatLobbyList(slobbies);
            for(std::list<ChatLobbyInfo>::iterator lit = slobbies.begin(); lit != slobbies.end(); lit++)
            {
                typename OutputT::Object lobby;
                ChatLobbyInfo& lobbyRecord = *lit;
                lobby["name"] = lobbyRecord.lobby_name;
                RsPeerId pid;
                mMsgs->getVirtualPeerId(lobbyRecord.lobby_id, pid);
                lobby["id"] = pid.toStdString();
                lobby["subscribed"] = true;
                result.push_back(lobby);
            }
            // unsubscirbed lobbies
            std::vector<VisibleChatLobbyRecord> ulobbies;
            mMsgs->getListOfNearbyChatLobbies(ulobbies);
            for(std::vector<VisibleChatLobbyRecord>::iterator vit = ulobbies.begin(); vit != ulobbies.end(); vit++)
            {
                typename OutputT::Object lobby;
                VisibleChatLobbyRecord& lobbyRecord = *vit;
                lobby["name"] = lobbyRecord.lobby_name;
                RsPeerId pid;
                mMsgs->getVirtualPeerId(lobbyRecord.lobby_id, pid);
                lobby["id"] = pid.toStdString();
                lobby["subscribed"] = false;
                result.push_back(lobby);
            }
            respData = result;
        }
        else if(req.mMethod == "PUT")
        {
            RsPeerId id = RsPeerId(req.mAdress.substr(1));

            if(!id.isNull() && reqData.HasKey("msg"))
            {
                // for now can send only id as message
                mMsgs->sendPrivateChat(id, reqData["msg"]);
            }
        }
    }

    RsMsgs* mMsgs;
};

ApiServer::ApiServer(const RsPlugInInterfaces &ifaces):
    WResource(), ifaces(ifaces)
{
    setDispositionType(Inline);
}

void ApiServer::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response)
{
    std::cerr << "TestRessource::handleRequest() pathInfo=" << request.pathInfo() << std::endl;

    PeersHandler peersHandler(ifaces.mPeers);
    ChatlobbiesHandler chatlobbiesHandler(ifaces.mMsgs);

    std::string path = request.pathInfo();
    std::string path1;
    std::string path2;
    size_t separator;
    separator = path.find("/", 1);
    if(separator != std::string::npos)
    {
        path1 = path.substr(0, separator);
        path2 = path.substr(separator);
    }
    else
    {
        path1 = path;
    }

    response.setMimeType("text/plain");
    Request req; req.mMethod = request.method(); req.mAdress = path2;
    Response resp;
    JsonResponse responseData;

    std::istreambuf_iterator<char> eos;
    std::string reqDataStr(std::istreambuf_iterator<char>(request.in()), eos);

    json::Value reqData = json::Deserialize(reqDataStr);


    if(path1 == "/peers")
    {
        peersHandler.handleRequest<JsonResponse>(req, resp, responseData);
    }
    else if(path1 == "/chatlobbies")
    {
        chatlobbiesHandler.handleRequest<json::Value, JsonResponse>(req, reqData, resp, responseData);
    }
    else if(path1 == "/v2")
    {
        resource_api::JsonStream instream;
        instream.setJsonString(reqDataStr);
        resource_api::JsonStream outstream;
        resource_api::Request req(instream);
        resource_api::Response resp(outstream);

        if(request.method() == "GET")
        {
            req.mMethod = resource_api::Request::GET;
        }
        else if(request.method() == "POST")
        {
            req.mMethod = resource_api::Request::POST;
        }
        else if(request.method() == "DELETE")
        {
            req.mMethod = resource_api::Request::DELETE_AA;
        }

        std::stack<std::string> stack;
        std::string str;
        for(std::string::reverse_iterator sit = path2.rbegin(); sit != path2.rend(); sit++)
        {
            if((*sit) != '/')
            {
                // add to front because we are traveling in reverse order
                str = *sit + str;
            }
            else
            {
                if(str != "")
                {
                    stack.push(str);
                    str.clear();
                }
            }
        }
        if(str != "")
        {
            stack.push(str);
        }
        req.mPath = stack;


        // pop the part wich gets consumed by this class
        req.mPath.pop();

        if(!stack.empty() && stack.top() == "help")
        {
            resource_api::PeersHandler peers_handler(ifaces.mPeers, ifaces.mMsgs);
            resource_api::IdentityHandler identity_handler(ifaces.mIdentity);
            resource_api::WallHandler wall_handler(RsWall::rsWall, ifaces.mIdentity);
            resource_api::ServiceControlHandler control_handler(rsServiceControl);
            response.out() <<
                              "Retroshare JSON over http interface\n"
                              "===================================\n"
                              "available subresources:\n"
                              "/peers\n"
                              "/identity\n"
                              "/wall\n"
                              "/servicecontrol\n"
                              "\n"
                              "/peers\n"
                              "------\n"
                              + peers_handler.help() +
                              "\n"
                              "/identity\n"
                              "---------\n"
                              + identity_handler.help() +
                              "\n"
                              "/wall\n"
                              "-----\n"
                              + wall_handler.help() +
                              "\n"
                              "/servicecontrol\n"
                              "---------------\n"
                              + control_handler.help() +
                              "\n"
                              ;
        }
        else if(!stack.empty() && stack.top() == "peers")
        {
            resource_api::PeersHandler handler(ifaces.mPeers, ifaces.mMsgs);
            handler.handleRequest(req, resp);
        }
        else if(!stack.empty() && stack.top() == "identity")
        {
            resource_api::IdentityHandler handler(ifaces.mIdentity);
            handler.handleRequest(req, resp);
        }
        else if(!stack.empty() && stack.top() == "wall")
        {
            resource_api::WallHandler handler(RsWall::rsWall, ifaces.mIdentity);
            handler.handleRequest(req, resp);
        }
        else if(!stack.empty() && stack.top() == "servicecontrol")
        {
            resource_api::ServiceControlHandler handler(rsServiceControl);
            handler.handleRequest(req, resp);
        }

        response.out() << outstream.getJsonString();


        // this does not work, the it gives compile error on addResourceHandler
        /*
        resource_api::ResourceRouter router;

        resource_api::PeersHandler peers_handler(ifaces.mPeers, ifaces.mMsgs);
        router.addResourceHandler("peers", &peers_handler, &resource_api::PeersHandler::handleRequest);

        resource_api::IdentityHandler identity_handler(ifaces.mIdentity);
        router.addResourceHandler("identity", &identity_handler, &resource_api::IdentityHandler::handleRequest);

        resource_api::WallHandler wall_handler(RsWall::rsWall, ifaces.mIdentity);
        router.addResourceHandler("wall", &wall_handler, &resource_api::WallHandler::handleRequest);

        resource_api::ServiceControlHandler control_handler(rsServiceControl);// todo: maybe add service control to plugin ifaces
        router.addResourceHandler("servicecontrol", &control_handler, &resource_api::ServiceControlHandler::handleRequest);

        router.handleRequest(req, resp);
        response.out() << outstream.getJsonString();
        */
    }
    response.out() << responseData.toStdString();

/*
    if(request.pathInfo() == "/peers")
    {
        response.setMimeType("text/plain");
        json::Array result;
        std::list<RsPeerId> sslIds;
        ifaces.mPeers->getFriendList(sslIds);
        for(std::list<RsPeerId>::iterator lit = sslIds.begin(); lit != sslIds.end(); lit++)
        {
            json::Object peer;
            peer["name"] = ifaces.mPeers->getPeerName(*lit);
            peer["online"] = ifaces.mPeers->isOnline(*lit);
            result.push_back(peer);
        }
        response.out() << json::Serialize(result) << std::endl;
    }
    else if(request.pathInfo() == "/chatlobbies")
    {
        response.setMimeType("text/plain");
        json::Array result;
        std::vector<VisibleChatLobbyRecord> lobbies;
        ifaces.mMsgs->getListOfNearbyChatLobbies(lobbies);
        for(std::vector<VisibleChatLobbyRecord>::iterator vit = lobbies.begin(); vit != lobbies.end(); vit++)
        {
            json::Object lobby;
            VisibleChatLobbyRecord& lobbyRecord = *vit;
            lobby["name"] = lobbyRecord.lobby_name;
            result.push_back(lobby);
        }
        response.out() << json::Serialize(result) << std::endl;
    }
    */

    /*
    if(request.pathInfo() == "/wait")
    {
        Sleep(3000);
    }

    Wt::Http::ResponseContinuation* continuation = request.continuation();

    if (continuation == NULL){
        response.setMimeType("text/plain");
        response.out() << request.pathInfo() << std::endl;
    }

    int last = 0;
    if(continuation){
        last = boost::any_cast<int>(continuation->data());
    }
    last++;
    response.out() << "Data item " << last << std::endl;

    // if we have not yet finished, create a continuation to serve more
    if (last < 10) {
        continuation = response.createContinuation();
        // remember what to do next
        continuation->setData(last);
    }
    */
}



// too complicated

/*
class Request
{
public:
    std::string mMethod; // "GET" or "PUT"
    std::string mAdress; // for example "org.retrohare.api.peers"
    std::string mStateCode; // if this code is present, then don't return a full result if the data did not change
};

class Response
{
public:
    int mReturnCode; // something like "200 OK", "304 Not Modified", "404 Not Found"
    std::string mStateCode;
    std::string mDebugString; // a humand readable error message
};

template <class ValueT>
class KeyValueReference
{
public:
    KeyValueReference(const std::string& key, const ValueT& value): key(key), value(value){}
    const std::string& key;
    const ValueT& value;
};

template <class ValueT>
KeyValueReference<T> makeKeyValueReference(const std::string& k, const ValueT& v)
{
    return KeyValueReference<ValueT>(k, v);
}

// markers
class BEGIN_OBJECT {};
class END_OBJECT {};

class BEGIN_ARRAY {};
class END_ARRAY {};

class JsonOutstream
{
public:
    JsonOutstream& operator << (BEGIN_OBJECT)
    {
        mLog += "BEGIN_OBJECT\n";
        mTypeStack.push_back(OBJECT);
        mObjectStack.push_back(json::Object());
        return *this;
    }

    JsonOutstream& operator << (END_OBJECT)
    {
        mLog += "END_OBJECT\n";
        if(mTypeStack.size() == 0)
        {
            mLog += "Error: type stack is empty";
            return *this;
        }
        if(mObjectStack.size() == 0)
        {
            mLog += "Error: object stack is empty";
            return *this;
        }
        if(mTypeStack.back() != OBJECT)
        {
            mLog += "Error: top of the stack is not an object";
        }
        mTypeStack.erase(mTypeStack.end()--);
        // todo

        return *this;
    }

    JsonOutstream& operator << (const KeyValueReference<std::string>& kv)
    {
        return *this;
    }
    const uint8_t OBJECT = 0;
    const uint8_t ARRAY = 1;
    std::vector<uint8_t> mTypeStack;
    std::vector<json::Object> mObjectStack;
    std::vector<json::Array> mArrayStack;

    std::string mLog;
};

class PeersHandler
{
public:
    PeersHandler(RsPeers* peers): mPeers(peers) {}

    template <class InputT, class OutputT>
    void handleRequest(Request& req, InputT& reqData, Response& resp, OutputT& respData)
    {
        if(req.mMethod == "GET")
        {
            std::list<RsPeerId> sslIds;
            mPeers->getFriendList(sslIds);
            respData << BEGIN_ARRAY();
            for(std::list<RsPeerId>::iterator lit = sslIds.begin(); lit != sslIds.end(); lit++)
            {
                respData << BEGIN_OBJECT();
                respData << makeKeyValueReference("name", mPeers->getPeerName(*lit));
                respData << makeKeyValueReference("online", mPeers->isOnline(*lit));
                respData << END_OBJECT();
            }
            respData << END_ARRAY();
        }
    }

    RsPeers* mPeers;
};

*/

}
