#include <Wt/WContainerWidget>

#include "RSWappFriendsPage.h"

#include <Wt/WAbstractTableModel>

class FriendListModel : public Wt::WAbstractTableModel
{
	public:
		FriendListModel(int rows, int columns, Wt::WObject *parent = 0)
			: Wt::WAbstractTableModel(parent),
			rows_(rows), columns_(columns)
		{ }

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return rows_;
			else
				return 0;
		}

		virtual int columnCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			if (!parent.isValid())
				return columns_;
			else
				return 0;
		}

		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			switch (role) {
				case Wt::DisplayRole:
					if (index.column() == 0)
						return Wt::WString("Row {1}").arg(index.row());
					else
						return Wt::WString("Item row {1}, col {2}")
							.arg(index.row()).arg(index.column());
				default:
					return boost::any();
			}
		}

		virtual boost::any headerData(int section,
				Wt::Orientation orientation = Wt::Horizontal,
				int role = Wt::DisplayRole) const
		{
			if (orientation == Wt::Horizontal) {
				switch (role) {
					case Wt::DisplayRole:
						return Wt::WString("Column {1}").arg(section);
					default:
						return boost::any();
				}
			} else
				return boost::any();
		}

	private:
		int rows_, columns_;
};

RSWappFriendsPage::RSWappFriendsPage(Wt::WContainerWidget *parent)
	: WCompositeWidget(parent)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	Wt::WTableView *tableView = new Wt::WTableView(_impl);

	tableView->setAlternatingRowColors(true);

	//tableView->setModel(fileFilterModel_);
	tableView->setSelectionMode(Wt::ExtendedSelection);
	tableView->setDragEnabled(true);

	tableView->setColumnWidth(0, 100);
	tableView->setColumnWidth(1, 150);
	tableView->setColumnWidth(2, 100);
	tableView->setColumnWidth(3, 60);
	tableView->setColumnWidth(4, 100);
	tableView->setColumnWidth(5, 100);

	tableView->setModel(new FriendListModel(5,7)) ;
}
