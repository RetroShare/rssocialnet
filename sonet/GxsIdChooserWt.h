#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WComboBox>

#include <retroshare/rsidentity.h>

// a dropdown bo to select a id
// own ids only

class GxsIdChooserWt: public Wt::WContainerWidget{
public:
    GxsIdChooserWt(Wt::WContainerWidget* parent = 0);

    void loadIds();
    RsGxsId getSelectedId();

private:
    Wt::WComboBox* _idCombo;
    std::list<RsGxsId> _ownIds;
};
