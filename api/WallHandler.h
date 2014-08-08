#pragma once

#include "ResourceRouter.h"

class RsIdentity;
namespace RsWall{
    class RsWall;
}

namespace resource_api
{

class WallHandler: public ResourceRouter
{
public:
    WallHandler(RsWall::RsWall* mRsWall, RsIdentity* identity);
    virtual std::string help();
private:
    RsWall::RsWall* mRsWall;
    RsIdentity* mRsIdentity;
    void handleWildcard(Request& req, Response& resp);
    void handleOwn(Request& req, Response& resp);
    void handleActivities(Request& req, Response& resp);
    void handleWall(Request& req, Response& resp);
    void handleAvatarImage(Request &req, Response &resp);
};
} // namespace resource_api
