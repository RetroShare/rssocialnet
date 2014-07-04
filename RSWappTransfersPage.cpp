#include <stdio.h>
#include <retroshare/rsfiles.h>

#include <Wt/WTimer>
#include <Wt/WText>
#include <Wt/WVBoxLayout>
#include <Wt/WCheckBox>
#include <Wt/WModelIndex>
#include <Wt/WPopupMenu>
#include <Wt/WTextArea>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>

#include "RSWappTransfersPage.h"

//static const uint32_t COLUMN_ACTIONS    = 0 ;
static const uint32_t COLUMN_FILENAME   = 0 ;
static const uint32_t COLUMN_FILESIZE   = 1 ;
static const uint32_t COLUMN_TRANSFERED = 2 ;
static const uint32_t COLUMN_SPEED      = 3 ;
static const uint32_t COLUMN_SOURCES    = 4 ;

static Wt::WString make_big_number(uint64_t n)
{
	if(n >= 1000000)
		return Wt::WString("{1}{2}").arg((int)(n/1000000)).arg((int)(n%1000000)) ;
	else
		return Wt::WString("{1}").arg((int)n) ;
}

static Wt::WString number_round(float f)
{
	std::cerr << "number rounding " << f << " = " << Wt::WString("{1}.{2}").arg((int)f).arg(((int)(f*100))%100) << std::endl;
	return Wt::WString("{1}.{2}").arg((int)f).arg(((int)(f*100))%100);
}

class DownloadsTransfersListModel : public Wt::WAbstractTableModel
{
	public:
		DownloadsTransfersListModel(RsFiles *mfiles,Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent), mFiles(mfiles)
		{
			_last_time_update = 0 ;
			_show_cache_transfers = true ;

			updateTransfersList() ;
		}

		void toggleShowCacheTransfers() 
		{ 
			_show_cache_transfers = !_show_cache_transfers ; 
			refresh() ;
		}
		bool showCacheTransfers() const { return _show_cache_transfers ; }

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
            //std::cerr << "asked for row count. Returning " << _downloads.size() << std::endl;

			return _downloads.size() ;
		}

		virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
            //std::cerr << "asked for column count" << std::endl;
			
			return 5;
		}

		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			if(index.column() >= 5 || index.row() >= (int)_downloads.size())
				return boost::any();

			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						//case COLUMN_ACTIONS   : return boost::any() ;
						case COLUMN_FILENAME  : return Wt::WString(_downloads[index.row()].fname) ;
						case COLUMN_FILESIZE  : return make_big_number(_downloads[index.row()].size) ;
						case COLUMN_TRANSFERED: return make_big_number(_downloads[index.row()].transfered) 
																					+ " (" + number_round(_downloads[index.row()].transfered
																									/(double)_downloads[index.row()].size*100.0f) + " %)";
						case COLUMN_SPEED     : return number_round(_downloads[index.row()].tfRate)+" Kb/s";
						case COLUMN_SOURCES   : return Wt::WString("{1}").arg((int)_downloads[index.row()].peers.size()) ;
						default:
									  return Wt::WString("Not connected") ;
					}
				case Wt::UserRole:
					switch(index.column())
					{
                    default: return _downloads[index.row()].hash ;
					}
				case Wt::ToolTipRole:
                return Wt::WString(_downloads[index.row()].hash.toStdString()) ;
				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[5] = { //Wt::WString("*"),
															Wt::WString("File name"),
															Wt::WString("Size"),
															Wt::WString("Transfered"),
															Wt::WString("Transfer speed"), 
															Wt::WString("Sources") } ;

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

		virtual void refresh()
		{
			updateTransfersList() ;
            //std::cerr << "Updating transfers model rows=" << rowCount() << ", columns=" << columnCount()  << std::endl;

			dataChanged().emit(index(0,0),index(rowCount()-1,columnCount()-1)) ;
		}
	private:
		void updateTransfersList() const
		{
			time_t now = time(NULL) ;

			//if(now < 2+_last_time_update)
			//	return ;

            //std::cerr << "Updating transfers list..." << std::endl;
			_last_time_update = now ;

            std::list<RsFileHash> hashes ;

			if(!mFiles->FileDownloads(hashes)) 
				std::cerr << "(EE) " << __PRETTY_FUNCTION__ << ": can't get list of downloads." << std::endl;

			_downloads.clear() ;

            for(std::list<RsFileHash>::const_iterator it(hashes.begin());it!=hashes.end();++it)
			{
				FileInfo info ;	// needs to be here, so that lists are empty.

				if(mFiles->FileDetails(*it,RS_FILE_HINTS_DOWNLOAD,info))
				{
					if(_show_cache_transfers || !(info.transfer_info_flags & RS_FILE_REQ_CACHE))
					{
						_downloads.push_back(info) ;
						std::cerr << "info.tfRate = " << info.tfRate*1024 << std::endl;
					}
				}
				else
					std::cerr << "Warning: can't get info for downloading hash " << *it << std::endl;
			}

			//		for(std::list<std::string>::const_iterator it(hashes.begin());it!=hashes.end();++it)
			//			if(FileDetails(*it,RS_FILE_HINTS_UPLOAD,info))
			//				_uploads.push_back(info) ;
			//			else
			//				std::cerr << "Warning: can't get info for uploading hash " << *it << std::endl;

            //std::cerr << _downloads.size() << " transfers in list." << std::endl;
		}

		mutable std::vector<FileInfo> _downloads ;
		mutable time_t _last_time_update ;
		mutable bool _show_cache_transfers ;

		RsFiles *mFiles ;
};

