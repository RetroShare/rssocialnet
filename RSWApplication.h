#pragma once

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <boost/smart_ptr.hpp>

#include <retroshare/rsplugin.h>
#include <retroshare/rsgxsifacetypes.h>
#include <retroshare/rsidentity.h>

namespace RsWall{class RsWall; class WallWidget; class IdentityPopupMenu;}
class RSWappSearchFilesPage;
class RSWappSocialNetworkPage;
class WebUITimer;

class RSWApplication: public Wt::WApplication
{
public:
    RSWApplication(const Wt::WEnvironment& env, const RsPlugInInterfaces& interf, WebUITimer* timer);
    virtual ~RSWApplication();

    // use this fn to get a pointer to this class
    static RSWApplication* instance();
    // shortcut to get the plugin interfaces
    static RsPlugInInterfaces ifaces();
    // shortcut to get the wall service
    static RsWall::RsWall* rsWall();// TODO

    // used by static WebUITimer functions to get the timer for this Wt::WApplication
    static WebUITimer* getTimer();

    // then use this fns to modify the application state
    void showWall(const RsGxsId &id);
    RsIdentityDetails getCurrentIdentity(){ return currentIdentity; }
    RsGxsCircleId getPreferredCirlceId(){ return RsGxsCircleId(); } // TODO

    // ressource caching system
    // usage:
    // create a key which identifies the client who wishes to cache a ressource and the ressource
    //   example: key = "AvatarWidget,grpId=...,timestamp=..."
    // check with the key if the ressource is cached
    // if not cached
    //   boost::shared_ptr ptr(new Wt::WMemoryResource());
    //   ptr.get()->setData(data);
    // keep a copy of the shared pointer to prevent deletion of the ressource

    // return a ressource with the key or null
    boost::shared_ptr<Wt::WMemoryResource> getCachedRessource(const std::string& key);
    // cache a ressource with the given key
    void cacheRessource(const std::string& key, boost::shared_ptr<Wt::WMemoryResource> ressource);

private:

    void onIdSelectionChanged(RsIdentityDetails details);

    //Wt::WTabWidget *tabW;
    Wt::WStackedWidget *stackW;
    RsPlugInInterfaces pluginInterfaces;
    WebUITimer* webUITimer;
    RsWall::WallWidget* wallWidget;
    RSWappSearchFilesPage *searchFilesPage;
    RSWappSocialNetworkPage *socialNetworkPage;
    RsWall::IdentityPopupMenu* idMenu;
    Wt::WMenuItem* idRootMenuItem;
    RsIdentityDetails currentIdentity;

    std::map<std::string, boost::shared_ptr<Wt::WMemoryResource> > ressourceCache;
    time_t nextRessourceCleanup;
};

