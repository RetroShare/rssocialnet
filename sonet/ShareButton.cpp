#include "ShareButton.h"

#include <Wt/WPushButton>

#include "RSWApplication.h"
#include "rswall.h"

namespace RsWall{

ShareButton::ShareButton(Wt::WContainerWidget* parent):
    WSplitButton("share on own wall", parent), tokenQueue(rsWall->getTokenService())
{
    actionButton()->clicked().connect(this, &ShareButton::onShareClicked);
    dropDownButton()->clicked().connect(this, &ShareButton::onDropdownClicked);
    dropDownButton()->setTextFormat(Wt::XHTMLUnsafeText);
    dropDownButton()->setText("<span class=\"caret\"></span>");
    tokenQueue.tokenReady().connect(this, &ShareButton::onTokenReady);
}

void ShareButton::onShareClicked()
{
    actionButton()->setEnabled(false);
    shareClickedSignal.emit(
                RSWApplication::instance()->getCurrentIdentity().mId,
                RSWApplication::instance()->getPreferredCirlceId());
    if(!grpId.isNull())
    {
        actionButton()->setText("PENDING");
        // create areference message
        uint32_t token;
        PostReferenceParams params;
        params.mAuthor = RSWApplication::instance()->getCurrentIdentity().mId;
        params.mCircle = RSWApplication::instance()->getPreferredCirlceId();
        params.mReferencedGroupId = grpId;
        params.mTargetWallOwner = RSWApplication::instance()->getCurrentIdentity().mId;
        rsWall->createPostReferenceMsg(token, params);
        tokenQueue.queueToken(token);
    }
}

void ShareButton::onDropdownClicked()
{
    // TODO
}

void ShareButton::onTokenReady(uint32_t , bool )
{
    actionButton()->setText("COMPLETE");
}
}//namespace RsWall
