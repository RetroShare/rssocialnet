#pragma once

#include <Wt/WTableView>

class RSWappChatLobbiesPage: public Wt::WCompositeWidget
{
	public:
		RSWappChatLobbiesPage(Wt::WContainerWidget *container) 
		{
			setImplementation(_impl = new Wt::WContainerWidget());
		}

	private:
		Wt::WContainerWidget *_impl;
};
