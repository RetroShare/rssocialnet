#include "WallTestWidget.h"

#include <Wt/WLabel>

WallTestWidget::WallTestWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    new Wt::WLabel("This is a page for testing the wall. select a wall to show its content", this);

    mWallChooserWidget = new WallChooserWidget(this);
    mWallChooserWidget->selectionChanged().connect(this, &WallTestWidget::onWallSelectionChanged);

    mWallWidget = new WallWidget(this);
}

void WallTestWidget::onWallSelectionChanged()
{
    mWallWidget->setWallId(mWallChooserWidget->getSelectedWallId());
}
