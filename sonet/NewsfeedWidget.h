#pragma once

#include <Wt/WContainerWidget>

namespace RsWall{
class ActivityWidget;

// display a activity from a single person or a group
// "bob and alice shared this post"
class NewsfeedWidget: public Wt::WContainerWidget{
public:
    NewsfeedWidget(Wt::WContainerWidget* parent = 0);
private:
    void checkActivities();
    Wt::WContainerWidget* mActivitiesContainer;
    std::vector<ActivityWidget*> mActivityWidgets;
};
}//namespace RsWall
