#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>

#include <Wt/WTimer>
#include <Wt/WTextArea>
#include <Wt/WPopupMenu>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WImage>
#include <Wt/WMessageBox>
#include <Wt/WRasterImage>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
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
				return 6;
			else
				return 0;
		}

		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			if(index.column() >= 6 || index.row() >= (int)_friends.size())
				return boost::any();

			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						case COLUMN_AVATAR: return Wt::WString("") ;
						case COLUMN_NAME: return Wt::WString(_friends[index.row()].name) + " (" + Wt::WString(_friends[index.row()].location)+")" ;
                    case COLUMN_PGP_ID: return Wt::WString(_friends[index.row()].gpg_id.toStdString()) ;
                    case COLUMN_SSL_ID: return Wt::WString(_friends[index.row()].id.toStdString()) ;
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
                //return Wt::WString(_friends[index.row()].id.toStdString()) ;
                return _friends[index.row()].id ;

				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[6] = { Wt::WString(""),
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
		const std::string& getAvatarUrl(int row) const
		{
			static const std::string null_str ;

			if(row >= (int)_friend_avatars.size())
				return null_str ;
			else
				return _friend_avatars[row] ;
		}
        const std::string& getAvatarUrl(const RsPeerId& peer_id) const
		{
			static const std::string null_string("") ;

			for(uint32_t i=0;i<_friends.size();++i)
                if(peer_id == _friends[i].id)
					return getAvatarUrl(i) ;

			return null_string;
		}

	private:
		void updateFriendList() const
		{
			time_t now = time(NULL) ;

			if(now < 2+_last_time_update)
				return ;
#ifdef DEBUG_FRIENDSPAGE	
			std::cerr << "Updating list..." << std::endl;
#endif
			_last_time_update = now ;

            std::list<RsPeerId> fids ;
			
			if(!mPeers->getFriendList(fids)) 
				std::cerr << "(EE) " << __PRETTY_FUNCTION__ << ": can't get list of friends." << std::endl;

			_friends.clear() ;
			_friend_avatars.clear() ;

            for(std::list<RsPeerId>::const_iterator it(fids.begin());it!=fids.end();++it)
			{
				_friends.push_back(RsPeerDetails()) ;

				mPeers->getPeerDetails(*it,_friends.back()) ;

				unsigned char *data = NULL ;
				int size = 0 ;

				mMsgs->getAvatarData(*it,data,size) ;

				std::string base64_string ;
				Radix64::encode((const char *)data,size,base64_string) ;

				_friend_avatars.push_back(std::string("data:image/jpeg;base64,"+base64_string)) ;

#ifdef DEBUG_FRIENDSPAGE	
				std::cerr << "Got new avatar for friend " << *it << ": " << _friend_avatars.back() << std::endl;
#endif
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

class AddFriendDialog: public Wt::WDialog
{
	public:
		AddFriendDialog(RsPeers *mp) : mPeers(mp)
	{
		Wt::WVBoxLayout *layout = new Wt::WVBoxLayout ;
		contents()->setLayout(layout) ;

		// add a text box to paste the certificate into
		_cert_area = new Wt::WTextArea(contents()) ;
		_cert_area->setEmptyText("Paste a Retroshare certificate here to make friend with someone.") ;
		_cert_area->setMinimumSize(560,300) ;
		layout->addWidget(_cert_area,1) ;

		_cert_area->changed().connect(this,&AddFriendDialog::updateCertInfo) ;

		// add a text label to display the info
	Wt::WString str ;
	str += "Not a valid certificate<br/>" ;
	str += "<b>Name</b>   \t\t: <br/>" ;
	str += "<b>PGP id</b> \t\t: <br/>" ;
	str += "<b>PGP fingerprint</b> \t: <br/>" ;
	str += "<b>Location name  </b> \t: <br/>" ;
	str += "<b>Location ID    </b> \t: <br/>" ;

		_info_label = new Wt::WLabel(str,contents()) ;
		_info_label->setMinimumSize(128,0) ;

		layout->addWidget(_info_label) ;

		// add buttons 'make friend', 'only add to keyring', 'cancel'

		Wt::WHBoxLayout *lay2 = new Wt::WHBoxLayout;

		_ok_bnt = new Wt::WPushButton("Make friend", contents());
		lay2->addWidget(_ok_bnt) ;
		_ok_bnt->clicked().connect(this, &AddFriendDialog::makeFriend);
		_ok_bnt->setEnabled(false) ;

		Wt::WPushButton *cn_bnt = new Wt::WPushButton("Cancel", contents());
		lay2->addWidget(cn_bnt) ;
		cn_bnt->clicked().connect(this, &Wt::WDialog::reject);

		lay2->addStretch() ;

		layout->addLayout(lay2) ;
		layout->addSpacing(0);

		_timer = new Wt::WTimer(this) ;
		_timer->setInterval(1000) ;
		_timer->timeout().connect(this,&AddFriendDialog::updateCertInfo) ;
		_timer->start() ;

		setMinimumSize(620,300) ;
		//resize() ;
	}

		virtual ~AddFriendDialog()
		{
			delete _info_label ;
			delete _cert_area ;
		}

		// updates the text when the pasted key has been parsed/changed.

		void updateCertInfo()
		{
			std::cerr << "updating cert info" << std::endl;

			std::string cert_str = _cert_area->text().toUTF8() ;
			std::string clean_cert ;
			uint32_t error_code ;
			int error;

			if((!mPeers->cleanCertificate(cert_str,clean_cert,error)) || clean_cert.empty())
			{
				Wt::WString str ;
				str += "<h2>Not a valid certificate</h2>" ;
				str += "<b>Name</b>   \t\t: <br/>" ;
				str += "<b>PGP id</b> \t\t: <br/>" ;
				str += "<b>PGP fingerprint</b> \t: <br/>" ;
				str += "<b>Location name  </b> \t: <br/>" ;
				str += "<b>Location ID    </b> \t: <br/>" ;

				_info_label->setText(str) ;
				_ok_bnt->setEnabled(false) ;
				return ;
			}
			RsPeerDetails pd ;

			if(mPeers->loadDetailsFromStringCert(clean_cert,pd,error_code)) 
			{
				Wt::WString str ;

				std::string new_name ;
				for(uint32_t i=0;i<pd.name.length();++i)
					if(pd.name[i] == '<')
						new_name += "&lt;" ;
					else if(pd.name[i] == '>')
						new_name += "&gt;" ;
					else
						new_name += pd.name[i] ;

				str += "<h2>Certificate info</h2>" ;
				str += "<b>Name</b>   \t\t: " + new_name + "<br/>" ;
                str += "<b>PGP id</b> \t\t: " + pd.gpg_id.toStdString() + "<br/>" ;
				str += "<b>Location name  </b> \t: " + pd.location +" <br/>" ;
                str += "<b>Location ID    </b> \t: " + pd.id.toStdString() + " <br/>" ;
				_ok_bnt->setEnabled(true) ;

				_info_label->setText(str) ;
			}
			else
			{
				Wt::WString str ;
				str += "<h2>Not a valid certificate</h2>" ;
				str += "<b>Name</b>   \t\t: <br/>" ;
				str += "<b>PGP id</b> \t\t: <br/>" ;
				str += "<b>PGP fingerprint</b> \t: <br/>" ;
				str += "<b>Location name  </b> \t: <br/>" ;
				str += "<b>Location ID    </b> \t: <br/>" ;
				_ok_bnt->setEnabled(false) ;

				_info_label->setText(str) ;
				return ;
			}
		}

		void makeFriend()
		{
			std::cerr << "making friend. " << std::endl;
			int error ;
            RsPeerId ssl_id ;
            RsPgpId pgp_id ;

			std::string cert_str = _cert_area->text().toUTF8() ;
			std::string clean_cert ;
			uint32_t error_code ;
			std::string error_str ;

			if((!mPeers->cleanCertificate(cert_str,clean_cert,error)) || clean_cert.empty())
			{
				Wt::WString str ;
				str += "<h2>Not a valid certificate</h2>" ;
				str += "<b>Name</b>   \t\t: <br/>" ;
				str += "<b>PGP id</b> \t\t: <br/>" ;
				str += "<b>PGP fingerprint</b> \t: <br/>" ;
				str += "<b>Location name  </b> \t: <br/>" ;
				str += "<b>Location ID    </b> \t: <br/>" ;

				_info_label->setText(str) ;
				_ok_bnt->setEnabled(false) ;
				return ;
			}
			if(! mPeers->loadCertificateFromString(clean_cert,ssl_id,pgp_id,error_str))
			{
				Wt::WMessageBox::show("Certificate error", "The certificate could not be loaded.\n\nError : "+error_str, Wt::Ok);
				return ;
			}

			if(! mPeers->addFriend(ssl_id,pgp_id))
			{
				Wt::WMessageBox::show("Certificate error", "The friend could not be added.)", Wt::Ok) ;
				return ;
			}

            Wt::WMessageBox::show("Added friend", "This friend has been successfuly added<br/>PGP id: "+pgp_id.toStdString()+"<br/>Location id : " +ssl_id.toStdString(), Wt::Ok);
			accept() ;
		}

	private:
		Wt::WLabel *_info_label ;
		Wt::WTextArea *_cert_area ;
		Wt::WTimer *_timer ;
		Wt::WPushButton *_ok_bnt ;
		RsPeers *mPeers ;
};

RSWappFriendsPage::RSWappFriendsPage(RsPeers *mpeers, RsMsgs *mmsgs, Wt::WContainerWidget *parent)
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

	_tableView->setColumnWidth(COLUMN_AVATAR,  20);
	_tableView->setColumnWidth(COLUMN_NAME  , 200);
	_tableView->setColumnWidth(COLUMN_PGP_ID, 150);
	_tableView->setColumnWidth(COLUMN_SSL_ID, 250);
	_tableView->setColumnWidth(COLUMN_LAST_S, 150);
	_tableView->setColumnWidth(COLUMN_IP    , 150);

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
#ifdef DEBUG_FRIENDSPAGE	
	std::cerr << "Custom poopup menu requested." << std::endl;
#endif

	if (event.button() == Wt::WMouseEvent::LeftButton) 
	{
		// Select the item, it was not yet selected.
		//
		if (!_tableView->isSelected(item))
			_tableView->select(item);

		if (_popupMenu) 
			delete _popupMenu ;

		// request information about the hash

        //std::string friend_id(boost::any_cast<Wt::WString>(_tableView->model()->data(item,Wt::UserRole)).toUTF8());
        RsPeerId friend_id = boost::any_cast<RsPeerId>(_tableView->model()->data(item,Wt::UserRole));

		_selected_friend = friend_id ;
#ifdef DEBUG_FRIENDSPAGE	
		std::cerr << "Making menu for friend id " << friend_id << std::endl;
#endif

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

#ifdef DEBUG_FRIENDSPAGE	
		std::cerr << "Popuping up menu!" << std::endl;
#endif
	}
}

void RSWappFriendsPage::refresh()
{
#ifdef DEBUG_FRIENDSPAGE	
	std::cerr << "refreshing friends page" << std::endl;
#endif
	_model->refresh() ;
	_tableView->refresh();
}

void RSWappFriendsPage::addFriend()
{
	// Popup a window to paste the friends' certificate. Also include a few info about the cert, if it's correct.

	AddFriendDialog(mPeers).exec() ;
}

void RSWappFriendsPage::showFriendDetails(const RsPeerId& friend_id)
{
	RsPeerDetails info ;

	if(!mPeers->getPeerDetails(friend_id,info))
	{
		std::cerr << "Can't get file details for friend " << friend_id << std::endl;
		return ;
	}

#ifdef DEBUG_FRIENDSPAGE	
	std::cerr << "Showing peer details: " << std::endl;
	std::cerr << info << std::endl;
#endif

	Wt::WDialog dialog ;
	dialog.setModal(false) ;

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout ;
	dialog.contents()->setLayout(layout) ;

	Wt::WHBoxLayout *layout2 = new Wt::WHBoxLayout ;
	Wt::WImage *img = new Wt::WImage(_model->getAvatarUrl(friend_id),dialog.contents());

	img->setMinimumSize(128,128) ;
	img->setMaximumSize(128,128) ;

	layout2->addWidget(img) ;
	layout2->addStretch() ;

	layout->addLayout(layout2,1) ;

	Wt::WString str ;
	str += "<br/>" ;
	str += "<b>Name</b>   \t\t: " + info.name + "<br/>" ;
    str += "<b>PGP id</b> \t\t: " + info.gpg_id.toStdString() + "<br/>" ;
    str += "<b>PGP fingerprint</b> \t: " + info.fpr.toStdString() + "<br/>" ;
	str += "<b>Location name  </b> \t: " + info.location + "<br/>" ;
    str += "<b>Location ID    </b> \t: " + info.id.toStdString() + "<br/>" ;

	layout->addWidget(new Wt::WLabel(str,dialog.contents())) ;

	Wt::WPushButton ok("Ok", dialog.contents());
	layout->addWidget(&ok) ;

	ok.clicked().connect(&dialog, &Wt::WDialog::accept);

	dialog.exec() ;
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
            if(Wt::WMessageBox::show("Deny friend?", "Do you really want to deny this friend? If so, you can add it back again later.\n\nPGP id: "+info.gpg_id.toStdString()+"\nName : " +info.name, Wt::Yes | Wt::No) == Wt::Yes)
				std::cerr << "Denying this friend" << std::endl;
			else
				std::cerr << "Keeping this friend" << std::endl;
		}
	} 
	else 
		_popupMenu->hide();
}


