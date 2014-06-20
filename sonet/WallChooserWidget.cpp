#include "WallChooserWidget.h"

#include "rswall.h"
#include "RsGxsUpdateBroadcastWt.h"

WallChooserWidget::WallChooserWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _TokenQueue(rsWall->getTokenService())
{
    _WallCombo = new Wt::WComboBox(this);
    _WallCombo->activated().connect(this, &WallChooserWidget::onSelectionChanged);
    requestWallIds();

    // always have to connect manually
    // would not have to do this if this class was derived from tokenqueuewt1
    _TokenQueue.tokenReady().connect(this, &WallChooserWidget::tokenCallback);
    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &WallChooserWidget::requestWallIds);
}

RsGxsGroupId WallChooserWidget::getSelectedWallId()
{
    int selection = _WallCombo->currentIndex();
    if(selection >= 0)
    {
        return _DataVec[selection].second;
    }
    else
    {
        return RsGxsGroupId();
    }
}

void WallChooserWidget::requestWallIds()
{
    uint32_t token;
    rsWall->requestWallGroups(token, RsGxsId());
    _TokenQueue.queueToken(token);
}

void WallChooserWidget::tokenCallback(uint32_t token, bool ok)
{
    if(ok)
    {
        //
        _DataVec.clear();
        _WallCombo->clear();

        std::vector<WallGroup> grps;
        rsWall->getWallGroups(token, grps);
        std::vector<WallGroup>::iterator vit;
        for(vit = grps.begin(); vit != grps.end(); vit++)
        {
            WallGroup& grp = *vit;
            _DataVec.push_back(std::pair<RsGxsId, RsGxsGroupId>(grp.mMeta.mAuthorId, grp.mMeta.mGroupId));

            _WallCombo->addItem("RsGxsId="+grp.mMeta.mAuthorId.toStdString()+" RsGxsGroupId="+grp.mMeta.mGroupId.toStdString());
        }
    }
    else
    {
        // this is not clean and can lead to fail in getSelected()
        _WallCombo->addItem("FAIL");
    }
}

void WallChooserWidget::onSelectionChanged(int)
{
    _SelectionChangedSignal.emit();
}
