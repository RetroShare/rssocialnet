#pragma once

#include <Wt/WTableView>
#include <Wt/WTextArea>

class DownloadsTransfersListModel ;

class RSWappTransfersPage: public Wt::WCompositeWidget
{
	public:
        RSWappTransfersPage(RsFiles *rsfiles, Wt::WContainerWidget *parent = 0) ;

		void downloadLink() ;
		void showCustomPopupMenu(const Wt::WModelIndex&, const Wt::WMouseEvent&) ;
		void popupAction() ;
		void toggleShowCacheTransfers() ;

		virtual void refresh() ;
	private:
		Wt::WContainerWidget *_impl;
		RsFiles *mFiles ;
		Wt::WTimer *_timer ;
		Wt::WTextArea *link_area ;
		Wt::WPopupMenu *_popupMenu ;
		Wt::WTableView *_tableView ;
        RsFileHash _selected_hash ;
		DownloadsTransfersListModel *_download_model ;
};
