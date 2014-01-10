#pragma once

#include <retroshare/rsnotify.h>
#include <util/rsthreads.h>
#include <Wt/WTreeView>

class RsFiles ;
class LocalSearchFilesModel ;

class RSWappSearchFilesPage: public Wt::WCompositeWidget, public NotifyClient
{
	public:
		RSWappSearchFilesPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

		virtual void refresh() ;
		virtual void notifyTurtleSearchResult(uint32_t search_id,const std::list<TurtleFileInfo>& files) ;

	private:
		Wt::WContainerWidget *_impl;
		Wt::WTreeView *_treeView ;
		Wt::WTableView* _tableView;
		Wt::WLineEdit *search_box ;
		Wt::WCheckBox *localcb;
		Wt::WCheckBox *remotecb;
		Wt::WCheckBox *distantcb;
		RsFiles *mFiles ;
		LocalSearchFilesModel *_shared_files_model ;
		void searchClicked();
		void tableClicked();

		std::map<uint32_t,std::list<TurtleFileInfo> > _turtle_search_results ;
};

