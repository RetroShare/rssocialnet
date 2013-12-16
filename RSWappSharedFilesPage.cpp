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

#include "RSWappSharedFilesPage.h"

#include <retroshare/rsfiles.h>
#include <retroshare/rstypes.h>

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

class LocalSharedFilesModel: public Wt::WAbstractTableModel
{
	public:
		LocalSharedFilesModel(RsFiles *mfiles,Wt::WObject *parent = 0)
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
			//dd.;
			switch (role) 
			{
				case Wt::DisplayRole:
					switch(index.column())
					{
						case COLUMN_FILENAME  : return Wt::WString(_searchResults[index.row()].name) ;
						//case COLUMN_SIZE  : return make_big_number(_downloads[index.row()].size) ;
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

RSWappSharedFilesPage::RSWappSharedFilesPage(Wt::WContainerWidget *parent,RsFiles *mfiles)
	: WCompositeWidget(parent),mFiles(mfiles)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	//_treeView = new Wt::WTreeView(_impl);
	_tableView = new Wt::WTableView(_impl);

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;

	_tableView->setAlternatingRowColors(true);

	_tableView->setSelectionMode(Wt::ExtendedSelection);
	_tableView->setDragEnabled(true);


	_tableView->setColumnWidth(0, 250);
	_tableView->setColumnWidth(1, 150);
	_tableView->setColumnWidth(2, 250);
	_tableView->setColumnWidth(3, 150);
	_tableView->setColumnWidth(4, 150);
	_tableView->setColumnWidth(5, 100);

	_shared_files_model = new LocalSharedFilesModel(mfiles) ;

	_tableView->setModel(_shared_files_model) ;
	layout->addWidget(_tableView,1) ;

	_tableView->setHeight(300) ;

	search_box = new Wt::WTextArea(_impl) ;
	search_box->setText("mp3") ;
	search_box->setHeight(100) ;
	layout->addWidget(search_box) ;

	Wt::WPushButton *btn = new Wt::WPushButton("Search!") ;
	btn->clicked().connect(this,&RSWappSharedFilesPage::searchClicked) ;
	layout->addWidget(btn) ;

	searchClicked();
}

void RSWappSharedFilesPage::searchClicked()
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

	FileSearchFlags fsf = RS_FILE_HINTS_REMOTE;

	std::list<DirDetails> initialResults;
	std::cerr << "stringlen "<< strings.size() <<std::endl;
	for(std::list<std::string>::iterator resultsIter = strings.begin(); resultsIter != strings.end(); resultsIter ++)
    {
        std::cerr << "str: " << *resultsIter << std::endl;
	}
	mFiles->SearchKeywords(strings,initialResults,fsf);
	std::cerr << "RESULTLEN: "<< initialResults.size() <<std::endl;
	_shared_files_model->displayList(initialResults);
	refresh();

}

void RSWappSharedFilesPage::refresh() {
	_shared_files_model->refresh();
	_tableView->refresh();
	_tableView->setHeight(300);//Without this line - wt wont update for me (wt3.3)

}
