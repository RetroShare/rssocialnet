#include "NewsfeedWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WBreak>

#include "sonet/CreatePostWidget.h"

NewsfeedWidget::NewsfeedWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    new Wt::WLabel("this is the NewsfeedWidget", this);
    new Wt::WBreak(this);
    new Wt::WLabel("belo is a CreatePostWidget", this);
    new CreatePostWidget(this);
}
