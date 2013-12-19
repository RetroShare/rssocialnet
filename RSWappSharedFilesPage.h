#pragma once

#include <Wt/WTreeView>

class RsFiles ;
class RsPeers ;
class LocalSharedFilesModel ;

class RSWappSharedFilesPage: public Wt::WCompositeWidget
{
	public:
		RSWappSharedFilesPage(Wt::WContainerWidget *container,RsFiles *rsfiles,RsPeers *mpeers) ;

	private:
		Wt::WContainerWidget *_impl;
		Wt::WTreeView *_treeView ;
		RsFiles *mFiles ;
		RsPeers *mPeers ;

		LocalSharedFilesModel *_shared_files_model ;
};
