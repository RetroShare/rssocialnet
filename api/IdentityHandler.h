#pragma once

#include "ResourceRouter.h"

class RsIdentity;

namespace resource_api
{

class IdentityHandler: public ResourceRouter
{
public:
    IdentityHandler(RsIdentity* identity);
    virtual std::string help();
private:
    RsIdentity* mRsIdentity;
    void handleWildcard(Request& req, Response& resp);
    void handleOwn(Request& req, Response& resp);
};
} // namespace resource_api
