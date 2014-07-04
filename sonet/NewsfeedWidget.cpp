#include "NewsfeedWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WBreak>

#include "sonet/CreatePostWidget.h"

namespace RsWall{

NewsfeedWidget::NewsfeedWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    new Wt::WLabel("this is the NewsfeedWidget", this);
    new Wt::WBreak(this);
    new Wt::WLabel("sorry, no newsfeed yet", this);
    //new Wt::WLabel("below is a CreatePostWidget", this);
    //new CreatePostWidget(this);
}
}//namespace RsWall
