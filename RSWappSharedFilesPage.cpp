#include <Wt/WContainerWidget>
#include <Wt/WAbstractItemModel>
#include <Wt/WVBoxLayout>
#include <Wt/WTreeView>

#include "RSWappSharedFilesPage.h"

#include <retroshare/rsfiles.h>
#include <retroshare/rstypes.h>
#include <retroshare/rspeers.h>

#define COLUMN_FILENAME 0
#define COLUMN_SIZE     1
#define COLUMN_AGE      2
#define COLUMN_FLAGS    3
#define COLUMN_GROUPS   4

static const uint32_t IND_LAST_DAY    = 3600*24 ;
static const uint32_t IND_LAST_WEEK   = 3600*24*7 ;
static const uint32_t IND_LAST_MONTH  = 3600*24*31	; // I know, this is approximate
static const uint32_t IND_ALWAYS      = ~(uint32_t)0 ;

static Wt::WString userFriendlyDuration(uint64_t seconds)
{
    if(seconds < 0) {
        return Wt::WString("Unknown");
    }
    if(seconds < 60) {
        return Wt::WString("< 1m");
    }
    int minutes = seconds / 60;
    if(minutes < 60) {
        return Wt::WString("{1} minutes").arg(minutes);
    }
    int hours = minutes / 60;
    minutes = minutes - hours*60;
    if(hours < 24) {
        return Wt::WString("{1}h {2}m").arg(hours).arg(minutes);
    }
    int days = hours / 24;
    hours = hours - days * 24;
    if(days < 365) {
        return Wt::WString("{1}d {2}h").arg(days).arg(hours);
    }
    int years = days / 365;
    days = days - years * 365;
    return Wt::WString("{1}y {2}d").arg(years).arg(days);
}


const Wt::WString groupName(const RsGroupInfo &groupInfo)
{
	if ((groupInfo.flag & RS_GROUP_FLAG_STANDARD) == 0) {
		/* no need to be translated */
		return groupInfo.name;
	}

	if (groupInfo.id == RS_GROUP_ID_FRIENDS) {
		return "Friends";
	}
	if (groupInfo.id == RS_GROUP_ID_FAMILY) {
		return "Family";
	}
	if (groupInfo.id == RS_GROUP_ID_COWORKERS) {
		return "Co-Workers";
	}
	if (groupInfo.id == RS_GROUP_ID_OTHERS) {
		return "Other Contacts";
	}
	if (groupInfo.id == RS_GROUP_ID_FAVORITES) {
		return "Favorites";
	}

	std::cerr << "GroupDefs::name: Unknown group id requested " << groupInfo.id;
	return "";
}


Wt::WString friendlyUnit(float val)
{
    if(val < 0) {
        return "Unknown";
    }
    const Wt::WString units[5] = {Wt::WString(" B"), Wt::WString(" KB"), Wt::WString(" MB"), Wt::WString(" GB"), Wt::WString(" TB,") };

    for(unsigned int i=0; i<5; ++i) 
	 {
        if (val < 1024.) {
            return Wt::WString("{1}").arg(val) + units[i] ;
        }
        val /= 1024.;
    }
    return  Wt::WString("{1}").arg(val) + Wt::WString(" TB");
}

class LocalSharedFilesModel: public Wt::WAbstractItemModel
{
	public:
		LocalSharedFilesModel(RsFiles *mfiles,RsPeers *mpeers,Wt::WObject *parent = 0)
			: Wt::WAbstractItemModel(parent), mFiles(mfiles), mPeers(mpeers)
		{
			RemoteMode = false ;
		}

		virtual int rowCount(const Wt::WModelIndex& parent = Wt::WModelIndex()) const
		{
			void *ref = (parent.isValid())? parent.internalPointer() : NULL ;

			DirDetails details;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if (!requestDirDetails(ref, details, flags))
			{
#ifdef RDM_DEBUG
				std::cerr << "lookup failed -> 0";
				std::cerr << std::endl;
#endif
				return 0;
			}
			if (details.type == DIR_TYPE_FILE)
			{
#ifdef RDM_DEBUG
				std::cerr << "lookup FILE: 0";
				std::cerr << std::endl;
#endif
				return 0;
			}

			/* else PERSON/DIR*/
#ifdef RDM_DEBUG
			std::cerr << "lookup PER/DIR #" << details.count;
			std::cerr << std::endl;
#endif
			return details.count;
		}
		virtual bool hasChildren(const Wt::WModelIndex &parent) const
		{
			if (!parent.isValid())
				return true;

			void *ref = parent.internalPointer();

			DirDetails details;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if (!requestDirDetails(ref, details, flags))
				return false;

			if (details.type == DIR_TYPE_FILE)
				return false;

			/* PERSON/DIR*/
			return (details.count > 0); /* do we have children? */
		}

