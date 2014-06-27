#include "WallWidget.h"
#include "RsGxsUpdateBroadcastWt.h"

#include <Wt/WBreak>
#include <Wt/WTimer>

#include <retroshare/rsidentity.h>

WallWidget::WallWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _TokenQueue(rsWall->getTokenService()),
    _WallGroupToken(0), _PostMsgToken(0), isEditing(false), subscribed(false)
{
    _ProfileContainer = new Wt::WContainerWidget(this);
    _ProfileEdit = new Wt::WTextArea(this);
    _ProfileEdit->hide();
    _EditButton = new Wt::WPushButton("PENDING", this);
    _EditButton->hide();
    _EditButton->clicked().connect(this, &WallWidget::onEditClicked);
    _TextArea = new Wt::WTextArea(this);
    _SubscribeButton = new Wt::WPushButton("PENDING", this);
    _SubscribeButton->clicked().connect(this, &WallWidget::onSubscribeClicked);

    _TokenQueue.tokenReady().connect(this, &WallWidget::tokenCallback);
    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &WallWidget::loadProfile);
}

void WallWidget::setWallId(const RsGxsGroupId &id)
{
    _GrpId = id;
    reload();
}

void WallWidget::setWallByAuthorId(const RsGxsId &id)
{
    _AuthorId = id;
    loadProfile();
}

void WallWidget::loadProfile()
{
    // TODO: remove this check when we can always use setWallByAuthorId
    if(!_AuthorId.isNull())
    {
        _TextArea->setText("setWallByAuthorId(\""+_AuthorId.toStdString()+"\")\nrequest WallGroups for author PENDING");
        rsWall->requestWallGroups(_WallGroupToken, _AuthorId);
        _TokenQueue.queueToken(_WallGroupToken);
        checkIfOwnWall();
        checkIfSubscribed();
    }
    else
    {
        // fallback to old behavior, to delete later
        reload();
    }
}

void WallWidget::reload()
{
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
    std::list<RsGxsGroupId> grpIds;
    grpIds.push_back(_GrpId);
    rsWall->getTokenService()->requestMsgInfo(_PostMsgToken, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);
    _TokenQueue.queueToken(_PostMsgToken);
    _TextArea->setText("PENDING grpId="+_GrpId.toStdString());
}

void WallWidget::checkIfOwnWall()
{
    std::list<RsGxsId> ids;
    if(rsIdentity->getOwnIds(ids))
    {
        if(std::find(ids.begin(), ids.end(), _AuthorId) != ids.end()){
            _EditButton->setText("edit profile text");
            _EditButton->show();
        } else {
            _EditButton->hide();
        }
    }
    // code below will never be executed, because the fn getOwnIds always returns true
    /*
    else
    {
        // ids not cached, try later
        Wt::WTimer::singleShot(100, this, &WallWidget::checkIfOwnWall);
    }*/
}

void WallWidget::checkIfSubscribed()
{
    if(rsWall->isAuthorSubscribed(_AuthorId, subscribed))
    {
        displaySubscribeStatus();
    }
    else
    {
        _SubscribeButton->setText("ERROR: could not get subscribe status");
    }
}

void WallWidget::onSubscribeClicked()
{
    subscribed = ~subscribed;
    rsWall->subscribeToAuthor(_AuthorId, subscribed);
    displaySubscribeStatus();
}

void WallWidget::displaySubscribeStatus()
{
    if(subscribed)
    {
        _SubscribeButton->setText("unsubscribe from this author");
    }
    else
    {
        _SubscribeButton->setText("subscribe to this author");
    }
}

void WallWidget::onEditClicked()
{
    if(isEditing)
    {
        _Grp.mProfileText = _ProfileEdit->text().toUTF8();
        uint32_t token;
        rsWall->updateWallGroup(token, _Grp);
        _ProfileEdit->hide();
        _EditButton->setText("edit profile text");
        isEditing = false;
    }
    else
    {
        _ProfileEdit->show();
        _EditButton->setText("save changes");
        isEditing = true;
    }
}

void WallWidget::tokenCallback(uint32_t token, bool ok)
{
    if(ok)
    {
        if(token == _WallGroupToken)
        {
            std::vector<WallGroup> wgs;
            rsWall->getWallGroups(token, wgs);
            if(!wgs.empty())
            {
                _ProfileContainer->clear();
                new Wt::WLabel("Wall Page of ", _ProfileContainer);
                IdentityLabelWidget* identityLabel = new IdentityLabelWidget(_ProfileContainer);
                new Wt::WBreak(_ProfileContainer);
                new Wt::WLabel("Profile text:", _ProfileContainer);
                new Wt::WBreak(_ProfileContainer);
                Wt::WLabel* profileText = new Wt::WLabel(_ProfileContainer);

                // use first group only
                // TODO: maybe should the backend merge all grps
                WallGroup& wg = wgs.front();
                _Grp = wg;
                _GrpId = wg.mMeta.mGroupId;
                identityLabel->setIdentity(_AuthorId);
                profileText->setText(wg.mProfileText);
                _ProfileEdit->setText(wg.mProfileText);
                // load this wall group
                reload();
            }
            else
            {
                _TextArea->setText("FAIL there are no WallGroups for this gxs-id");
            }
        }
        if(token == _PostMsgToken)
        {
            std::string text;
            text += "COMPLETE\n";
            std::vector<ReferenceMsg> refMsgs;
            rsWall->getPostReferenceMsgs(token, refMsgs);
            std::vector<ReferenceMsg>::iterator vit;
            for(vit = refMsgs.begin(); vit != refMsgs.end(); vit++)
            {
                ReferenceMsg& msg = *vit;
                text += "refMsgId="+msg.mMeta.mMsgId.toStdString()+" referenced grpId="+msg.mReferencedGroup.toStdString()+"\n";
            }
            _TextArea->setText(text);

            for(std::vector<WallRootPostWidget*>::iterator vit2 = _PostWidgets.begin(); vit2 != _PostWidgets.end(); vit2++)
            {
                delete *vit2;
            }
            _PostWidgets.clear();

            for(vit = refMsgs.begin(); vit != refMsgs.end(); vit++)
            {
                ReferenceMsg& msg = *vit;
                WallRootPostWidget* widget = new WallRootPostWidget(msg.mReferencedGroup, this);
                _PostWidgets.push_back(widget);
            }
        }
    }
    else
    {
        _TextArea->setText("FAIL");
    }
}
