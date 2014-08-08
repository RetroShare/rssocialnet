#pragma once

#include <Wt/WResource>

#include <retroshare/rsplugin.h>

namespace new_api{

class ApiServer: public Wt::WResource
{
public:
    ApiServer(const RsPlugInInterfaces& ifaces);
    ~ApiServer(){ beingDeleted(); } // Wt wants this

    void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);

private:
    RsPlugInInterfaces ifaces;
};

}
