#pragma once

#include <Wt/WContainerWidget>
#include "TokenQueueWt2.h"
namespace RsWall{
// show a group of avatar widgets
// for first show all known identities which have a wall-grp
class AvatarGroupWidget: public Wt::WContainerWidget
{
public:
    AvatarGroupWidget(Wt::WContainerWidget* parent = 0);
private:
    void requestWallMetas();
    void onTokenReady(uint32_t token, bool ok);
    void onGrpChange();
    TokenQueueWt2 mTokenQueue;
};
} // namespace RsWall
