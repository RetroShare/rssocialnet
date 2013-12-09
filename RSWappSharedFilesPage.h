#pragma once

#include <Wt/WTreeView>

class RsFiles ;
class LocalSharedFilesModel ;

class RSWappSharedFilesPage: public Wt::WCompositeWidget
{
	public:
		RSWappSharedFilesPage(Wt::WContainerWidget *container,RsFiles *rsfiles) ;

	private:
		Wt::WContainerWidget *_impl;
		Wt::WTreeView *_treeView ;
		RsFiles *mFiles ;
		LocalSharedFilesModel *_shared_files_model ;
};