		virtual int columnCount(const Wt::WModelIndex& ) const
		{
			return 5;
		}
		Wt::WString computeDirectoryPath(const DirDetails& details) const
		{
			Wt::WString dir ;
			DirDetails det(details) ;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if(!requestDirDetails(det.parent,det,flags))
				return Wt::WString(); 

#ifdef SHOW_TOTAL_PATH
			do
			{
#endif
				dir = Wt::WString::fromUTF8(det.name.c_str())+"/"+dir ;

#ifdef SHOW_TOTAL_PATH
				if(!requestDirDetails(det.parent,det,flags))
					break ;
			}
			while(det.parent != NULL);
#endif
			return dir ;
		}
		Wt::WString getFlagsString(FileStorageFlags flags) const
		{
			char str[11] = "-  -  -" ;

			if(flags & DIR_FLAGS_BROWSABLE_GROUPS) 	str[0] = 'B' ;
			if(flags & DIR_FLAGS_BROWSABLE_OTHERS) 	str[3] = 'B' ;
			if(flags & DIR_FLAGS_NETWORK_WIDE_OTHERS) str[6] = 'N' ;

			return Wt::WString(str) ;
		}
		Wt::WString getGroupsString(const std::list<std::string>& group_ids)
		{
			Wt::WString groups_str ;
			RsGroupInfo group_info ;

			for(std::list<std::string>::const_iterator it(group_ids.begin());it!=group_ids.end();)
				if(mPeers->getGroupInfo(*it,group_info)) 
				{
					groups_str += groupName(group_info) ;

					if(++it != group_ids.end())
						groups_str += ", " ;
				}
				else
					++it ;

			return groups_str ;
		}

		Wt::WString getAgeIndicatorString(const DirDetails &details) const
		{
			Wt::WString ret("");
			Wt::WString nind = "NEW";
			uint32_t age = details.age;

			switch (ageIndicator) {
				case IND_LAST_DAY:
					if (age < 24 * 60 * 60) return nind;
					break;
				case IND_LAST_WEEK:
					if (age < 7 * 24 * 60 * 60) return nind;
					break;
				case IND_LAST_MONTH:
					if (age < 30 * 24 * 60 * 60) return nind;
					break;
					//		case IND_OLDER:
					//			if (age >= 30 * 24 * 60 * 60) return oind;
					//			break;
				case IND_ALWAYS:
					return ret;
				default:
					return ret;
			}

			return ret;
		}


		boost::any displayRole(const DirDetails& details,int coln) const
		{
			if (details.type == DIR_TYPE_FILE) /* File */
				switch(coln)
				{
					case 0: return Wt::WString::fromUTF8(details.name.c_str());
					case 1: return friendlyUnit(details.count);
					case 2: return userFriendlyDuration(details.age);
					case 3: return Wt::WString::fromUTF8(rsPeers->getPeerName(details.id).c_str());
					case 4: return computeDirectoryPath(details);
					default:
							  return boost::any() ;
				}

			return boost::any();
		} /* end of DisplayRole */

		boost::any sortRole(const Wt::WModelIndex& /*index*/,const DirDetails& details,int coln) const
		{
			if (details.type == DIR_TYPE_PERSON) /* Person */
			{
				switch(coln)
				{
					case 0:
						//return (RemoteMode)?(Wt::WString::fromUtf8(rsPeers->getPeerName(details.id).c_str())):tr("My files");
						return "My files";
					case 1:
						return Wt::WString();
					case 2:
						return details.min_age;
					default:
						return Wt::WString();
				}
			}
			else if (details.type == DIR_TYPE_FILE) /* File */
			{
				switch(coln)
				{
					case 0:
						return Wt::WString::fromUTF8(details.name.c_str());
					case 1:
						return details.count;
					case 2:
						return  details.age;
					case 3:
						return getFlagsString(details.flags);
					case 4:
						{
							Wt::WString ind("");
						//	if (ageIndicator != IND_ALWAYS)
						//		ind = getAgeIndicatorString(details);
							return ind;
						}
					default:
						return "FILE";
				}
			}
			else if (details.type == DIR_TYPE_DIR) /* Dir */
			{
				switch(coln)
				{
					case 0:
						return Wt::WString::fromUTF8(details.name.c_str());
					case 1:
						return details.count;
					case 2:
						return details.min_age;
					case 3:
						return getFlagsString(details.flags);
					default:
						return "DIR";
				}
			}
			return boost::any();
		}
		virtual boost::any data(const Wt::WModelIndex& index, int role = Wt::DisplayRole) const
		{
			if (!index.isValid())
				return boost::any();

			/* get the data from the index */
			void *ref = index.internalPointer();
			int coln = index.column();

			DirDetails details;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if(!requestDirDetails(ref, details, flags))
				return boost::any();

//			if(role == RetroshareDirModel::FileNameRole) /* end of FileNameRole */
//				return Wt::WString::fromUTF8(details.name.c_str());

//			if (role == Qt::TextColorRole)
//			{
//				if(details.min_age > ageIndicator)
//					return boost::any(QColor(Qt::gray)) ;
//				else
//					return boost::any() ; // standard
//			} /* end of TextColorRole */


//			if(role == Wt::DecorationRole)
//				return decorationRole(details,coln) ;

//			if (role == Wt::TextAlignmentRole)
//			{
//				if(coln == 1)
//					return int( Wt::AlignRight | Wt::AlignVCenter);
//				return boost::any() ;
//			} /* end of TextAlignmentRole */

			if (role == Wt::DisplayRole)
				return displayRole(details,coln) ;

//			if (role == Wt::SortRole)
//				return sortRole(index,details,coln) ;

			return boost::any() ;
		}

