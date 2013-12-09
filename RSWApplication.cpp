#include <retroshare/rsplugin.h>

#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>

#include "RSWApplication.h"
#include "RSWappFriendsPage.h"
#include "RSWappTransfersPage.h"
#include "RSWappChatLobbiesPage.h"
#include "RSWappConfigPage.h"
#include "RSWappSharedFilesPage.h"

RSWApplication::RSWApplication(const WEnvironment& env,const RsPlugInInterfaces& interf)
   : WApplication(env)
{
	setTitle("Hello world");               // application title

	Wt::WContainerWidget *container = new Wt::WContainerWidget();
	Wt::WTabWidget *tabW = new Wt::WTabWidget(container);

	tabW->addTab(new RSWappFriendsPage(container,interf.mPeers), "Friends", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappTransfersPage(container,interf.mFiles),"Transfers", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappSharedFilesPage(container,interf.mFiles),"Shared files", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappChatLobbiesPage(container),"Chat lobbies", Wt::WTabWidget::PreLoading);
	tabW->addTab(new RSWappConfigPage(container),"Config", Wt::WTabWidget::PreLoading);

	//Wt::WMenuItem *tab = tabW->addTab(new Wt::WTextArea("You can close this tab by clicking on the close icon."), "Close");
	//tab->setCloseable(true);
	tabW->setStyleClass("tabwidget");
	setCssTheme("polished");

	root()->addWidget(container) ;
}

void RSWApplication::greet()
{
    greeting_->setText("Hello there, " + nameEdit_->text());
}

