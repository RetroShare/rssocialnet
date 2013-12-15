#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>

#include <Wt/WTimer>
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WImage>
#include <Wt/WMessageBox>
#include <Wt/WRasterImage>
#include <Wt/WVBoxLayout>
#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>
#include <Wt/WAbstractItemDelegate>

#include <util/radix64.h>
#include "RSWappFriendsPage.h"

static const uint32_t COLUMN_AVATAR = 0x00 ;
static const uint32_t COLUMN_NAME   = 0x01 ;
static const uint32_t COLUMN_PGP_ID = 0x02 ;
static const uint32_t COLUMN_SSL_ID = 0x03 ;
static const uint32_t COLUMN_LAST_S = 0x04 ;
static const uint32_t COLUMN_IP     = 0x05 ;

class FriendListModel : public Wt::WAbstractTableModel
{
	public:
		FriendListModel(RsPeers *peers,RsMsgs *msgs,Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent), mPeers(peers), mMsgs(msgs) 
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
						case COLUMN_AVATAR: return Wt::WString("") ;
						case COLUMN_NAME: return Wt::WString(_friends[index.row()].name) ;
						case COLUMN_PGP_ID: return Wt::WString(_friends[index.row()].gpg_id) ;
						case COLUMN_SSL_ID: return Wt::WString(_friends[index.row()].id) ;
						case COLUMN_LAST_S: if(_friends[index.row()].state & RS_PEER_STATE_CONNECTED)
									  return Wt::WString("Now") ;
								  else
									  return lastSeenString(_friends[index.row()].lastConnect) ;
						case COLUMN_IP: 
								  if(_friends[index.row()].state & RS_PEER_STATE_CONNECTED)
								  {
									  std::string s_ip = _friends[index.row()].connectAddr ;

									  return Wt::WString(s_ip + ":{1}").arg(_friends[index.row()].connectPort) ;
								  }
								  else
									  return Wt::WString("---") ;
					}

				case Wt::UserRole:
					return Wt::WString(_friends[index.row()].id) ;

				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[6] = { Wt::WString("Avatar"),
															Wt::WString("Name (location)"),
															Wt::WString("PGP id"),
															Wt::WString("Location ID"),
															Wt::WString("Last seen"), 
															Wt::WString("IP:Port") } ;
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
		void refresh()
		{
			updateFriendList() ;
			dataChanged().emit(index(0,0),index(rowCount(),columnCount())) ;
		}
		const std::string& getAvatarUrl(int row)
		{
			static const std::string null_str ;

			if(row >= _friend_avatars.size())
				return null_str ;
			else
				return _friend_avatars[row] ;
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
			_friend_avatars.clear() ;

			for(std::list<std::string>::const_iterator it(fids.begin());it!=fids.end();++it)
			{
				_friends.push_back(RsPeerDetails()) ;

				mPeers->getPeerDetails(*it,_friends.back()) ;

				unsigned char *data = NULL ;
				int size = 0 ;

				mMsgs->getAvatarData(*it,data,size) ;

				std::string base64_string ;
				Radix64::encode((const char *)data,size,base64_string) ;

				_friend_avatars.push_back(std::string("data:image/jpeg;base64,"+base64_string)) ;

				std::cerr << "Got new avatar for friend " << *it << ": " << _friend_avatars.back() << std::endl;
			}
		}
		int rows_, columns_;
		mutable std::vector<RsPeerDetails> _friends ;
		mutable std::vector<std::string> _friend_avatars;
		mutable time_t _last_time_update ;

		RsPeers *mPeers ;
		RsMsgs *mMsgs ;
};

class FriendPageAvatarDelegate: public Wt::WAbstractItemDelegate
{
	public:
		FriendPageAvatarDelegate(FriendListModel *model)
		:	_model(model)
		{
		}
		virtual Wt::WWidget *update(Wt::WWidget *widget,const Wt::WModelIndex& index,Wt::WFlags<Wt::ViewItemRenderFlag> flags)
		{
			if(widget != NULL && dynamic_cast<Wt::WImage*>(widget) != NULL)
			{
				static_cast<Wt::WImage*>(widget)->setImageRef(_model->getAvatarUrl( index.row() )) ;
				return widget ;
			}
			else
				return new Wt::WImage(_model->getAvatarUrl( index.row() )) ;
		}

	private:
		FriendListModel *_model ;
};

