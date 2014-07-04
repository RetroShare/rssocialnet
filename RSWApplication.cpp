#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>

#include <Wt/WNavigationBar>
#include <Wt/WStackedWidget>

#if WT_VERSION >= 0x03030200
#include <Wt/WBootstrapTheme>
#endif

#include "RSWApplication.h"

#include "RSWappTestPage.h"
#include "RSWappSocialNetworkPage.h"

#include "RSWappFriendsPage.h"
#include "RSWappTransfersPage.h"
#include "RSWappChatLobbiesPage.h"
#include "RSWappConfigPage.h"
#include "RSWappSearchFilesPage.h"
#include "RSWappSharedFilesPage.h"

#include "sonet/SonetUtil.h"

RSWApplication::RSWApplication(const Wt::WEnvironment& env,const RsPlugInInterfaces& interf)
    : WApplication(env), pluginInterfaces(interf)
{
    setTitle(Wt::WString("Retroshare Social Network Plugin. Version {1} TODO: use git version id").arg(SVN_REVISION_NUMBER)); // application title

	Wt::WContainerWidget *container = new Wt::WContainerWidget();

    Wt::WNavigationBar *navigation = new Wt::WNavigationBar(container);

    stackW = new Wt::WStackedWidget(container);
    // the docs say: the menu can be styled using CSS to make it horizontal
    //   todo: find out how to do this
    Wt::WMenu* menu = new Wt::WMenu(stackW);
    navigation->addMenu(menu);

    menu->addItem("SocialNetwork", new RSWappSocialNetworkPage());
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
    setTheme(theme);
#endif
	root()->addWidget(container) ;
}

RSWApplication::~RSWApplication()
{
    pluginInterfaces.mNotify->unregisterNotifyClient(searchFilesPage);
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

// want to have this with internal paths later
void RSWApplication::showWall(const RsGxsId &id)
{
    wallWidget->setWallByAuthorId(id);
    stackW->setCurrentWidget(wallWidget);

    //tabW->setCurrentWidget(wallWidget);

    // hide the menu item for this tab again
    //tabW->setTabHidden(tabW->indexOf(wallWidget), true);
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
    }
}
