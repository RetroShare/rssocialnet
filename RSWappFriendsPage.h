#pragma once

#include <Wt/WTableView>

class RsPeers ;
class RSWappFriendsPage: public Wt::WCompositeWidget
{
	public:
		RSWappFriendsPage(Wt::WContainerWidget *container,RsPeers *rspeers) ;

	private:
		Wt::WContainerWidget *_impl;
		RsPeers *mPeers ;
};
