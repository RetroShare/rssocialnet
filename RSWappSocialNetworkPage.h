#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WBorderLayout>
#include <Wt/WMenu>

#include <retroshare/rsgxsifacetypes.h>

namespace RsWall{ class WallWidget; }

class RSWappSocialNetworkPage : public Wt::WCompositeWidget
{
public:
    RSWappSocialNetworkPage(Wt::WContainerWidget* parent = 0);

    void showWall(const RsGxsId& authorId);
    void setOwnId(const RsGxsId& authorId);

private:
    Wt::WContainerWidget* _pageContainer;
    Wt::WBorderLayout* _pageLayout;
    Wt::WContainerWidget* _leftContainer;
    Wt::WContainerWidget* _centerContainer;
    Wt::WMenu* _menu;
    RsWall::WallWidget* _ownWallWidget;
    RsWall::WallWidget* _generalWallWidget;
};
