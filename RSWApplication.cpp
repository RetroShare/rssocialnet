#include <Wt/WContainerWidget>
#include <Wt/WMenuItem>
#include <Wt/WTabWidget>
#include <Wt/WTextArea>

#include "RSWApplication.h"
#include "RSWappFriendsPage.h"

RSWApplication::RSWApplication(const WEnvironment& env)
   : WApplication(env)
{
	setTitle("Hello world");               // application title

	Wt::WContainerWidget *container = new Wt::WContainerWidget();
	Wt::WTabWidget *tabW = new Wt::WTabWidget(container);

	tabW->addTab(new RSWappFriendsPage(container), "Friends", Wt::WTabWidget::LazyLoading);
	tabW->addTab(new Wt::WTextArea("The contents of the tabs are pre-loaded in the browser to ensure swift switching."), "Preload", Wt::WTabWidget::PreLoading);
	tabW->addTab(new Wt::WTextArea("You could change any other style attribute of the tab widget by modifying the style class. The style class 'trhead' is applied to this tab."), "Style", Wt::WTabWidget::PreLoading);

	Wt::WMenuItem *tab = tabW->addTab(new Wt::WTextArea("You can close this tab by clicking on the close icon."), "Close");
	//tab->setCloseable(true);
	tabW->setStyleClass("tabwidget");

	root()->addWidget(container) ;
}

void RSWApplication::greet()
{
    greeting_->setText("Hello there, " + nameEdit_->text());
}

