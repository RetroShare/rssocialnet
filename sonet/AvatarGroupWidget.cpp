#include "AvatarGroupWidget.h"
#include "RsGxsUpdateBroadcastWt.h"
#include "rswall.h"
#include "AvatarWidget.h"

namespace RsWall{

AvatarGroupWidget::AvatarGroupWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), mTokenQueue(rsWall->getTokenService())
{
    mTokenQueue.tokenReady().connect(this, &AvatarGroupWidget::onTokenReady);
    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &AvatarGroupWidget::onGrpChange);
    requestWallMetas();
}

void AvatarGroupWidget::requestWallMetas()
{
    uint32_t token;
    rsWall->requestWallGroupMetas(token, RsGxsId());
    mTokenQueue.queueToken(token);
}

void AvatarGroupWidget::onTokenReady(uint32_t token, bool ok)
{
    if(ok)
    {
        std::vector<RsGroupMetaData> metas;
        rsWall->getWallGroupMetas(token, metas);

        // delete all child widgets
        clear();
        // add new widgets
        for(std::vector<RsGroupMetaData>::iterator vit = metas.begin(); vit != metas.end(); vit++)
        {
            AvatarWidgetWt* avatar = new AvatarWidgetWt(false, this);
            avatar->setIdentity(vit->mAuthorId);
            // make it inline, so avatarwidgets can appear in one line
            //avatar->setInline(true);// don't enable, this destroys the layout
        }
    }
    else
    {
        std::cerr << "AvatarGroupWidget::onTokenReady(): token failed" << std::endl;
    }
}

void AvatarGroupWidget::onGrpChange(/* maybe want to receive the list of changed grps later to update only the changed grps*/)
{
    requestWallMetas();
}
}//namespace RsWall
