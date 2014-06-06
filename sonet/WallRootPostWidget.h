#pragma once

#include <Wt/WContainerWidget>

#include "AvatarWidget.h"

// contains a root post in a thread and the comments below
class WallRootPostWidget: public Wt::WContainerWidget{
public:
    WallRootPostWidget(Wt::WContainerWidget* parent = 0);
    // the grp behind grpId should be of type PostGroup
    void setGroupId(RsGxsGroupId& grpId);
private:
    void grpsChanged(const std::list<RsGxsGroupId> &grps);
    RsGxsGroupId _mGrpId;
    AvatarWidget* _mAvatarWidget;
    Wt::WContainerWidget* _mCenterContainer;
};
