#pragma once

#include <Wt/WSplitButton>
#include <Wt/WSignal>

#include <sonet/TokenQueueWt2.h>

#include <retroshare/rsgxsifacetypes.h>

// this button can be used in auto-mode
//   use setGrpdId() to tell the button which content to share on click
// or this button can be used in manual mode
//   just connect to the signal and then get the selected wall-id
namespace RsWall{
class ShareButton: public Wt::WSplitButton
{
public:
    ShareButton(Wt::WContainerWidget* parent = 0);

    // set the grp-id of the content to be shared
    //   this widget will then handle the creation of a reference message
    void setGrpId(RsGxsGroupId grpId){ this->grpId = grpId; }
    // use this signal if you want to handle the sharing on your own
    // TODO: extend this to allow to define target-wall owner?
    Wt::Signal<RsGxsId, RsGxsCircleId >& shareClicked(){ return shareClickedSignal; }

private:
    void onShareClicked();
    void onDropdownClicked();
    void onTokenReady(uint32_t token, bool ok);
    TokenQueueWt2 tokenQueue;
    RsGxsGroupId grpId;
    Wt::Signal<RsGxsId, RsGxsCircleId > shareClickedSignal;
};
}//namespace RsWall
