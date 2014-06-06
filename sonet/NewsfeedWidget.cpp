#include "NewsfeedWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

#include "sonet/CreatePostWidget.h"

NewsfeedWidget::NewsfeedWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    new Wt::WLabel("this is the NewsfeedWidget", this);
    new CreatePostWidget(this);
}
