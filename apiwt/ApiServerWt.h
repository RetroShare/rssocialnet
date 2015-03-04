#pragma once

#include <Wt/WResource>
#include "../api/ApiServer.h"

namespace resource_api{

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