RSWappTransfersPage::RSWappTransfersPage(RsFiles *mfiles, Wt::WContainerWidget *parent)
	: WCompositeWidget(parent),mFiles(mfiles)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	_tableView = new Wt::WTableView(_impl);

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;

	_tableView->setAlternatingRowColors(true);

	_tableView->setSelectionMode(Wt::ExtendedSelection);
	_tableView->setDragEnabled(true);

	//_tableView->setColumnWidth(COLUMN_ACTIONS   ,  20);
	_tableView->setColumnWidth(COLUMN_FILENAME  , 400);
	_tableView->setColumnWidth(COLUMN_FILESIZE  , 150);
	_tableView->setColumnWidth(COLUMN_TRANSFERED, 150);
	_tableView->setColumnWidth(COLUMN_SPEED     , 150);
	_tableView->setColumnWidth(COLUMN_SOURCES   , 150);

	_download_model = new DownloadsTransfersListModel(mfiles) ;

	_tableView->setModel(_download_model) ;
	layout->addWidget(_tableView,1) ;
	_tableView->setHeight(300) ;
	_popupMenu = NULL ;

	_tableView->mouseWentUp().connect(this,&RSWappTransfersPage::showCustomPopupMenu) ;

	Wt::WCheckBox *cb = new Wt::WCheckBox(Wt::WString("Show cache transfers"),_impl) ;
	cb->changed().connect(this,&RSWappTransfersPage::toggleShowCacheTransfers) ;
	layout->addWidget(cb) ;

	cb->setChecked(_download_model->showCacheTransfers());

	link_area = new Wt::WTextArea(_impl) ;
	link_area->setEmptyText("Paste Retroshare links here to download them,\nand press Parse to parse the links and download the files.") ;
	link_area->setHeight(100) ;
	layout->addWidget(link_area) ;

	Wt::WPushButton *btn = new Wt::WPushButton("Parse!") ;
	btn->clicked().connect(this,&RSWappTransfersPage::downloadLink) ;
	layout->addWidget(btn) ;

	_timer = new Wt::WTimer(this) ;

	_timer->setInterval(3000) ;
	_timer->timeout().connect(this,&RSWappTransfersPage::refresh) ;
	_timer->start() ;
}

void RSWappTransfersPage::refresh()
{
	_download_model->refresh();
	_tableView->setHeight(_tableView->height());
	//_tableView->scheduleRender();
}

void RSWappTransfersPage::showCustomPopupMenu(const Wt::WModelIndex& item, const Wt::WMouseEvent& event) 
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

        RsFileHash hash = boost::any_cast<RsFileHash>(_tableView->model()->data(item,Wt::UserRole));

		_selected_hash = hash ;
		std::cerr << "Making menu for hash " << hash << std::endl;

		FileInfo info ;
		if(!mFiles->FileDetails(hash,RS_FILE_HINTS_DOWNLOAD,info))
		{
            std::cerr << "Can't get file details for hash " << hash.toStdString() << std::endl;
			return ;
		}
		_popupMenu = new Wt::WPopupMenu();
		_popupMenu->addItem("Cancel");

		if((uint32_t)info.downloadStatus == FT_STATE_DOWNLOADING)
		{
			_popupMenu->addItem("Pause");
		}
		if((uint32_t)info.downloadStatus == FT_STATE_PAUSED)
		{
			_popupMenu->addItem("Resume");
		}

		// Wt::WPopupMenu *subMenu = new Wt::WPopupMenu();
		// subMenu->addItem("Streaming");
		// subMenu->addItem("Progressive");
		// subMenu->addItem("Random");
		// subMenu->addItem("Sub Item 2");
		// _popupMenu->addMenu("Chunk strategy",subMenu);

		/*
		 * This is one method of executing a popup, which does not block a
		 * thread for a reentrant event loop, and thus scales.
		 *
		 * Alternatively you could call WPopupMenu::exec(), which returns
		 * the result, but while waiting for it, blocks the thread.
		 */      
		_popupMenu->aboutToHide().connect(this, &RSWappTransfersPage::popupAction);
		_popupMenu->popup(event);

		std::cerr << "Popuping up menu!" << std::endl;
	}
}

