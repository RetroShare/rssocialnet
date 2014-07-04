#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
#include <Wt/WSignal>

#include <retroshare/rsgxsifacetypes.h>

#include "TokenQueueWt2.h"

namespace RsWall{
class WallChooserWidget: public Wt::WContainerWidget
{
public:
    WallChooserWidget(Wt::WContainerWidget* parent = 0);
    RsGxsGroupId getSelectedWallId();

    Wt::Signal<void>& selectionChanged(){ return _SelectionChangedSignal;}
private:
    void requestWallIds();
    void tokenCallback(uint32_t token, bool ok);
    void onSelectionChanged(int);
    Wt::Signal<void> _SelectionChangedSignal;
    TokenQueueWt2 _TokenQueue;
    Wt::WComboBox* _WallCombo;
    std::vector<std::pair<RsGxsId, RsGxsGroupId> > _DataVec;
};
}//namespace RsWall
