#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
#include <Wt/WTimer>

#include <retroshare/rsidentity.h>

// a dropdown bo to select a id
// own ids only
namespace RsWall{
class GxsIdChooserWt: public Wt::WContainerWidget{
public:
    GxsIdChooserWt(Wt::WContainerWidget* parent = 0);

    void loadIds();
    RsGxsId getSelectedId();

private:
    Wt::WTimer mTimer;
    Wt::WComboBox* _idCombo;
    std::list<RsGxsId> _ownIds;
};
}//namespace RsWall
