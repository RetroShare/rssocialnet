#pragma once

#include <Wt/WTableView>

class RSWappTransfersPage: public Wt::WCompositeWidget
{
	public:
		RSWappTransfersPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

	private:
		Wt::WContainerWidget *_impl;
		RsFiles *mFiles ;
};
