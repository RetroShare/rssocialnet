#pragma once

#include <Wt/WTableView>

class RsPeers ;
class FriendListModel ;

class RSWappFriendsPage: public Wt::WCompositeWidget
{
	public:
		RSWappFriendsPage(Wt::WContainerWidget *container,RsPeers *rspeers) ;

		virtual void refresh() ;

	private:
		Wt::WContainerWidget *_impl;
		RsPeers *mPeers ;
		FriendListModel *_model ;
		Wt::WTimer *_timer ;
		Wt::WTableView *_tableView ;
};
