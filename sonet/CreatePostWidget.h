#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>
#include <Wt/WPushButton>

#include "GxsIdChooserWt.h"

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
    Wt::WPushButton* _submitPushButton;
};
