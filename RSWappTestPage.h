#pragma once

#include <Wt/WCompositeWidget>

class RSWappTestPage : public Wt::WCompositeWidget
{
public:
    RSWappTestPage(Wt::WContainerWidget* parent);
private:
    Wt::WContainerWidget *_impl;
};
