#pragma once

#include "Wt/WLabel"
#include "retroshare/rsidentity.h"

namespace RsWall{
// this widget shows the name of a given gxs-id
class IdentityLabelWidget: public Wt::WLabel
{
public:
    IdentityLabelWidget(Wt::WContainerWidget* parent = 0);
    void setIdentity(const RsGxsId& id);
private:
    void tryLoadIdentity();
    RsGxsId mIdentity;
};
}//namespace RsWall
