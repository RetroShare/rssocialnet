#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>
#include <Wt/WPushButton>

#include "GxsIdChooserWt.h"
#include "TokenQueueWt2.h"

namespace RsWall{
// experimental widget to create a new wall
// it shows a box to select an identity and a TextArea for the profile text
class CreateWallWidget: public Wt::WContainerWidget
{
public:
    CreateWallWidget(Wt::WContainerWidget* parent = 0);
private:
    void createWall();
    void tokenCallback(uint32_t token, bool ok);
    TokenQueueWt2 _mTokenQueue;
    Wt::WTextArea* _profileText;
    GxsIdChooserWt* _idChooser;
    Wt::WPushButton* _submitPushButton;
};
}//namespace RsWall
