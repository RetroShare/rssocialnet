#pragma once

#include <Wt/WTableView>

class RSWappSharedFilesPage: public Wt::WCompositeWidget
{
	public:
		RSWappSharedFilesPage(Wt::WContainerWidget *container) 
		{
			setImplementation(_impl = new Wt::WContainerWidget());
		}

	private:
		Wt::WContainerWidget *_impl;
};
