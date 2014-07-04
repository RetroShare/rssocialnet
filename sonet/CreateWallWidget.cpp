#include "CreateWallWidget.h"

#include "rswall.h"

namespace RsWall{

CreateWallWidget::CreateWallWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _mTokenQueue(rsWall->getTokenService())
{
    _mTokenQueue.tokenReady().connect(this, &CreateWallWidget::tokenCallback);

    _profileText = new Wt::WTextArea(this);
    _profileText->setEmptyText("profile text");

    _idChooser = new GxsIdChooserWt(this);

    _submitPushButton = new Wt::WPushButton("create new wall for selected id", this);
    _submitPushButton->clicked().connect(this, &CreateWallWidget::createWall);
}

void CreateWallWidget::createWall()
{
    if(_mTokenQueue.tokensWaiting())
    {
        return;
    }
    if(_profileText->text().empty())
    {
        return;
    }
    WallGroup wg;
    wg.mProfileText = _profileText->text().toUTF8();
    wg.mMeta.mAuthorId = _idChooser->getSelectedId();
    // todo
    //wg.mAvatarImage
    //wg.mWallImage
    uint32_t token;
    rsWall->createWallGroup(token, wg);
    _mTokenQueue.queueToken(token);
    _profileText->setText("PENDING");
}

void CreateWallWidget::tokenCallback(uint32_t token, bool ok)
{
    if(ok)
    {
        _profileText->setText("COMPLETE");
    }
    else
    {
        _profileText->setText("FAIL");
    }
}
}//namespace RsWall
