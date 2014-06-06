#include "WallRootPostWidget.h"

#include <Wt/WBorderLayout>

#include "rswall.h"
#include "RsGxsUpdateBroadcastWt.h"

WallRootPostWidget::WallRootPostWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    Wt::WBorderLayout *layout = new Wt::WBorderLayout();
    this->setLayout(layout);

    _mAvatarWidget = new AvatarWidget(false);
    layout->addWidget(_mAvatarWidget, Wt::WBorderLayout::West);

    // a BorderLayout can only handle one widget in each region
    // else will crash
    // docs also say only one widget
    // so make another container and place widgets there
    _mCenterContainer = new Wt::WContainerWidget();
    layout->addWidget(_mCenterContainer, Wt::WBorderLayout::Center);

    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &WallRootPostWidget::grpsChanged);
}

void WallRootPostWidget::setGroupId(RsGxsGroupId &grpId)
{
    _mGrpId = grpId;
    // request stuff
}

void WallRootPostWidget::grpsChanged(const std::list<RsGxsGroupId> &grps)
{
    //
}
