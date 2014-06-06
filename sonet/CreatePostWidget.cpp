#include "CreatePostWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

#include "rswall.h"

CreatePostWidget::CreatePostWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    _messageTextArea = new Wt::WTextArea(this);
    _messageTextArea->clicked().connect(this, &CreatePostWidget::textAreaClicked);
    // can use setEmptyText instead of this
    enterDefaultText();

    _idChooser = new GxsIdChooserWt(this);

    _submitPushButton = new Wt::WPushButton("post this", this);

    _submitPushButton->clicked().connect(this, &CreatePostWidget::submitMessage);
}

void CreatePostWidget::textAreaClicked()
{
    if(_defaultText){
        _messageTextArea->setText("");
        _defaultText = false;
    }
}

void CreatePostWidget::submitMessage()
{
    Wt::WString text = _messageTextArea->text();
    if(text == ""){
        // can use setEmptyText instead of this
        enterDefaultText();
    }else{
        PostMsg msg;
        uint32_t token;
        RsGxsId author;
        author = _idChooser->getSelectedId();
        if(author.isNull()){
            return;
        }
        msg.mMeta.mAuthorId = author;
        // todo: circle
        msg.mPostText = text.toUTF8();
        rsWall->createPost(token, msg);
    }
}

void CreatePostWidget::enterDefaultText()
{
    _defaultText = true;
    _messageTextArea->setText("say something");
}