		Wt::WModelIndex parent( const Wt::WModelIndex & index ) const
		{
			/* create the index */
			if (!index.isValid())
				return Wt::WModelIndex();

			void *ref = index.internalPointer();

			DirDetails details;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if(!requestDirDetails(ref, details, flags))
			{
#ifdef RDM_DEBUG
				std::cerr << "Failed Lookup -> invalid";
				std::cerr << std::endl;
#endif
				return Wt::WModelIndex();
			}

			if (!(details.parent))
			{
#ifdef RDM_DEBUG
				std::cerr << "success. parent is Root/NULL --> invalid";
				std::cerr << std::endl;
#endif
				return Wt::WModelIndex();
			}

#ifdef RDM_DEBUG
			std::cerr << "success index(" << details.prow << ",0," << details.parent << ")";
			std::cerr << std::endl;

#endif
			return createIndex(details.prow, 0, details.parent);
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
		bool requestDirDetails(void *ref,DirDetails& details,FileSearchFlags flags) const
		{
			// We should use a cache instead of calling RsFiles::RequestDirDetails(), which is very costly
			// due to some pointer checking crap.

			return mFiles->RequestDirDetails(ref, details, flags) ;
		}

		Wt::WModelIndex index(int row, int column, const Wt::WModelIndex & parent) const
		{
			std::cerr << "index: row=" << row << ", column=" << column << std::endl;
			if(row < 0)
				return Wt::WModelIndex() ;

			void *ref = (parent.isValid()) ? parent.internalPointer() : NULL;

			DirDetails details;
			FileSearchFlags flags = (RemoteMode)?RS_FILE_HINTS_REMOTE:RS_FILE_HINTS_LOCAL;

			if (!requestDirDetails(ref, details, flags))
				return Wt::WModelIndex();


			/* now iterate through the details to
			 * get the reference number
			 */

			std::list<DirStub>::iterator it;
			int i = 0;
			for(it = details.children.begin(); ((i < row) && (it != details.children.end())); ++it,++i) ;

			if (it == details.children.end())
				return Wt::WModelIndex();

			/* we can just grab the reference now */

			std::cerr << "returning new index with ref=" << (void*)it->ref << std::endl;
			return createIndex(row, column, it->ref);
		}

	private:
		RsFiles *mFiles ;
		RsPeers *mPeers ;
		bool RemoteMode ;
		int ageIndicator ;
};

RSWappSharedFilesPage::RSWappSharedFilesPage(Wt::WContainerWidget *parent,RsFiles *mfiles,RsPeers *mpeers)
	: WCompositeWidget(parent),mFiles(mfiles)
{
	setImplementation(_impl = new Wt::WContainerWidget()) ;

	_treeView = new Wt::WTreeView(_impl);

	Wt::WVBoxLayout *layout = new Wt::WVBoxLayout() ;
	_impl->setLayout(layout) ;

	_treeView->setAlternatingRowColors(true);

	_treeView->setSelectionMode(Wt::ExtendedSelection);
	_treeView->setDragEnabled(true);

	_shared_files_model = new LocalSharedFilesModel(mfiles,mpeers) ;

	_treeView->setModel(_shared_files_model) ;
	layout->addWidget(_treeView,1) ;

	_treeView->setHeight(300) ;
}
