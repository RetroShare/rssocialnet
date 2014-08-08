#include "WallWidget.h"
#include "RsGxsUpdateBroadcastWt.h"

#include <Wt/WBreak>
#include <Wt/WFileUpload>
#include <Wt/WTemplate>
#include <fstream>

#include <retroshare/rsidentity.h>
#include "RSWApplication.h"
#include "util/imageresize.h"

namespace RsWall{

WallWidget::WallWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _TokenQueue(rsWall->getTokenService()),
    isEditing(false), subscribed(false), _WallGroupToken(0), _PostMsgToken(0)
{
    Wt::WTemplate* templ = new Wt::WTemplate("<h1>Wall of ${name}</h1>", this);
    _HeaderIdentityLabel = new IdentityLabelWidget();
    templ->bindWidget("name", _HeaderIdentityLabel);

    _AvatarWidget = new AvatarWidgetWt(false, this);

    // the container contains the wall posts
    _ProfileContainer = new Wt::WContainerWidget(this);
    _ProfileEdit = new Wt::WTextArea(this);
    _ProfileEdit->hide();
    _EditButton = new Wt::WPushButton("PENDING", this);
    _EditButton->hide();
    _EditButton->clicked().connect(this, &WallWidget::onEditClicked);
    _TextArea = new Wt::WTextArea(this);
    // textarea is only useful for debugging, so hide for normal users
    _TextArea->hide();
    _SubscribeButton = new Wt::WPushButton("PENDING", this);
    _SubscribeButton->clicked().connect(this, &WallWidget::onSubscribeClicked);

    _EditAvatarButton = new Wt::WPushButton("edit avatar image", this);
    _EditAvatarButton->hide();
    _AvatarFileUpload = new Wt::WFileUpload(this);
    _AvatarFileUpload->hide();
    _UploadAvatarButton = new Wt::WPushButton("upload avatar image", this);
    _UploadAvatarButton->hide();
    _EditAvatarButton->clicked().connect(_EditAvatarButton, &Wt::WPushButton::hide);
    _EditAvatarButton->clicked().connect(_AvatarFileUpload, &Wt::WFileUpload::show);
    _EditAvatarButton->clicked().connect(_UploadAvatarButton, &Wt::WPushButton::show);
    _UploadAvatarButton->clicked().connect(_AvatarFileUpload, &Wt::WFileUpload::upload);
    _UploadAvatarButton->clicked().connect(_UploadAvatarButton, &Wt::WPushButton::disable);
    _AvatarFileUpload->uploaded().connect(this, &WallWidget::onAvatarImageUploaded);

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
        _AvatarWidget->setIdentity(_AuthorId);
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
    RSWApplication::ifaces().mIdentity->getOwnIds(ids);
    if(std::find(ids.begin(), ids.end(), _AuthorId) != ids.end()){
        _EditButton->setText("edit profile");
        _EditButton->show();

        _EditAvatarButton->show();
        _SubscribeButton->hide();

    } else {
        _EditButton->hide();

        _EditAvatarButton->hide();
        _SubscribeButton->show();
    }
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
        _SubscribeButton->setText("subscribe to this author to download the posts on this wall");
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
        _EditButton->setText("edit profile");
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
                new Wt::WBreak(_ProfileContainer);
                new Wt::WLabel("Profile text:", _ProfileContainer);
                new Wt::WBreak(_ProfileContainer);
                Wt::WLabel* profileText = new Wt::WLabel(_ProfileContainer);

                // use first group only
                // TODO: maybe the backend should merge all grps
                WallGroup& wg = wgs.front();
                _Grp = wg;
                _GrpId = wg.mMeta.mGroupId;
                _HeaderIdentityLabel->setIdentity(_AuthorId);
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

            for(std::vector<ReferenceWidget*>::iterator vit2 = _ReferenceWidgets.begin(); vit2 != _ReferenceWidgets.end(); vit2++)
            {
                delete *vit2;
            }
            _ReferenceWidgets.clear();

            for(std::vector<WallRootPostWidget*>::iterator vit2 = _PostWidgets.begin(); vit2 != _PostWidgets.end(); vit2++)
            {
                delete *vit2;
            }
            _PostWidgets.clear();

            for(vit = refMsgs.begin(); vit != refMsgs.end(); vit++)
            {
                ReferenceMsg& msg = *vit;

                ReferenceWidget* refWidget = new ReferenceWidget(this);
                refWidget->setReferenceMsg(msg);
                _ReferenceWidgets.push_back(refWidget);

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

void WallWidget::onAvatarImageUploaded()
{
    std::string tmpFileName = _AvatarFileUpload->spoolFileName();

    std::ifstream file(tmpFileName.c_str(), std::ios_base::binary);
    file.seekg(0, std::ios_base::end);
    uint32_t size = file.tellg();
    file.seekg(0, std::ios_base::beg);

    std::vector<uint8_t> buf;
    buf.resize(size);
    file.read((char*)&buf[0],buf.size());
    file.close();

    ImageUtil::limitImageSize(buf, _Grp.mAvatarImage.mData, 500, 500);

    uint32_t token;
    rsWall->updateWallGroup(token, _Grp);

    _EditAvatarButton->show();
    _AvatarFileUpload->hide();
    _UploadAvatarButton->hide();
    _UploadAvatarButton->enable();
}
}//namespace RsWall
