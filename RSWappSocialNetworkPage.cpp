#include "RSWappSocialNetworkPage.h"

#include <Wt/WStackedWidget>

#include "sonet/FirstStepsWidget.h"
#include "sonet/NewsfeedWidget.h"
#include "sonet/CreateWallWidget.h"
#include "sonet/WallChooserWidget.h"
#include "sonet/WallTestWidget.h"
#include "sonet/AvatarWidget.h"

RSWappSocialNetworkPage::RSWappSocialNetworkPage(Wt::WContainerWidget *parent):
    WCompositeWidget(parent)
{
    _pageContainer = new Wt::WContainerWidget();
    setImplementation(_pageContainer);
    _pageLayout = new Wt::WBorderLayout();
    _pageContainer->setLayout(_pageLayout);

    _leftContainer = new Wt::WContainerWidget();
    _centerContainer = new Wt::WContainerWidget();

    _pageLayout->addWidget(_leftContainer, Wt::WBorderLayout::West);
    _pageLayout->addWidget(_centerContainer, Wt::WBorderLayout::Center);

    Wt::WStackedWidget *contents = new Wt::WStackedWidget();
    _centerContainer->addWidget(contents);
    _menu = new Wt::WMenu(contents, Wt::Vertical);
    //_menu->setInternalPathEnabled();
    _leftContainer->addWidget(_menu);

    _menu->addItem("First steps", new class RsWall::FirstStepsWidget());
    _menu->addItem("Newsfeed", new RsWall::NewsfeedWidget());
}
