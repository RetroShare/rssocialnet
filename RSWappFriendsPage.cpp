#include <retroshare/rspeers.h>

#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>

#include "RSWappFriendsPage.h"

class FriendListModel : public Wt::WAbstractTableModel
{
	public:
		FriendListModel(RsPeers *peers,Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent), mPeers(peers)
		{
			_last_time_update = 0 ;
		}

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return _friends.size() ;
			else
				return 0;
		}

		virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return 5;
			else
				return 0;
		}

		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			if(index.column() >= 5 || index.row() >= _friends.size())
				return boost::any();

			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						case 0: return Wt::WString(_friends[index.row()].name) ;
						case 1: return Wt::WString(_friends[index.row()].gpg_id) ;
						case 2: return Wt::WString(_friends[index.row()].id) ;
						case 3: if(_friends[index.row()].state & RS_PEER_STATE_CONNECTED)
									  return Wt::WString("Now") ;
								  else
									  return lastSeenString(_friends[index.row()].lastConnect) ;
						case 4: 
								  if(_friends[index.row()].state & RS_PEER_STATE_CONNECTED)
								  {
									  std::string s_ip = _friends[index.row()].connectAddr ;

									  return Wt::WString(s_ip + ":{1}").arg(_friends[index.row()].connectPort) ;
								  }
								  else
									  return Wt::WString("---") ;
					}
				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[5] = { Wt::WString("Name (location)"),
															Wt::WString("PGP id"),
															Wt::WString("Location ID"),
															Wt::WString("Last seen"), 
															Wt::WString("IP:Port") } ;
			updateFriendList() ;

			if (orientation == Wt::Horizontal) 
				switch (role) 
				{
					case Wt::DisplayRole:
						return col_names[section] ;
					default:
						return boost::any();
				}
			else
				return boost::any();
		}

		static Wt::WString lastSeenString(time_t s)
		{
			time_t now = time(NULL) ;

			if(now < 5+s) return Wt::WString("Now") ;
			if(now < 3600+s) return Wt::WString("One hour ago") ;
			if(now < 86400+s) return Wt::WString("In the last 24 hours") ;
			if(now < 7*86400+s) return Wt::WString("Few days ago") ;

			return Wt::WString("Long time ago / never") ;
		}
	private:
		void updateFriendList() const
		{
			time_t now = time(NULL) ;

			if(now < 2+_last_time_update)
				return ;
			
			std::cerr << "Updating list..." << std::endl;
			_last_time_update = now ;

			std::list<std::string> fids ;
			
			if(!mPeers->getFriendList(fids)) 
				std::cerr << "(EE) " << __PRETTY_FUNCTION__ << ": can't get list of friends." << std::endl;

			_friends.clear() ;

			for(std::list<std::string>::const_iterator it(fids.begin());it!=fids.end();++it)
			{
				_friends.push_back(RsPeerDetails()) ;

				mPeers->getPeerDetails(*it,_friends.back()) ;
			}
		}
		int rows_, columns_;
		mutable std::vector<RsPeerDetails> _friends ;
		mutable time_t _last_time_update ;
		RsPeers *mPeers ;
};

RSWappFriendsPage::RSWappFriendsPage(Wt::WContainerWidget *parent,RsPeers *mpeers)
	: WCompositeWidget(parent),mPeers(mpeers)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	Wt::WTableView *tableView = new Wt::WTableView(_impl);

	tableView->setAlternatingRowColors(true);

	//tableView->setModel(fileFilterModel_);
	tableView->setSelectionMode(Wt::ExtendedSelection);
	tableView->setDragEnabled(true);

	tableView->setColumnWidth(0, 100);
	tableView->setColumnWidth(1, 150);
	tableView->setColumnWidth(2, 250);
	tableView->setColumnWidth(3, 150);
	tableView->setColumnWidth(4, 150);
	tableView->setColumnWidth(5, 100);

	tableView->setModel(new FriendListModel(mpeers)) ;
}
