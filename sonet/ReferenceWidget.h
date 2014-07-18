#pragma once

#include <Wt/WContainerWidget>
#include "IdentityLabelWidget.h"
#include "rswall.h"

namespace RsWall{
// display a small note like "bob shared one hour ago"
// this widgte is for the wall, because it displays only one identity
class ReferenceWidget: public Wt::WContainerWidget
{
public:
    ReferenceWidget(Wt::WContainerWidget* parent = 0);
    void setReferenceMsg(const ReferenceMsg& msg);
private:
    IdentityLabelWidget* mIdLabel;
    Wt::WLabel* mMoreText;
};
}// namepsace RsWall
