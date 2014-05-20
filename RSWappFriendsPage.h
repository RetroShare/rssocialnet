#pragma once

#include <Wt/WTableView>

class RsPeers ;
class FriendListModel ;

class RSWappFriendsPage: public Wt::WCompositeWidget
{
	public:
		RSWappFriendsPage(Wt::WContainerWidget *container,RsPeers *rspeers,RsMsgs *rsmsgs) ;

		virtual void refresh() ;

	private:
		void showFriendDetails(const std::string& friend_id) ;
		void showCustomPopupMenu(const Wt::WModelIndex&, const Wt::WMouseEvent&) ;
		void popupAction() ;
		void addFriend() ;

		Wt::WContainerWidget *_impl;
		RsPeers *mPeers ;
		RsMsgs *mMsgs ;
		FriendListModel *_model ;
		Wt::WTableView *_tableView ;
		Wt::WPopupMenu *_popupMenu ;
		std::string _selected_friend ;
		Wt::WTimer *_timer ;
};
