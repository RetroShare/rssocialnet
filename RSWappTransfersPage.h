#pragma once

#include <Wt/WTableView>

class RSWappTransfersPage: public Wt::WCompositeWidget
{
	public:
		RSWappTransfersPage(Wt::WContainerWidget *container) 
		{
			setImplementation(_impl = new Wt::WContainerWidget());
		}

	private:
		Wt::WContainerWidget *_impl;
};
