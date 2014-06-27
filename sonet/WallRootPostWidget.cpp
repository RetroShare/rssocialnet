#include "WallRootPostWidget.h"

#include <Wt/WBorderLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WBreak>
#include <Wt/WDateTime>

#include "rswall.h"
#include "RsGxsUpdateBroadcastWt.h"


WallRootPostWidget::WallRootPostWidget(RsGxsGroupId grpId, Wt::WContainerWidget *parent):
    WContainerWidget(parent), _mTokenQueue(rsWall->getTokenService()),_mGrpId(grpId)
{
    Wt::WBorderLayout *layout = new Wt::WBorderLayout();
    //Wt::WHBoxLayout *layout = new Wt::WHBoxLayout();
    this->setLayout(layout);

    _mAvatarWidget = new AvatarWidget(false);
    layout->addWidget(_mAvatarWidget, Wt::WBorderLayout::West);
    //layout->addWidget(_mAvatarWidget);


    // a BorderLayout can only handle one widget in each region
    // else will crash
    // docs also say only one widget
    // so make another container and place widgets there
    _mCenterContainer = new Wt::WContainerWidget();
    layout->addWidget(_mCenterContainer, Wt::WBorderLayout::Center);
    //layout->addWidget(_mCenterContainer);

    _mIdChooser = new GxsIdChooserWt(_mCenterContainer);
    _mWallChooser = new WallChooserWidget(_mCenterContainer);
    Wt::WPushButton* button = new Wt::WPushButton("share this post", _mCenterContainer);
    button->clicked().connect(this, &WallRootPostWidget::onShareButtonClicked);

    new Wt::WBreak(_mCenterContainer);
    // will look like this:
    // user1 wrote on 1.1.2011 11:11
    _mIdentityLabel = new IdentityLabelWidget(_mCenterContainer);
    _mTimeLabel = new Wt::WLabel(_mCenterContainer);

    new Wt::WBreak(_mCenterContainer);
    _mText = new Wt::WLabel(_mCenterContainer);

    new Wt::WBreak(_mCenterContainer);
    new Wt::WLabel("comments:", _mCenterContainer);
    _CommentContainer = new CommentContainerWidget(rsWall, rsWall, _mCenterContainer);

    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &WallRootPostWidget::grpsChanged);

    _mTokenQueue.tokenReady().connect(this, &WallRootPostWidget::onTokenReady);

    setGroupId(grpId);
}

void WallRootPostWidget::setGroupId(RsGxsGroupId &grpId)
{
    _mGrpId = grpId;
    // request stuff
    uint32_t token;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
    std::list<RsGxsGroupId> grpIds;
    grpIds.push_back(grpId);
    rsWall->getTokenService()->requestMsgInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);

    _mTokenQueue.queueToken(token);

    _mText->setText("PENDING");
}

void WallRootPostWidget::onTokenReady(uint32_t token, bool ok)
{
    if(ok)
    {
        PostMsg pm;
        rsWall->getPostMsg(token, pm);
        _mAvatarWidget->setIdentity(pm.mMeta.mAuthorId);

        _mIdentityLabel->setIdentity(pm.mMeta.mAuthorId);
        _mTimeLabel->setText(" wrote on " + Wt::WDateTime::fromTime_t(pm.mMeta.mPublishTs).toString());

        _mText->setText(Wt::WString::fromUTF8(pm.mPostText));

        _CommentContainer->setGrpMsgId(RsGxsGrpMsgIdPair(pm.mMeta.mGroupId, pm.mMeta.mMsgId));
    }
    else
    {
        _mText->setText("FAIL");
    }
}

void WallRootPostWidget::grpsChanged(const std::list<RsGxsGroupId> &grps)
{
    //
}

void WallRootPostWidget::onShareButtonClicked()
{
    ReferenceMsg refMsg;
    refMsg.mMeta.mAuthorId = _mIdChooser->getSelectedId();
    refMsg.mMeta.mGroupId = _mWallChooser->getSelectedWallId();
    refMsg.mReferencedGroup = _mGrpId;
    uint32_t token;
    rsWall->createPostReferenceMsg(token, refMsg);
}
