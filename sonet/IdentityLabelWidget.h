#pragma once

#include "Wt/WLabel"
#include <Wt/WTimer>

#include "retroshare/rsidentity.h"

namespace RsWall{
class IdentityLabelWidget: public Wt::WLabel
{
public:
    IdentityLabelWidget(Wt::WContainerWidget* parent = 0);
    void setIdentity(const RsGxsId& id);
private:
    void tryLoadIdentity();
    RsGxsId mIdentity;
    Wt::WTimer mTimer;
};
}//namespace RsWall
