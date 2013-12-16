#pragma once

#include <Wt/WTreeView>

class RsFiles ;
class LocalSearchFilesModel ;

class RSWappSearchFilesPage: public Wt::WCompositeWidget
{
	public:
		RSWappSearchFilesPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

		virtual void refresh() ;
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
};
