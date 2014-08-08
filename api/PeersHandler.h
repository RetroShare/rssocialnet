#pragma once

#include "ResourceRouter.h"

class RsPeers;
class RsMsgs;

namespace resource_api
{

class PeersHandler: public ResourceRouter
{
public:
    PeersHandler(RsPeers* peers, RsMsgs* msgs);

    virtual std::string help();
private:
    RsPeers* mRsPeers;
    RsMsgs* mRsMsgs;
    void handleWildcard(Request& req, Response& resp);
    void handleExamineCert(Request& req, Response& resp);
};
} // namespace resource_api
