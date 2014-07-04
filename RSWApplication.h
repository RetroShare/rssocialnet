#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <retroshare/rsplugin.h>
#include <retroshare/rsgxsifacetypes.h>
#include "sonet/WallWidget.h"
#include "RSWappSearchFilesPage.h"
#include "sonet/IdentityPopupMenu.h"

class RSWApplication: public Wt::WApplication
{
public:
    RSWApplication(const Wt::WEnvironment& env, const RsPlugInInterfaces& interf);
    virtual ~RSWApplication();

    // use this fn to get a pointer to this class
    static RSWApplication* instance();
    // shortcut to get the plugin interfaces
    static RsPlugInInterfaces ifaces();
    // shortcut to get the wall service
    static RsWall::RsWall* rsWall();// TODO

    // then use this fns to modify the application state
    void showWall(const RsGxsId &id);
    RsIdentityDetails getCurrentIdentity(){ return currentIdentity; }
    RsGxsCircleId getPreferredCirlceId(){ return RsGxsCircleId(); } // TODO

private:

    void onIdSelectionChanged(RsIdentityDetails details);

    //Wt::WTabWidget *tabW;
    Wt::WStackedWidget *stackW;
    RsPlugInInterfaces pluginInterfaces;
    RsWall::WallWidget* wallWidget;
    RSWappSearchFilesPage *searchFilesPage;
    RsWall::IdentityPopupMenu* idMenu;
    Wt::WMenuItem* idRootMenuItem;
    RsIdentityDetails currentIdentity;
};

