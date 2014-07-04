#include "CreatePostWidget.h"

#include <Wt/WContainerWidget>
#include <Wt/WLabel>

#include "rswall.h"

namespace RsWall{

CreatePostWidget::CreatePostWidget(Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    _messageTextArea = new Wt::WTextArea(this);
    _messageTextArea->clicked().connect(this, &CreatePostWidget::textAreaClicked);
    // can use setEmptyText instead of this
    enterDefaultText();

    new Wt::WLabel("select the author of the new post", this);
    _idChooser = new GxsIdChooserWt(this);

    new Wt::WLabel("select on which wall the post appears", this);
    _wallChooser = new WallChooserWidget(this);

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

        RsGxsId author = _idChooser->getSelectedId();
        if(author.isNull()){
            return;
        }
        msg.mMeta.mAuthorId = author;

        RsGxsGroupId targetWall = _wallChooser->getSelectedWallId();
        if(targetWall.isNull()){
            return;
        }
        // this is a trick:
        // use the group id to signal the wall the post should appear on
        msg.mMeta.mGroupId = targetWall;

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
}//namespace RsWall