RSWappFriendsPage::RSWappFriendsPage(Wt::WContainerWidget *parent,RsPeers *mpeers,RsMsgs *mmsgs)
	: WCompositeWidget(parent),mPeers(mpeers)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;

	_tableView = new Wt::WTableView(_impl);
	_tableView->setAlternatingRowColors(true);

	//tableView->setModel(fileFilterModel_);
	_tableView->setSelectionMode(Wt::ExtendedSelection);
	_tableView->setDragEnabled(true);

	_tableView->setColumnWidth(0, 200);
	_tableView->setColumnWidth(1, 150);
	_tableView->setColumnWidth(2, 250);
	_tableView->setColumnWidth(3, 150);
	_tableView->setColumnWidth(4, 150);
	_tableView->setColumnWidth(5, 100);

	_tableView->setModel(_model = new FriendListModel(mpeers,mmsgs)) ;
	_model->refresh() ;

	_tableView->setItemDelegateForColumn(COLUMN_AVATAR,new FriendPageAvatarDelegate(_model)) ;

	Wt::WPushButton *add_friend_button = new Wt::WPushButton("Add new friend") ;
	layout->addWidget(add_friend_button,1) ;

	layout->addWidget(_tableView) ;

	_popupMenu = NULL ;
	_tableView->mouseWentUp().connect(this,&RSWappFriendsPage::showCustomPopupMenu) ;

	add_friend_button->clicked().connect(this,&RSWappFriendsPage::addFriend) ;

	// add a button to add new friends.
	//
	_timer = new Wt::WTimer(this) ;

	_timer->setInterval(5000) ;
	_timer->timeout().connect(this,&RSWappFriendsPage::refresh) ;
	_timer->start() ;
}

void RSWappFriendsPage::showCustomPopupMenu(const Wt::WModelIndex& item, const Wt::WMouseEvent& event) 
{
	std::cerr << "Custom poopup menu requested." << std::endl;

	if (event.button() == Wt::WMouseEvent::LeftButton) 
	{
		// Select the item, it was not yet selected.
		//
		if (!_tableView->isSelected(item))
			_tableView->select(item);

		if (_popupMenu) 
			delete _popupMenu ;

		// request information about the hash

		std::string friend_id(boost::any_cast<Wt::WString>(_tableView->model()->data(item,Wt::UserRole)).toUTF8());

		_selected_friend = friend_id ;
		std::cerr << "Making menu for friend id " << friend_id << std::endl;

		_popupMenu = new Wt::WPopupMenu();

		_popupMenu->addItem("Show details");
		_popupMenu->addItem("Deny friend");

		/*
		 * This is one method of executing a popup, which does not block a
		 * thread for a reentrant event loop, and thus scales.
		 *
		 * Alternatively you could call WPopupMenu::exec(), which returns
		 * the result, but while waiting for it, blocks the thread.
		 */      
		_popupMenu->aboutToHide().connect(this, &RSWappFriendsPage::popupAction);
		_popupMenu->popup(event);

		std::cerr << "Popuping up menu!" << std::endl;
	}
}

void RSWappFriendsPage::refresh()
{
	std::cerr << "refreshing friends page" << std::endl;
	_model->refresh() ;
	_tableView->refresh();
}

void RSWappFriendsPage::addFriend()
{
}

void RSWappFriendsPage::showFriendDetails(const std::string& friend_id)
{
	RsPeerDetails info ;

	if(!mPeers->getPeerDetails(friend_id,info))
	{
		std::cerr << "Can't get file details for friend " << friend_id << std::endl;
		return ;
	}

	std::cerr << "Showing peer details: " << std::endl;
	std::cerr << info << std::endl;
}

void RSWappFriendsPage::popupAction() 
{
	if (_popupMenu->result()) 
	{
		/*
		 * You could also bind extra data to an item using setData() and
		 * check here for the action asked. For now, we just use the text.
		 */
		Wt::WString text = _popupMenu->result()->text();
		_popupMenu->hide();

		if(text == "Show details")
			showFriendDetails(_selected_friend) ;
		else if(text == "Deny friend")
		{
	RsPeerDetails info ;

	if(!mPeers->getPeerDetails(_selected_friend,info))
	{
		std::cerr << "Can't get file details for hash " << _selected_friend << std::endl;
		return ;
	}
			if(Wt::WMessageBox::show("Deny friend?", "<p>Do you really want to deny this friend? If so, you can add it back again later.\n\nPGP id: "+info.gpg_id+"\nName : " +info.name, Wt::Yes | Wt::No) == Wt::Yes)
				std::cerr << "Denying this friend" << std::endl;
			else
				std::cerr << "Keeping this friend" << std::endl;
		}
	} 
	else 
		_popupMenu->hide();
}


