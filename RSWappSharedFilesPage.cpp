#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>
#include <Wt/WVBoxLayout>
#include <Wt/WTreeView>

#include "RSWappSharedFilesPage.h"

#include <retroshare/rsfiles.h>
#include <retroshare/rstypes.h>

#define COLUMN_FILENAME 0
#define COLUMN_SIZE     1
#define COLUMN_AGE      2
#define COLUMN_FLAGS    3
#define COLUMN_GROUPS   4

class LocalSharedFilesModel: public Wt::WAbstractTableModel
{
	public:
		LocalSharedFilesModel(RsFiles *mfiles,Wt::WObject *parent = 0)
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
				return 6;
			else
				return 0;
		}

		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			if(index.column() >= 6 || index.row() >= (int)_downloads.size())
				return boost::any();

			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						default:
														return boost::any();
					}
				case Wt::UserRole:
					switch(index.column())
					{
						default: 
														return boost::any();
					}
				case Wt::ToolTipRole:
														return boost::any();
				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[6] = { 
				Wt::WString("*"),
				Wt::WString("File name"),
				Wt::WString("Size"),
				Wt::WString("Age"),
				Wt::WString("Flags"), 
				Wt::WString("Groups") } ;

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
				{
					if(_show_cache_transfers || !(info.transfer_info_flags & RS_FILE_REQ_CACHE))
						_downloads.push_back(info) ;
				}
				else
					std::cerr << "Warning: can't get info for downloading hash " << *it << std::endl;

			//		for(std::list<std::string>::const_iterator it(hashes.begin());it!=hashes.end();++it)
			//			if(FileDetails(*it,RS_FILE_HINTS_UPLOAD,info))
			//				_uploads.push_back(info) ;
			//			else
			//				std::cerr << "Warning: can't get info for uploading hash " << *it << std::endl;
		}

		mutable std::vector<FileInfo> _downloads ;
		mutable time_t _last_time_update ;
		mutable bool _show_cache_transfers ;

		RsFiles *mFiles ;
};

RSWappSharedFilesPage::RSWappSharedFilesPage(Wt::WContainerWidget *parent,RsFiles *mfiles)
	: WCompositeWidget(parent),mFiles(mfiles)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	_treeView = new Wt::WTreeView(_impl);

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;

	_treeView->setAlternatingRowColors(true);

	_treeView->setSelectionMode(Wt::ExtendedSelection);
	_treeView->setDragEnabled(true);

	_shared_files_model = new LocalSharedFilesModel(mfiles) ;

	_treeView->setModel(_shared_files_model) ;
	layout->addWidget(_treeView,1) ;

	_treeView->setHeight(300) ;
}
