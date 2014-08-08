#include "ResourceRouter.h"



namespace resource_api
{

class TestResource: public ResourceRouter
{
public:
    TestResource()
    {
        addResourceHandler("eins", this, &TestResource::eins);
    }
    void eins(Request& req, Response& resp)
    {
        //
    }
};

void ResourceRouter::handleRequest(Request& req, Response& resp)
{
    std::vector<std::pair<std::string, HandlerBase*> >::iterator vit;
    if(!req.mPath.empty())
    {
        for(vit = mHandlers.begin(); vit != mHandlers.end(); vit++)
        {
            if(vit->first == req.mPath.top())
            {
                req.mPath.pop();
                vit->second->handleRequest(req, resp);
                return;
            }
        }
    }
    // not found, search for wildcard handler
    for(vit = mHandlers.begin(); vit != mHandlers.end(); vit++)
    {
        if(vit->first == "*")
        {
            // don't pop the path component, because it may contain usefull info for the wildcard handler
            //req.mPath.pop();
            vit->second->handleRequest(req, resp);
            return;
        }
    }
}

} // namespace resource_api
