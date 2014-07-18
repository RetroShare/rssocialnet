#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>
#include <Wt/WBreak>

#if WT_VERSION < 0x03030000
#error rssocialnet needs at least Wt version 3.3.0 because of Wt::WNavigationBar
#endif
#include <Wt/WNavigationBar>
#include <Wt/WStackedWidget>

#if WT_VERSION >= 0x03030200
#include <Wt/WBootstrapTheme>
#endif

#include "RSWApplication.h"

#include "RSWappFriendsPage.h"
#include "RSWappTransfersPage.h"
#include "RSWappChatLobbiesPage.h"
#include "RSWappConfigPage.h"
#include "RSWappSharedFilesPage.h"
#include "RSWappSearchFilesPage.h"

#include "RSWappTestPage.h"
#include "RSWappSocialNetworkPage.h"

#include "sonet/SonetUtil.h"
#include "sonet/RsGxsUpdateBroadcastWt.h"
#include "sonet/WallWidget.h"
#include "sonet/IdentityPopupMenu.h"

const int RSWAPPLICATION_RESSOURCE_CLEANUP_PERIOD = 10*60; // in seconds

RSWApplication::RSWApplication(const Wt::WEnvironment& env,const RsPlugInInterfaces& interf, WebUITimer* timer)
    : WApplication(env), pluginInterfaces(interf), webUITimer(timer),
      nextRessourceCleanup(time(NULL)+RSWAPPLICATION_RESSOURCE_CLEANUP_PERIOD)
{
    enableUpdates(true);// required for server side events (WebUITimer)

    setTitle(Wt::WString("Retroshare Social Network Plugin. Version {1} TODO: use git version id").arg(SVN_REVISION_NUMBER)); // application title

	Wt::WContainerWidget *container = new Wt::WContainerWidget();

    Wt::WNavigationBar *navigation = new Wt::WNavigationBar(container);

    stackW = new Wt::WStackedWidget(container);
    Wt::WMenu* menu = new Wt::WMenu(stackW);
    menu->addStyleClass("navbar-nav");// bootstrap class to make the menu horizontal
    navigation->addMenu(menu);

    menu->addItem("SocialNetwork", socialNetworkPage = new RSWappSocialNetworkPage());
    menu->addItem("TestPage", new RSWappTestPage());
    menu->addItem("Friends", new RSWappFriendsPage(interf.mPeers, interf.mMsgs));
    menu->addItem("Transfers", new RSWappTransfersPage(interf.mFiles));
    menu->addItem("Shared files", new RSWappSharedFilesPage(interf.mFiles,interf.mPeers));
    menu->addItem("Search", searchFilesPage = new RSWappSearchFilesPage(interf.mFiles));

    wallWidget = new RsWall::WallWidget();
    stackW->addWidget(wallWidget);

    // IdentityPopupMenue and the idRootMenue is good for:
    // - display the currently selected id in a menu item of the root menu
    // - show popup menue when the user clicks on the root menue item
    // - let the user select another id
    // it would be nice to have this as a widget
    idMenu = new RsWall::IdentityPopupMenu();
    idMenu->identitySelectionChanged().connect(this, &RSWApplication::onIdSelectionChanged);

    Wt::WMenu* idRootMenu = new Wt::WMenu();
    idRootMenu->addStyleClass("navbar-nav navbar-right");// bootstrap class to make the menu horizontal
    navigation->addMenu(idRootMenu, Wt::AlignRight);

    currentIdentity = idMenu->getCurrentIdentity();
    idRootMenuItem = new Wt::WMenuItem("");
    idRootMenuItem->setMenu(idMenu);
    idRootMenu->addItem(idRootMenuItem);


/*    tabW = new Wt::WTabWidget(container);
    //tabW->setInternalPathEnabled();

    tabW->addTab(new RSWappSocialNetworkPage(container), "SocialNetwork", Wt::WTabWidget::PreLoading)->setPathComponent("sonet");
    tabW->addTab(new RSWappTestPage(container), "TestPage", Wt::WTabWidget::PreLoading);
    tabW->addTab(wallWidget = new WallWidget(container), "WallWidget(hidden)", Wt::WTabWidget::PreLoading);
    tabW->setTabHidden(tabW->indexOf(wallWidget), true);

	tabW->addTab(new RSWappFriendsPage(container,interf.mPeers,interf.mMsgs), "Friends", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappTransfersPage(container,interf.mFiles),"Transfers", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappSharedFilesPage(container,interf.mFiles,interf.mPeers),"Shared files", Wt::WTabWidget::PreLoading);
    tabW->addTab(searchFilesPage = new RSWappSearchFilesPage(container,interf.mFiles),"Search", Wt::WTabWidget::PreLoading);
    */
//	tabW->addTab(new RSWappChatLobbiesPage(container),"Chat lobbies", Wt::WTabWidget::PreLoading);
//	tabW->addTab(new RSWappConfigPage(container),"Config", Wt::WTabWidget::PreLoading);

    pluginInterfaces.mNotify->registerNotifyClient(searchFilesPage) ;

	//Wt::WMenuItem *tab = tabW->addTab(new Wt::WTextArea("You can close this tab by clicking on the close icon."), "Close");
	//tab->setCloseable(true);
    //tabW->setStyleClass("tabwidget");
    //setCssTheme("polished");
    //setCssTheme("bootstrap");

    //setCssTheme(""); // don't use any css styling

#if WT_VERSION >= 0x03030200
    Wt::WBootstrapTheme* theme = new Wt::WBootstrapTheme();
    theme->setVersion(Wt::WBootstrapTheme::Version3);
    theme->setResponsive(true);
    setTheme(theme);
    // this gives a bit color to the buttons
    useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
#endif
	root()->addWidget(container) ;
}