void RSWappTransfersPage::popupAction() 
{
	if (_popupMenu->result()) 
	{
		/*
		 * You could also bind extra data to an item using setData() and
		 * check here for the action asked. For now, we just use the text.
		 */
		Wt::WString text = _popupMenu->result()->text();
		_popupMenu->hide();

		if(text == "Cancel")
		{
            if(Wt::WMessageBox::show("Cancel transfer?", "<p>Do you really want to cancel this file (hash="+_selected_hash.toStdString()+")</p>", Wt::Ok | Wt::No) == Wt::Ok)
				mFiles->FileCancel(_selected_hash) ;
		}
		else if(text == "Pause")
			mFiles->FileControl(_selected_hash,RS_FILE_CTRL_PAUSE) ;
		else if(text == "Resume")
			mFiles->FileControl(_selected_hash,RS_FILE_CTRL_START) ;
		else if(text == "Show cache transfers")
			_download_model->toggleShowCacheTransfers();

		//popupActionBox_->buttonClicked().connect(this, &TreeViewDragDrop::dialogDone);
		//popupActionBox_->show();
	} 
	else 
		_popupMenu->hide();
}

void RSWappTransfersPage::toggleShowCacheTransfers()
{
	std::cerr << "Toggling show cache transfers." << std::endl;
	_download_model->toggleShowCacheTransfers() ;
}

void RSWappTransfersPage::downloadLink()
{
	std::string lstr = link_area->text().toUTF8() ;

	std::cerr << "Parsing links:" << std::endl;
	std::cerr << lstr << std::endl;

	std::vector<std::string> names ;
    std::vector<RsFileHash> hashs ;
	std::vector<uint64_t   > sizes ;

	// parse the link. We can do better, and for now this is really basic stuff.
	//
	size_t current_position = 0 ;

	while(true)
	{
		// look for the next occurence of "retroshare://file?"
		
		size_t s = lstr.find("retroshare://file?",current_position) ;

		if(s == std::string::npos)
			break ;

		// split into chunks separated by &

		size_t s2 = lstr.find_first_of('&',s) ;

		if(s2 == std::string::npos)
			break ;

		std::string name_str = lstr.substr(s+18+std::string("name=").length(),s2-s-18-std::string("name=").length()) ;
		
		size_t s3 = lstr.find_first_of('&',s2+1) ;
		if(s3 == std::string::npos)
			break ;

		uint64_t size ;
		if(sscanf(lstr.substr(s2+1,s3-s2-1).c_str(),"size=%lu",&size) != 1)
			break ;

		size_t s4 = lstr.find_first_not_of("0123456789abcdef",s3+1+std::string("hash=").length()) ;

		if(s == std::string::npos)
			break ;

		std::string hash_str = lstr.substr(s3+1+std::string("hash=").length(),s4-s3-1-std::string("hash=").length()) ;

		std::cerr << "s = " << s << ", s2=" << s2 << ", s3=" << s3 << ",s4=" << s4 << std::endl;
		std::cerr << "name: " << name_str << std::endl;
		std::cerr << "hash: " << hash_str << std::endl;
		std::cerr << "size: " << size << std::endl;

		// extract field for name, size and hash
		//
		current_position = s4 ;

		names.push_back(name_str) ;
        hashs.push_back(RsFileHash(hash_str)) ;
		sizes.push_back(size) ;
	}

	if(names.size() > 0)
	{
		Wt::WText *out = new Wt::WText(_impl);
		out->setMargin(10, Wt::Left);

		Wt::WString num_files = Wt::WString("{1}").arg((int)names.size()) ;

		Wt::WString files_str = "<ul>" ;
		for(uint32_t i=0;i<names.size();++i)
            files_str += "<li>" + hashs[i].toStdString() + ", " + Wt::WString("{1}").arg((int)(sizes[i]/1000)) + Wt::WString("{1}").arg((int)(sizes[i]%1000)) + " bytes, name: " + names[i] + "</li>" ;

		files_str += "</ul>";

		Wt::StandardButton answer = Wt::WMessageBox::show("Download these files?", "<p>Download these " +num_files+" files?</p>"+files_str, Wt::Ok | Wt::Cancel);

		if (answer == Wt::Ok)
		{
            std::list<RsPeerId> srcids ;

			for(uint32_t i=0;i<names.size();++i)
                mFiles->FileRequest(names[i],hashs[i],sizes[i],"",RS_FILE_REQ_ANONYMOUS_ROUTING,srcids) ;

			link_area->setText("") ;
		}
		else
		{
			out->setText("Waiting on your decision...");
		}
	}
	else
		Wt::WMessageBox::show("No file parsed","No retroshare links found in supplied test. Sorry!", Wt::Ok);

	refresh();
}

