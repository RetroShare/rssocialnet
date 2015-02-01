#include "ApiServer.h"

#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/Http/ResponseContinuation>

#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>

#include <time.h>
#include <unistd.h>
#include "json.h"

#include <retroshare/rsservicecontrol.h>
#include "rswall.h"
#include "JsonStream.h"
#include "StateTokenServer.h" // for the state token serialisers

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

namespace resource_api{

// old code, only to copy and paste from
// to be removed
/*
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
*/

ApiServer::ApiServer(const RsPlugInInterfaces &ifaces):
    //mPlugInInterfaces(ifaces)
    mStateTokenServer(),
    mPeersHandler(&mStateTokenServer, ifaces.mNotify, ifaces.mPeers, ifaces.mMsgs),
    mIdentityHandler(ifaces.mIdentity),
    mWallHandler(RsWall::rsWall, ifaces.mIdentity),
    mServiceControlHandler(rsServiceControl)
{
    // the dynamic cast is to not confuse the addResourceHandler template like this:
    // addResourceHandler(derived class, parent class)
    // the template would then be instantiated using derived class as parameter
    // and then parent class would not match the type
    mRouter.addResourceHandler("peers",dynamic_cast<ResourceRouter*>(&mPeersHandler),
                               &PeersHandler::handleRequest);
    mRouter.addResourceHandler("identity", dynamic_cast<ResourceRouter*>(&mIdentityHandler),
                               &IdentityHandler::handleRequest);
    mRouter.addResourceHandler("wall", dynamic_cast<ResourceRouter*>(&mWallHandler),
                               &WallHandler::handleRequest);
    mRouter.addResourceHandler("servicecontrol", dynamic_cast<ResourceRouter*>(&mServiceControlHandler),
                               &ServiceControlHandler::handleRequest);
    mRouter.addResourceHandler("statetokenservice", dynamic_cast<ResourceRouter*>(&mStateTokenServer),
                               &StateTokenServer::handleRequest);

}

std::string ApiServer::handleRequest(Request &request)
{
    resource_api::JsonStream outstream;
    std::stringstream debugString;

    StreamBase& data = outstream.getStreamToMember("data");
    resource_api::Response resp(data, debugString);

    ResponseTask* task = mRouter.handleRequest(request, resp);

    time_t start = time(NULL);
    while(task && task->doWork(request, resp))
    {
        usleep(10*1000);
        /*if(time(NULL) > (start+5))
        {
            std::cerr << "ApiServer::handleRequest() Error: task timed out" << std::endl;
            resp.mDebug << "Error: task timed out." << std::endl;
            resp.mReturnCode = resource_api::Response::FAIL;
            break;
        }*/
    }
    if(task)
        delete task;

    std::string returncode;
    switch(resp.mReturnCode){
    case resource_api::Response::NOT_SET:
        returncode = "not_set";
        break;
    case resource_api::Response::OK:
        returncode = "ok";
        break;
    case resource_api::Response::WARNING:
        returncode = "warning";
        break;
    case resource_api::Response::FAIL:
        returncode = "fail";
        break;
    }

    // evil HACK, remove this
    if(data.isRawData())
        return data.getRawData();

    outstream << resource_api::makeKeyValueReference("returncode", returncode);
    if(!resp.mStateToken.isNull())
        outstream << resource_api::makeKeyValueReference("statetoken", resp.mStateToken);
    return outstream.getJsonString();
}

ApiServerWt::ApiServerWt(const RsPlugInInterfaces &ifaces):
    WResource(), mApiServer(ifaces)
{
    setDispositionType(Inline);
}

void ApiServerWt::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response)
{
    // TODO: protect againts handling untrusted content to the browser
    // see:
    // http://www.dotnetnoob.com/2012/09/security-through-http-response-headers.html
    // http://www.w3.org/TR/CSP2/
    // https://code.google.com/p/doctype-mirror/wiki/ArticleContentSniffing

    // tell Internet Explorer to not do content sniffing
    response.addHeader("X-Content-Type-Options", "nosniff");
    // lots of TODO

    std::cerr << "TestRessource::handleRequest() pathInfo=" << request.pathInfo() << std::endl;

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

    //response.setMimeType("text/plain");
    //response.setMimeType(("application/json"));

    std::istreambuf_iterator<char> eos;
    std::string reqDataStr(std::istreambuf_iterator<char>(request.in()), eos);

    if(path1 == "/v2")
    {
        resource_api::JsonStream instream;
        instream.setJsonString(reqDataStr);
        resource_api::Request req(instream);

        if(request.method() == "GET")
        {
            req.mMethod = resource_api::Request::GET;
        }
        else if(request.method() == "POST")
        {
            req.mMethod = resource_api::Request::PUT;
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
        req.mFullPath = path2;

        std::string result = mApiServer.handleRequest(req);

        // EVIL HACK remove
        if(result[0] != '{')
            response.setMimeType("image/png");
        else
            response.setMimeType("text/plain");

        response.out() << result;

    }

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

} // namespace resource_api
