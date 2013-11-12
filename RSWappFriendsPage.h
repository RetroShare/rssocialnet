#pragma once

#include <Wt/WTableView>

class RSWappFriendsPage: public Wt::WCompositeWidget
{
	public:
		RSWappFriendsPage(Wt::WContainerWidget *container) ;

	private:
		Wt::WContainerWidget *_impl;
};
