#include "ApiServerWt.h"

#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/Http/ResponseContinuation>

#include "../api/JsonStream.h"
#include "rswall.h"

namespace resource_api{

ApiServerWt::ApiServerWt(const RsPlugInInterfaces &ifaces):
    WResource(), mApiServer()
{
    setDispositionType(Inline);
    mApiServer.loadMainModules(ifaces);
    mApiServer.loadWallModule(ifaces, RsWall::rsWall);
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
