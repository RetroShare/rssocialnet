#include "GxsIdChooserWt.h"

#include "rswall.h"

#include "SonetUtil.h"
#include "RSWApplication.h"
#include "WebUITimer.h"

namespace RsWall{

GxsIdChooserWt::GxsIdChooserWt(Wt::WContainerWidget *parent):
    WContainerWidget(parent), _idCombo(new Wt::WComboBox(this))
{
    loadIds();
}

void GxsIdChooserWt::loadIds()
{
    bool ok = true;
    _ownIds.clear();
    RSWApplication::ifaces().mIdentity->getOwnIds(_ownIds);
    _idCombo->clear();
    std::vector<RsIdentityDetails> details;
    std::list<RsGxsId>::iterator lit;
    for(lit = _ownIds.begin(); lit != _ownIds.end(); lit++)
    {
        RsIdentityDetails detail;
        ok &= RSWApplication::ifaces().mIdentity->getIdDetails(*lit, detail);
        if(ok) { details.push_back(detail); }
    }
    if(ok)
    {
        std::vector<RsIdentityDetails>::iterator it;
        for(it = details.begin(); it != details.end(); it++){
            _idCombo->addItem(SonetUtil::formatGxsId(*it));
        }
    }
    else
    {
        // id details not cached, try later
        WebUITimer::singleShotNextTick(this, &GxsIdChooserWt::loadIds);
    }
}

RsGxsId GxsIdChooserWt::getSelectedId()
{
    int selection = _idCombo->currentIndex();
    if(selection >= 0){
        std::list<RsGxsId>::iterator it = _ownIds.begin();
        for(int i = 0; i < selection; i++){
            it++;
        }
        return *it;
    }else{
        return RsGxsId();
    }
}
}//namespace RsWall
