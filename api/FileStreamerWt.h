#pragma once

#include <Wt/WResource>
#include <retroshare/rsplugin.h>

class FileStreamerWt: public Wt::WResource
{
public:
    FileStreamerWt(const RsPlugInInterfaces& ifaces);
    ~FileStreamerWt(){ beingDeleted(); } // Wt wants this

    void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);

private:
    RsPlugInInterfaces mIfaces;
};
