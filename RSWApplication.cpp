#include <retroshare/rsplugin.h>

#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>

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

RSWApplication::RSWApplication(const WEnvironment& env,const RsPlugInInterfaces& interf)
   : WApplication(env)
{
    setTitle(Wt::WString("Retroshare Social Network Plugin. Version {1} TODO: use git version id").arg(SVN_REVISION_NUMBER)); // application title

	Wt::WContainerWidget *container = new Wt::WContainerWidget();
    tabW = new Wt::WTabWidget(container);
    //tabW->setInternalPathEnabled();

	RSWappSearchFilesPage *search ;

    tabW->addTab(new RSWappSocialNetworkPage(container), "SocialNetwork", Wt::WTabWidget::PreLoading)->setPathComponent("sonet");
    tabW->addTab(new RSWappTestPage(container), "TestPage", Wt::WTabWidget::PreLoading);
    tabW->addTab(wallWidget = new WallWidget(container), "WallWidget(hidden)", Wt::WTabWidget::PreLoading);
    tabW->setTabHidden(tabW->indexOf(wallWidget), true);

	tabW->addTab(new RSWappFriendsPage(container,interf.mPeers,interf.mMsgs), "Friends", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappTransfersPage(container,interf.mFiles),"Transfers", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappSharedFilesPage(container,interf.mFiles,interf.mPeers),"Shared files", Wt::WTabWidget::PreLoading);
	tabW->addTab(search = new RSWappSearchFilesPage(container,interf.mFiles),"Search", Wt::WTabWidget::PreLoading);
//	tabW->addTab(new RSWappChatLobbiesPage(container),"Chat lobbies", Wt::WTabWidget::PreLoading);
//	tabW->addTab(new RSWappConfigPage(container),"Config", Wt::WTabWidget::PreLoading);

	interf.mNotify->registerNotifyClient(search) ;

	//Wt::WMenuItem *tab = tabW->addTab(new Wt::WTextArea("You can close this tab by clicking on the close icon."), "Close");
	//tab->setCloseable(true);
	tabW->setStyleClass("tabwidget");
    //setCssTheme("polished");
    //setCssTheme("bootstrap");
#if WT_VERSION >= 0x03030200
    WBootstrapTheme* theme = new WBootstrapTheme();
    theme->setVersion(WBootstrapTheme::Version3);
    setTheme(theme);
#endif
	root()->addWidget(container) ;
}

// want to have this with internal paths later
void RSWApplication::showWall(const RsGxsId &id)
{
    wallWidget->setWallByAuthorId(id);
    tabW->setCurrentWidget(wallWidget);
    // hide the menu item for this tab again
    tabW->setTabHidden(tabW->indexOf(wallWidget), true);
}
