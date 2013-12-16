#include <Wt/WContainerWidget>
#include <Wt/WAbstractTableModel>
#include <Wt/WVBoxLayout>
#include <Wt/WTreeView>
#include <Wt/WTableView>
#include <Wt/WTextArea>
#include <Wt/WTextEdit>
#include <Wt/WPushButton>
#include <Wt/WModelIndex>
#include <Wt/WTimer>
#include <Wt/WLineEdit>
#include <Wt/WCheckBox>

#include "RSWappSearchFilesPage.h"

#include <retroshare/rsfiles.h>
#include <retroshare/rstypes.h>
#include <retroshare/rsturtle.h>
#include <retroshare/rsnotify.h>

#define COLUMN_FILENAME 0
#define COLUMN_SIZE     1
#define COLUMN_AGE      2
#define COLUMN_FLAGS    3
#define COLUMN_GROUPS   4

static Wt::WString make_big_number(uint64_t n)
{
	if(n >= 1000000)
		return Wt::WString("{1}{2}").arg((int)(n/1000000)).arg((int)(n%1000000)) ;
	else
		return Wt::WString("{1}").arg((int)n) ;
}

class LocalSearchFilesModel: public Wt::WAbstractTableModel
{
	public:
		LocalSearchFilesModel(RsFiles *mfiles,Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent), mFiles(mfiles)
		{
		}

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return _searchResults.size() ;
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
			std::cerr << "data row: " << index.row() << std::endl;
			if(index.column() >= 6 || index.row() >= (int)_searchResults.size())
				return boost::any();

			//DirDetails dd;
			//dd.count;
			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						case COLUMN_FILENAME  : return Wt::WString(_searchResults[index.row()].name) ;
						case COLUMN_SIZE  : return make_big_number(_searchResults[index.row()].count) ;
						case COLUMN_AGE: return make_big_number(_searchResults[index.row()].age);
						default:
														return boost::any();
					}
				case Wt::UserRole:
					switch(index.column())
					{

					default: return Wt::WString(_searchResults[index.row()].hash) ;
					}
				case Wt::ToolTipRole:
						Wt::WString(_searchResults[index.row()].hash) ;
				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section, Wt::Orientation orientation = Wt::Horizontal, int role = Wt::DisplayRole) const
		{
			static Wt::WString col_names[6] = { 
				Wt::WString("File Name"),
				Wt::WString("Size"),
				Wt::WString("Age"),
				Wt::WString("*"),
				Wt::WString("*"),
				Wt::WString("*") } ;

			//updateTransfersList() ;

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
		std::list<DirDetails> getItems(std::list<int> jobList) const{
			std::list<DirDetails> items;
		    for(std::list<int>::iterator resultsIter = jobList.begin(); resultsIter != jobList.end(); resultsIter ++)
		    {
		        int index = *resultsIter;
				items.push_back(_searchResults[index]) ;
			}
			return items;
		}
		void displayList(std::list<DirDetails> dList) const
		{
			_searchResults.clear() ;

			DirDetails dd;
		    for(std::list<DirDetails>::iterator resultsIter = dList.begin(); resultsIter != dList.end(); resultsIter ++)
		    {
		        std::cerr << "RSWUI search result: " << dd.hash << std::endl;
		        dd = *resultsIter;
				_searchResults.push_back(dd) ;
			}
			std::cerr << "Updating search model rows=" << rowCount() << ", columns=" << columnCount()  << std::endl;

		}

		virtual void refresh()
		{
			std::cerr << "Updating search model rows=" << rowCount() << ", columns=" << columnCount()  << std::endl;

			dataChanged().emit(index(0,0),index(rowCount(),columnCount())) ;
		}
	private:

		mutable std::vector<DirDetails> _searchResults ;

		RsFiles *mFiles ;
};

