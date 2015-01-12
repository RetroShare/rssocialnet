#pragma once

#include <Wt/WResource>

#include <retroshare/rsplugin.h>

#include "ApiTypes.h"
#include "PeersHandler.h"
#include "IdentityHandler.h"
#include "WallHandler.h"
#include "ServiceControlHandler.h"
#include "StateTokenServer.h"

namespace resource_api{

// main entry point for all resource api calls

// call chain is like this:
// Wt -> ApiServerWt -> ApiServer -> different handlers
// later i want to replace the parts with Wt with something else
// (Wt is a too large framework, a simple http server would be enough)
// maybe use libmicrohttpd
// the other use case for this api is a qt webkit view
// this works without html, the webkitview calls directly into our c++ code

// general part of the api server
// should work with any http library or a different transport protocol
class ApiServer
{
public:
    ApiServer(const RsPlugInInterfaces& ifaces);

    // it is currently hard to separate into http and non http stuff
    // mainly because the http path is used in the api
    // this has to change later
    // for now let the http part make the request object
    // and the general apiserver part makes the response
    std::string handleRequest(Request& request);

private:
    void handleHelp(Request& req, Response& resp);

    //RsPlugInInterfaces mPlugInInterfaces;
    StateTokenServer mStateTokenServer; // goes first, as others may depend on it
    PeersHandler mPeersHandler;
    IdentityHandler mIdentityHandler;
    WallHandler mWallHandler;
    ServiceControlHandler mServiceControlHandler;

    ResourceRouter mRouter;
};

// main entry point of the http api
class ApiServerWt: public Wt::WResource
{
public:
    ApiServerWt(const RsPlugInInterfaces& ifaces);
    ~ApiServerWt(){ beingDeleted(); } // Wt wants this

    void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);

private:
    ApiServer mApiServer;
};

}
