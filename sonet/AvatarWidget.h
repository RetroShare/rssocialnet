#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WImage>
#include <Wt/WLabel>

#include <retroshare/rsgxsifacetypes.h>

/*
  what is this good for?
  - display the avatar image itself
  - (show the name)
  - show if this id is anon or pgp-signed

  what a about the id?
  - only the id is unique, but have two ids: RsGxsId and pgp-id
  - these ids are just ugly
  - maybe maintain a whitelist of "friendly nodes"

  - maybe display some action buttons and more info when the user moves the mouse over the avatar image
     - want to hide things to have a clean user interface
     - if the mouse area is large enaugh, the user will find out quickly that some info is hidden

  have to listen for changed avatar images?
  - maybe later
*/
class AvatarWidget: public Wt::WContainerWidget
{
public:
    AvatarWidget(bool small, Wt::WContainerWidget* parent = 0);
    void setIdentity(RsGxsId& identity);
private:
    Wt::WLabel* _mNameLabel;
    Wt::WImage* _mAvatarImage;
};
