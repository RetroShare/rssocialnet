#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WLabel>

#include <retroshare/rsgxsifacetypes.h>

#include "IdentityLabelWidget.h"
#include "TokenQueueWt2.h"

/*
  what is this good for?
  - display the avatar image itself
  - (show the name)
  - show if this id is anon or pgp-signed

  what about the id?
  - only the id is unique, but have two ids: RsGxsId and pgp-id
  - these ids are just ugly
  - maybe maintain a whitelist of "friendly nodes"

  - maybe display some action buttons and more info when the user moves the mouse over the avatar image
     - want to hide things to have a clean user interface
     - if the mouse area is large enough, the user will find out quickly that some info is hidden

  have to listen for changed avatar images?
  - maybe later
*/
namespace RsWall{
// this class is called AvatarWidgetWt to avoid a name collision with AvatarWidget from rs-gui
// a name collision leads to a runtime crash on linux
class AvatarWidgetWt: public Wt::WContainerWidget
{
public:
    AvatarWidgetWt(bool small, Wt::WContainerWidget* parent = 0);
    void setIdentity(RsGxsId& identity);
private:
    void onLabelClicked();
    RsGxsId _mId;
    IdentityLabelWidget* _mIdentityLabel;
    uint32_t _mImageSize;
    Wt::WImage* _mAvatarImage;
    boost::shared_ptr<Wt::WMemoryResource> _mAvatarImageRessource;

    TokenQueueWt2 _mTokenQueue;
    void onTokenReady(uint32_t token, bool ok);

    void onGrpsChanged(const std::list<RsGxsGroupId>& grpIds);
};

} // namespace RsWall
