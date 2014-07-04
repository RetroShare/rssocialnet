#include "CreatePostWidget2.h"

#include "CreatePostWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

#include "rswall.h"

namespace RsWall{

CreatePostWidget2::CreatePostWidget2(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _tokenQueue(rsWall->getTokenService())
{
    _tokenQueue.tokenReady().connect(this, &CreatePostWidget2::onTokenReady);
    _messageTextArea = new Wt::WTextArea(this);
    _messageTextArea->setEmptyText("make a post");
    _shareButton = new ShareButton(this);
    _shareButton->shareClicked().connect(this, &CreatePostWidget2::onShareClicked);
}

void CreatePostWidget2::onShareClicked(const RsGxsId& authorId, const RsGxsCircleId& circleId)
{
    Wt::WString text = _messageTextArea->text();
    if(text != ""){
        _messageTextArea->setText("PENDING");
        _shareButton->actionButton()->disable();
        _shareButton->dropDownButton()->disable();

        PostReferenceParams params;
        params.mAuthor = authorId;
        params.mCircle = circleId;
        //params.mReferencedGroupId     filled in by service
        params.mTargetWallOwner = authorId;
        uint32_t token;
        rsWall->createPost(token, params, text.toUTF8());
        _tokenQueue.queueToken(token);
    }
}

void CreatePostWidget2::onTokenReady(uint32_t token, bool ok)
{
    if(ok)
    {
        _messageTextArea->setText("COMPLETE");
    }
    else
    {
        _messageTextArea->setText("FAIL");
    }
    _shareButton->actionButton()->enable();
    _shareButton->dropDownButton()->enable();
}
}//namespace RsWall
