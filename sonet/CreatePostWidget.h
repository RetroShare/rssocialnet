#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>
#include <Wt/WPushButton>

#include "GxsIdChooserWt.h"
#include "WallChooserWidget.h"

namespace RsWall{
// TODO: maybe remove this widget

// this widget allos to create a new RootWallPost
// it shows a TextArea, controls to select the audience and a submit button
class CreatePostWidget: public Wt::WContainerWidget{
public:
    CreatePostWidget(Wt::WContainerWidget* parent = 0);

    void textAreaClicked();
    void submitMessage();

private:
    bool _defaultText;
    void enterDefaultText();
    Wt::WTextArea* _messageTextArea;
    GxsIdChooserWt* _idChooser;
    WallChooserWidget* _wallChooser;
    Wt::WPushButton* _submitPushButton;
};

} // namespace RsWall
