#pragma once

#include <Wt/WTableView>

class RSWappConfigPage: public Wt::WCompositeWidget
{
	public:
		RSWappConfigPage(Wt::WContainerWidget *container) 
		{
			setImplementation(_impl = new Wt::WContainerWidget());

		}

	private:
		Wt::WContainerWidget *_impl;
};
