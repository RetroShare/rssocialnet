#include "WallWidget.h"

WallWidget::WallWidget(Wt::WContainerWidget *parent):
    WCompositeWidget(parent)
{
    Wt::WContainerWidget* container = new Wt::WContainerWidget();
    setImplementation(container);
}
