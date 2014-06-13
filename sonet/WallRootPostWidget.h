#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

#include "AvatarWidget.h"
#include "TokenQueueWt2.h"
#include "CommentContainerWidget.h"

// contains a root post in a thread and the comments below
class WallRootPostWidget: public Wt::WContainerWidget{
public:
    WallRootPostWidget(RsGxsGroupId grpId, Wt::WContainerWidget *parent = 0);
    // the grp behind grpId should be of type PostGroup
    void setGroupId(RsGxsGroupId& grpId);
private:
    void onTokenReady(uint32_t token, bool ok);
    void grpsChanged(const std::list<RsGxsGroupId> &grps);
    TokenQueueWt2 _mTokenQueue;
    RsGxsGroupId _mGrpId;
    AvatarWidget* _mAvatarWidget;
    Wt::WLabel* _mText;
    Wt::WContainerWidget* _mCenterContainer;
    CommentContainerWidget* _CommentContainer;
};
