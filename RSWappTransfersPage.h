#pragma once

#include <Wt/WTableView>
#include <Wt/WTextArea>

class RSWappTransfersPage: public Wt::WCompositeWidget
{
	public:
		RSWappTransfersPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

		void downloadLink() ;

	private:
		Wt::WContainerWidget *_impl;
		RsFiles *mFiles ;
		Wt::WTimer *_timer ;
		Wt::WTextArea *link_area ;
};