RSWappSearchFilesPage::RSWappSearchFilesPage(Wt::WContainerWidget *parent,RsFiles *mfiles)
	: WCompositeWidget(parent),mFiles(mfiles)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;



	//_treeView = new Wt::WTreeView(_impl);
	_tableView = new Wt::WTableView(_impl);

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;


	search_box = new Wt::WLineEdit(_impl) ;
	search_box->setText("mp3") ;
	//search_box->setHeight(50) ;
	layout->addWidget(search_box) ;

	localcb = new Wt::WCheckBox(Wt::WString("Search Local"),_impl) ;
	layout->addWidget(localcb);
	remotecb = new Wt::WCheckBox(Wt::WString("Search Remote"),_impl) ;
	layout->addWidget(remotecb);
	distantcb = new Wt::WCheckBox(Wt::WString("Search Distant"),_impl) ;
	layout->addWidget(distantcb);

	Wt::WPushButton *btn = new Wt::WPushButton("Search!") ;
	btn->clicked().connect(this,&RSWappSearchFilesPage::searchClicked) ;
	layout->addWidget(btn) ;

	_tableView->setAlternatingRowColors(true);

	_tableView->setSelectionMode(Wt::ExtendedSelection);
	_tableView->setDragEnabled(true);


	_tableView->setColumnWidth(0, 250);
	_tableView->setColumnWidth(1, 150);
	_tableView->setColumnWidth(2, 250);
	_tableView->setColumnWidth(3, 150);
	_tableView->setColumnWidth(4, 150);
	_tableView->setColumnWidth(5, 100);

	_shared_files_model = new LocalSearchFilesModel(mfiles) ;

	_tableView->setModel(_shared_files_model) ;

	_tableView->doubleClicked().connect(this,&RSWappSearchFilesPage::tableClicked) ;
	layout->addWidget(_tableView,1) ;

	_tableView->setHeight(300) ;


	Wt::WPushButton *dlbtn = new Wt::WPushButton("Download selected") ;
	dlbtn->clicked().connect(this,&RSWappSearchFilesPage::searchClicked) ;
	layout->addWidget(dlbtn) ;


	searchClicked();
}

void RSWappSearchFilesPage::tableClicked()
{
	//_tableView->selectedIndexes().begin().
	Wt::WModelIndex index;
	std::list<int> jobList;
	const Wt::WModelIndexSet selectedRows = _tableView->selectedIndexes();
	for (Wt::WModelIndexSet::iterator i = selectedRows.begin();
		i != selectedRows.end(); ++i) {
			index = *i;
			jobList.push_back(index.row());
	}
	DirDetails dd;
	std::list<DirDetails> items = _shared_files_model->getItems(jobList);
	for (std::list<DirDetails>::iterator i = items.begin();
		i != items.end(); ++i) {
			dd = *i;

			FileInfo finfo ;
		    rsFiles->FileDetails(dd.hash, RS_FILE_HINTS_REMOTE, finfo) ;

		    std::list<std::string> srcIds;
		    for(std::list<TransferInfo>::const_iterator it(finfo.peers.begin());it!=finfo.peers.end();++it)
		    {
		        srcIds.push_back((*it).peerId) ;
		    }

		    if (rsFiles->FileRequest(dd.name, dd.hash, dd.count, "", RS_FILE_REQ_ANONYMOUS_ROUTING, srcIds)) {
				std::cerr << "\n\n DOWNLOADING: " << dd.name << ", " << dd.hash << ", " << dd.count <<std::endl;
		    } else {
				std::cerr << "\n\n SKIPDL: " << dd.name << ", " << dd.hash << ", " << dd.count <<std::endl;
		        //fileExist.append(link.name());
		    }
	}
	//dryRunSignal().emit(jobList);
}

void RSWappSearchFilesPage::searchClicked()
{
	std::cerr << "SEARCHCLICKED"<<std::endl;
	std::string lstr = search_box->text().toUTF8() ;
	std::istringstream iss(lstr);
	/*std::vector<std::string> tokens;
	tokens = {std::istream_iterator<std::string>{iss},
	         std::istream_iterator<std::string>{}};*/

	std::list<std::string> strings;
    std::string s;
    while (std::getline(iss, s, ' ')) {
        std::cout << s << std::endl;
        strings.push_back(s);
    }

	/*TurtleRequestId req_id ;

    if(distantcb->checkState())
    {
        req_id = rsTurtle->turtleSearch(strings.front()) ;
    }*/


	std::list<DirDetails> results;
	if(localcb->checkState()){
		FileSearchFlags fsf = RS_FILE_HINTS_LOCAL;
		std::list<DirDetails> initialResults;
		mFiles->SearchKeywords(strings,initialResults,fsf);
		std::cerr << "RESULTLEN: "<< initialResults.size() <<std::endl;

		for (std::list<DirDetails>::iterator i = initialResults.begin(); i != initialResults.end(); ++i)
			results.push_back(*i);
	}
	if(remotecb->checkState()){
		FileSearchFlags fsf = RS_FILE_HINTS_REMOTE;
		std::list<DirDetails> initialResults;
		mFiles->SearchKeywords(strings,initialResults,fsf);
		std::cerr << "RESULTLEN: "<< initialResults.size() <<std::endl;

		for (std::list<DirDetails>::iterator i = initialResults.begin(); i != initialResults.end(); ++i)
			results.push_back(*i);
	}
	_shared_files_model->displayList(results);
	refresh();

}

void RSWappSearchFilesPage::refresh() {
	_shared_files_model->refresh();
	_tableView->refresh();
	_tableView->setHeight(300);//Without this line - wt wont update for me (wt3.3)

}
