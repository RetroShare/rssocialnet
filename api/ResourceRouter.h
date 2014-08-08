#pragma once

#include "ApiTypes.h"

namespace resource_api
{

class ResourceRouter
{
public:

    void handleRequest(Request& req, Response& resp);

    template <class T>
    void addResourceHandler(std::string name, T* instance, void (T::*callback)(Request& req, Response& resp));

    // implement this to return an help text
    virtual std::string help(){return "";}

private:
    class HandlerBase
    {
    public:
        virtual void handleRequest(Request& req, Response& resp) = 0;
    };
    template <class T>
    class Handler: public HandlerBase
    {
    public:
        virtual void handleRequest(Request &req, Response &resp)
        {
            (instance->*method)(req, resp);
        }
        T* instance;
        void (T::*method)(Request& req, Response& resp);
    };

    std::vector<std::pair<std::string, HandlerBase*> > mHandlers;
};

// the advantage of this approach is:
// the method name is arbitrary, one class can have many different handler methods
// with raw objects the name of the handler method would be fixed, and we would need one class for every handler
template <class T>
void ResourceRouter::addResourceHandler(std::string name, T* instance, void (T::*callback)(Request& req, Response& resp))
{
    Handler<T>* handler = new Handler<T>();
    handler->instance = instance;
    handler->method = callback;
    mHandlers.push_back(std::make_pair(name, handler));
}

} // namespace resource_api
