#pragma once

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include <Wt/WBorderLayout>
#include <Wt/WMenu>

class RSWappSocialNetworkPage : public Wt::WCompositeWidget
{
public:
    RSWappSocialNetworkPage(Wt::WContainerWidget* parent);
private:
    Wt::WContainerWidget* _pageContainer;
    Wt::WBorderLayout* _pageLayout;
    Wt::WContainerWidget* _leftContainer;
    Wt::WContainerWidget* _centerContainer;
    Wt::WMenu* _menu;
};
