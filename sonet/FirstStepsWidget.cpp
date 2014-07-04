#include "FirstStepsWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WTemplate>
#include <Wt/WBreak>
#include "CreatePostWidget2.h"
#include "AvatarGroupWidget.h"

namespace RsWall{

FirstStepsWidget::FirstStepsWidget(Wt::WContainerWidget *parent):
    WCompositeWidget(parent)
{
    Wt::WContainerWidget* container = new Wt::WContainerWidget();
    setImplementation(container);
    Wt::WTemplate* templ = new Wt::WTemplate(container);
    templ->setTemplateText(
                "<h1>Retroshare social network plugin: first steps</h1>"
                "<p>(goal: these steps should be very easy and obvious to do.)</p>"
                "<p>1. connect to your friends (we can't remove this step, only can make it simple)</p>"
                "<p>2. if you don't have a gxs-identity, create one. (see the top right corner)"
                " (idea: create a pgp-linked id by default. can derive the name from the pgp-name"
                " (but this is the job of rsidentities))</p>"
                "<p>3. use the widget below to create a post. this should automatically create a wall. The list below should show your new wall.</p>"
                "<p>while(!isPerfect){ read the code, give feedback, contribute }</p>"
                "<p>How the wall works: every user has a wall. you can make posts on your wall and on the walls of others."
                " posts can be shared. then the post will appear on another wall. currently only public posts are possible.</p>"
                "<p>The widget below shows all currently known identities with a wall."
                " Later a Filter should be applied to show only interesting identities. Like \"People you may know\"."
                " The social network starts by subscribing to another wall.</p>"
                "<p>NOTE: the event handling is buggy. For an unknown reason it can happen that the timer which should check if the ui needs updates stops working. Then reload the page.</p>"
                );

    new Wt::WBreak(container);
    new Wt::WLabel("Use the CreatePostWidget2 below to create a post on your own wall.", container);
    new Wt::WBreak(container);
    new CreatePostWidget2(container);

    new Wt::WBreak(container);
    new Wt::WLabel("A list of all currently know identities which have a wall. Click on the name label or avatar image to view the wall.", container);
    new Wt::WBreak(container);
    new AvatarGroupWidget(container);
}
}//namespace RsWall
