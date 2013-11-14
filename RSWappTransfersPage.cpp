
#include <retroshare/rsfiles.h>

#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>

#include "RSWappTransfersPage.h"

class DownloadsTransfersListModel : public Wt::WAbstractTableModel
{
	public:
		DownloadsTransfersListModel(RsFiles *mfiles,Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent), mFiles(mfiles)
		{
			_last_time_update = 0 ;
		}

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return _downloads.size() ;
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
			if(index.column() >= 5 || index.row() >= _downloads.size())
				return boost::any();

			switch (role) 
			{
				case Wt::DisplayRole:
//					switch(index.column())
//					{
//						case 0: return Wt::WString(_friends[index.row()].name) ;
//						case 1: return Wt::WString(_friends[index.row()].gpg_id) ;
//						case 2: return Wt::WString(_friends[index.row()].id) ;
//						case 3: return lastSeenString(_friends[index.row()].lastConnect) ;
//						case 4: 
//								  if(_friends[index.row()].state & RS_PEER_STATE_CONNECTED)
//								  {
//									  std::string s_ip = _friends[index.row()].extAddr ;
//
//									  return Wt::WString(_friends[index.row()].extAddr + ":{1}").arg(_friends[index.row()].extPort) ;
//								  }
//								  else
									  return Wt::WString("Not connected") ;
//					}
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
			updateTransfersList() ;

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
		void updateTransfersList() const
		{
			time_t now = time(NULL) ;

			if(now < 2+_last_time_update)
				return ;
			
			std::cerr << "Updating transfers list..." << std::endl;
			_last_time_update = now ;

			std::list<std::string> hashes ;
			
				if(!mFiles->FileDownloads(hashes)) 
					std::cerr << "(EE) " << __PRETTY_FUNCTION__ << ": can't get list of downloads." << std::endl;

				_downloads.clear() ;

				FileInfo info ;

				for(std::list<std::string>::const_iterator it(hashes.begin());it!=hashes.end();++it)
					if(mFiles->FileDetails(*it,RS_FILE_HINTS_DOWNLOAD,info))
						_downloads.push_back(info) ;
					else
						std::cerr << "Warning: can't get info for downloading hash " << *it << std::endl;

		//		if(!mFiles->FileUploads(hashes)) 
		//			std::cerr << "(EE) " << __PRETTY_FUNCTION__ << ": can't get list of uploads." << std::endl;

		//		for(std::list<std::string>::const_iterator it(hashes.begin());it!=hashes.end();++it)
		//			if(FileDetails(*it,RS_FILE_HINTS_UPLOAD,info))
		//				_uploads.push_back(info) ;
		//			else
		//				std::cerr << "Warning: can't get info for uploading hash " << *it << std::endl;
		}

		mutable std::vector<FileInfo> _downloads ;
		mutable time_t _last_time_update ;

		RsFiles *mFiles ;
};

RSWappTransfersPage::RSWappTransfersPage(Wt::WContainerWidget *parent,RsFiles *mfiles)
	: WCompositeWidget(parent),mFiles(mfiles)
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

	tableView->setModel(new DownloadsTransfersListModel(mfiles)) ;
}
