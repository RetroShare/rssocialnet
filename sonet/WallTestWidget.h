#pragma once

#include <Wt/WContainerWidget>

#include "WallChooserWidget.h"
#include "WallWidget.h"

namespace RsWall{
class WallTestWidget: public Wt::WContainerWidget
{
public:
    WallTestWidget(Wt::WContainerWidget* parent = 0);

private:
    void onWallSelectionChanged();
    WallChooserWidget* mWallChooserWidget;
    WallWidget* mWallWidget;
};
}//namespace RsWall