RSWApplication::~RSWApplication()
{
    pluginInterfaces.mNotify->unregisterNotifyClient(searchFilesPage);
    RsWall::RsGxsUpdateBroadcastWt::unregisterApplication();
}

RSWApplication* RSWApplication::instance()
{
    RSWApplication* sonet = dynamic_cast<RSWApplication*>(Wt::WApplication::instance());
    if(sonet == NULL)
    {
        std::cerr << "FATAL ERROR in RSWApplication::instance(): this Wt::WApplication is not of type RSWApplictaion." << std::endl;
    }
    return sonet;
}

RsPlugInInterfaces RSWApplication::ifaces()
{
    return RSWApplication::instance()->pluginInterfaces;
}

WebUITimer* RSWApplication::getTimer()
{
    return RSWApplication::instance()->webUITimer;
}

// want to have this with internal paths later
void RSWApplication::showWall(const RsGxsId &id)
{
    // forward to social network page
    socialNetworkPage->showWall(id);

    // when using navigation bar and wallWidget in the stack
    //wallWidget->setWallByAuthorId(id);
    //stackW->setCurrentWidget(wallWidget);


    // when using the tab widget

    //tabW->setCurrentWidget(wallWidget);

    // hide the menu item for this tab again
    //tabW->setTabHidden(tabW->indexOf(wallWidget), true);
}

boost::shared_ptr<Wt::WMemoryResource> RSWApplication::getCachedRessource(const std::string &key)
{
    std::map<std::string, boost::shared_ptr<Wt::WMemoryResource> >::iterator mit;
    mit = ressourceCache.find(key);
    if(mit == ressourceCache.end())
    {
        std::cerr << "RSWApplication::getCachedRessource() was not cached key=" << key << std::endl;
        return boost::shared_ptr<Wt::WMemoryResource>();
    }
    std::cerr << "RSWApplication::getCachedRessource() was cached key=" << key << std::endl;
    return mit->second;
}

void RSWApplication::cacheRessource(const std::string &key, boost::shared_ptr<Wt::WMemoryResource> ressource)
{
    if( ressource.get() != NULL)
    {
        std::cerr << "RSWApplication::cacheRessource() cached key=" << key << std::endl;
        ressourceCache[key] = ressource;
    }

    if(time(NULL) > nextRessourceCleanup)
    {
        // clean up old entries
        std::vector<std::map<std::string, boost::shared_ptr<Wt::WMemoryResource> >::iterator > toDelete;
        std::map<std::string, boost::shared_ptr<Wt::WMemoryResource> >::iterator mit;
        for(mit = ressourceCache.begin(); mit != ressourceCache.end(); mit++)
        {
            if(mit->second.unique())
            {
                std::cerr << "RSWApplication::cacheRessource() cleaning key=" << mit->first << std::endl;
                toDelete.push_back(mit);
            }
        }
        std::vector<std::map<std::string, boost::shared_ptr<Wt::WMemoryResource> >::iterator >::iterator vit;
        for(vit = toDelete.begin(); vit != toDelete.end(); vit++)
        {
            ressourceCache.erase(*vit);
        }
        nextRessourceCleanup = time(NULL) + RSWAPPLICATION_RESSOURCE_CLEANUP_PERIOD;
    }
}

void RSWApplication::onIdSelectionChanged(RsIdentityDetails details)
{
    currentIdentity = details;
    if(details.mId.isNull())
    {
        idRootMenuItem->setText("click here to create a new identity");
    }
    else
    {
        idRootMenuItem->setText("your are " + SonetUtil::formatGxsId(currentIdentity));
        socialNetworkPage->setOwnId(details.mId);
    }
}
