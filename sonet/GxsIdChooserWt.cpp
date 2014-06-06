#include "GxsIdChooserWt.h"

#include "rswall.h"

GxsIdChooserWt::GxsIdChooserWt(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    _idCombo = new Wt::WComboBox(this);
    loadIds();
}

void GxsIdChooserWt::loadIds()
{
    rsIdentity->getOwnIds(_ownIds);
    _idCombo->clear();
    std::list<RsGxsId>::iterator it;
    for(it = _ownIds.begin(); it != _ownIds.end(); it++){
        _idCombo->addItem(it->toStdString());
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
