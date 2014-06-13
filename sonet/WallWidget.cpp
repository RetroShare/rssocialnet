#include "WallWidget.h"

WallWidget::WallWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _TokenQueue(rsWall->getTokenService())
{
    _TokenQueue.tokenReady().connect(this, &WallWidget::tokenCallback);

    _TextArea = new Wt::WTextArea(this);
}

void WallWidget::setWallId(const RsGxsGroupId &id)
{
    uint32_t token;
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
    std::list<RsGxsGroupId> grpIds;
    grpIds.push_back(id);
    rsWall->getTokenService()->requestMsgInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);
    _TokenQueue.queueToken(token);
    _TextArea->setText("PENDING grpId="+id.toStdString());
}

void WallWidget::tokenCallback(uint32_t token, bool ok)
{
    if(ok)
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
    else
    {
        _TextArea->setText("FAIL");
    }
}
