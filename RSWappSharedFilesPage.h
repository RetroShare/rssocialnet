#pragma once

#include <Wt/WTreeView>

class RsFiles ;
class LocalSharedFilesModel ;

class RSWappSharedFilesPage: public Wt::WCompositeWidget
{
	public:
		RSWappSharedFilesPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

		virtual void refresh() ;
	private:
		Wt::WContainerWidget *_impl;
		Wt::WTreeView *_treeView ;
		Wt::WTableView* _tableView;
		Wt::WLineEdit *search_box ;
		RsFiles *mFiles ;
		LocalSharedFilesModel *_shared_files_model ;
		void searchClicked();
		void tableClicked();
};
