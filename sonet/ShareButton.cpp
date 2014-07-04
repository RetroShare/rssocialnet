#include "ShareButton.h"

#include <Wt/WPushButton>

#include "RSWApplication.h"
#include "rswall.h"

namespace RsWall{

ShareButton::ShareButton(Wt::WContainerWidget* parent):
    WSplitButton("share on own wall", parent)
{
    actionButton()->clicked().connect(this, &ShareButton::onShareClicked);
    dropDownButton()->clicked().connect(this, &ShareButton::onDropdownClicked);
}

void ShareButton::onShareClicked()
{
    shareClickedSignal.emit(
                RSWApplication::instance()->getCurrentIdentity().mId,
                RSWApplication::instance()->getPreferredCirlceId());
    if(!grpId.isNull())
    {
        // create areference message
        uint32_t token;
        PostReferenceParams params;
        params.mAuthor = RSWApplication::instance()->getCurrentIdentity().mId;
        params.mCircle = RSWApplication::instance()->getPreferredCirlceId();
        params.mReferencedGroupId = grpId;
        params.mTargetWallOwner = RSWApplication::instance()->getCurrentIdentity().mId;
        rsWall->createPostReferenceMsg(token, params);
    }
}

void ShareButton::onDropdownClicked()
{
    // TODO
}
}//namespace RsWall
