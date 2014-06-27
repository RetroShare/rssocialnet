#include "CommentContainerWidget.h"

#include <Wt/WBorderLayout>

#include "AvatarWidget.h"
#include "RsGxsUpdateBroadcastWt.h"

CommentContainerWidget::CommentContainerWidget(RsGxsCommentService *commentService,
                                               RsGxsIfaceHelper *ifaceHelper,
                                               Wt::WContainerWidget *parent):
    WContainerWidget(parent),
    _IfaceHelper(ifaceHelper), _CommentService(commentService), _TokenQueue(ifaceHelper->getTokenService())
{
    _TokenQueue.tokenReady().connect(this, &CommentContainerWidget::onTokenReady);

    new Wt::WLabel("CommentContainerWidget: IDLE", this);
    RsGxsUpdateBroadcastWt::get(_IfaceHelper)->msgsChanged().connect(this, &CommentContainerWidget::requestComments);
}

void CommentContainerWidget::setGrpMsgId(RsGxsGrpMsgIdPair id)
{
    _GrpMsgId = id;
    requestComments();
}

void CommentContainerWidget:: requestComments()
{
    // msgrelated request disabled until i understand it
    // probably the msg items stored don't have the right ids set
    /*
    // the requets is stolen from GxsCommentTreeWidget
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_RELATED_DATA;
    opts.mOptions = RS_TOKREQOPT_MSG_THREAD | RS_TOKREQOPT_MSG_LATEST;

    std::vector<RsGxsGrpMsgIdPair> msgIds;
    msgIds.push_back(_GrpMsgId);

    uint32_t token;
    _IfaceHelper->getTokenService()->requestMsgRelatedInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, msgIds);
    _TokenQueue.queueToken(token);
    */
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;

    std::list<RsGxsGroupId> grpIds;
    grpIds.push_back(_GrpMsgId.first);

    uint32_t token;
    _IfaceHelper->getTokenService()->requestMsgInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);
    _TokenQueue.queueToken(token);

    clear();
    new Wt::WLabel("PEDING", this);
}

void CommentContainerWidget::onTokenReady(uint32_t token, bool ok)
{
    if(ok)
    {
        clear();
        std::vector<RsGxsComment> comments;
        //_CommentService->getRelatedComments(token, comments);
        _CommentService->getCommentData(token, comments);
        for(std::vector<RsGxsComment>::iterator vit = comments.begin(); vit != comments.end(); vit++)
        {
            RsGxsComment& comment = *vit;
            Wt::WContainerWidget* outerContainer = new Wt::WContainerWidget(this);
            Wt::WBorderLayout *layout = new Wt::WBorderLayout();
            outerContainer->setLayout(layout);
            AvatarWidget* avatar = new AvatarWidget(true);
            avatar->setIdentity(comment.mMeta.mAuthorId);
            layout->addWidget(avatar, Wt::WBorderLayout::West);
            Wt::WLabel* text = new Wt::WLabel(Wt::WString::fromUTF8(comment.mComment));
            layout->addWidget(text, Wt::WBorderLayout::Center);
        }
        // note: the widgets the pointers where pointing to where deleted by clear()
        _TextArea = new Wt::WTextArea(this);
        _TextArea->setEmptyText("your comment");
        _IdChooser = new GxsIdChooserWt(this);
        _SubmitButton = new Wt::WPushButton("submit", this);
        _SubmitButton->clicked().connect(this, &CommentContainerWidget::onSubmitComment);
    }
    else
    {
        clear();
        new Wt::WLabel("FAIL", this);
    }
}

void CommentContainerWidget::onSubmitComment()
{
    // todo: handle token result
    uint32_t token;
    RsGxsComment comment;
    comment.mMeta.mGroupId = _GrpMsgId.first;
    comment.mMeta.mAuthorId = _IdChooser->getSelectedId();
    comment.mComment = _TextArea->text().toUTF8();
    _CommentService->createComment(token, comment);
    //requestComments();
}
