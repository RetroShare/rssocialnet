#include "NewsfeedWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WBreak>
#include <algorithm>

#include "sonet/CreatePostWidget.h"
#include "IdentityLabelWidget.h"
#include "WallRootPostWidget.h"
#include "rswall.h"
#include "WebUITimer.h"

namespace RsWall{

// a quick and dirty widget to display the following:
// - who shared this post
// - who commented on this post
// - the post itself
//
// TODO: figure out how to display a list of identities in a nice way
class ActivityWidget: public Wt::WContainerWidget
{
public:
    // grp id should be a of type post group
    ActivityWidget(const RsGxsGroupId& grpId, Wt::WContainerWidget* parent = 0):
        WContainerWidget(parent), mReferencedGroupId(grpId)
    {
        mShareIdentityLabelContainer = new Wt::WContainerWidget(this);
        new Wt::WLabel("shared ", this);

        mCommentIdentityLabelContainer = new Wt::WContainerWidget(this);
        new Wt::WLabel(" made a comment on the following post:", this);

        new WallRootPostWidget(grpId, this);
    }
    RsGxsGroupId mReferencedGroupId;
    Wt::WContainerWidget* mShareIdentityLabelContainer;
    std::vector<IdentityLabelWidget*> mShareIdentityLabelWidgets;

    Wt::WContainerWidget* mCommentIdentityLabelContainer;
    std::vector<IdentityLabelWidget*> mCommentIdentityLabelWidgets;

    void updateShareIdentities(const std::vector<RsGxsId>& identities)
    {
        // assuming the vector is only updated at the end
        // then it is enough to add the new elements from the end which are not present in the own list
        // even the own list of identity label widgets is then not needed
        // an integer for size would be enough
        for(uint32_t i = mShareIdentityLabelWidgets.size(); i < identities.size(); i++)
        {
            IdentityLabelWidget* label = new IdentityLabelWidget(mShareIdentityLabelContainer);
            new Wt::WLabel(" ", mShareIdentityLabelContainer);
            label->setIdentity(identities[i]);
            mShareIdentityLabelWidgets.push_back(label);
        }
    }
    // same for comment
    void updateCommentIdentities(const std::vector<RsGxsId>& identities)
    {
        for(uint32_t i = mCommentIdentityLabelWidgets.size(); i < identities.size(); i++)
        {
            IdentityLabelWidget* label = new IdentityLabelWidget(mCommentIdentityLabelContainer);
            new Wt::WLabel(" ", mCommentIdentityLabelContainer);
            label->setIdentity(identities[i]);
            mCommentIdentityLabelWidgets.push_back(label);
        }
    }
};

NewsfeedWidget::NewsfeedWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    new Wt::WLabel("this is the NewsfeedWidget, it shows when others share or comment a post", this);
    //new Wt::WBreak(this);
    //new Wt::WLabel("sorry, no newsfeed yet", this);
    //new Wt::WLabel("below is a CreatePostWidget", this);
    //new CreatePostWidget(this);
    mActivitiesContainer = new Wt::WContainerWidget(this);

    NewsfeedWidget::checkActivities();
}

void NewsfeedWidget::checkActivities()
{
    std::vector<Activity> activities;
    rsWall->getCurrentActivities(activities);
    std::vector<ActivityWidget*> widgetsToKeep;
    for(std::vector<Activity>::iterator vit = activities.begin(); vit != activities.end(); vit++)
    {
        ActivityWidget* widget = NULL;
        // search for an existing widget
        for(std::vector<ActivityWidget*>::iterator vit2 = mActivityWidgets.begin(); vit2 != mActivityWidgets.end(); vit2++)
        {
            if(vit->mReferencedGroup == (*vit2)->mReferencedGroupId)
            {
                widget = *vit2;
            }
        }
        if(widget == NULL)
        {
            widget = new ActivityWidget(vit->mReferencedGroup);
            // insert widget at the first position
            mActivitiesContainer->insertWidget(0, widget);
        }
        widget->updateShareIdentities(vit->mShared);
        widget->updateCommentIdentities(vit->mCommented);
        widgetsToKeep.push_back(widget);
    }
    // remove old widgets
    for(std::vector<ActivityWidget*>::iterator vit = mActivityWidgets.begin(); vit != mActivityWidgets.end(); vit++)
    {
        if(std::find(widgetsToKeep.begin(), widgetsToKeep.end(), *vit) == widgetsToKeep.end())
        {
            delete *vit;
        }
    }
    mActivityWidgets.swap(widgetsToKeep);
    WebUITimer::singleShotNextTick(this, &NewsfeedWidget::checkActivities);
}
}//namespace RsWall
