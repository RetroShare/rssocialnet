#include "GxsIdChooserWt.h"

#include "rswall.h"

GxsIdChooserWt::GxsIdChooserWt(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    _idCombo = new Wt::WComboBox(this);

    mTimer.setInterval(100);
    mTimer.setSingleShot(true);
    mTimer.timeout().connect(this, &GxsIdChooserWt::loadIds);

    loadIds();
}

void GxsIdChooserWt::loadIds()
{
    bool ok = true;
    _ownIds.clear();
    ok &= rsIdentity->getOwnIds(_ownIds);
    _idCombo->clear();
    std::vector<RsIdentityDetails> details;
    std::list<RsGxsId>::iterator lit;
    for(lit = _ownIds.begin(); lit != _ownIds.end(); lit++)
    {
        RsIdentityDetails detail;
        ok &= rsIdentity->getIdDetails(*lit, detail);
        if(ok) { details.push_back(detail); }
    }
    if(ok)
    {
        std::vector<RsIdentityDetails>::iterator it;
        for(it = details.begin(); it != details.end(); it++){
            const RsIdentityDetails& detail = *it;
            std::string anon;
            if(detail.mPgpLinked){
                anon = "[pgp]";
            } else {
                anon = "[anon]";
            }
            _idCombo->addItem(detail.mNickname + "[" + detail.mId.toStdString().substr(0, 5) + "]" + anon);
        }
    }
    else
    {
        // id details not cached, try later
        mTimer.start();
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
