#include "FirstStepsWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

FirstStepsWidget::FirstStepsWidget(Wt::WContainerWidget *parent):
    WCompositeWidget(parent)
{
    Wt::WContainerWidget* container = new Wt::WContainerWidget();
    setImplementation(container);
    container->addWidget(new Wt::WLabel("this is the FirstStepsWidget"));
}
