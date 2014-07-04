#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>

#include "ShareButton.h"
#include "TokenQueueWt2.h"

namespace RsWall{
// this widget allows to create a new RootWallPost
// it shows a TextArea, controls to select the audience and a submit button
class CreatePostWidget2: public Wt::WContainerWidget{
public:
    CreatePostWidget2(Wt::WContainerWidget* parent = 0);
private:
    void onShareClicked(const RsGxsId& authorId, const RsGxsCircleId& circleId);
    TokenQueueWt2 _tokenQueue;
    void onTokenReady(uint32_t token, bool ok);
    Wt::WTextArea* _messageTextArea;
    ShareButton* _shareButton;
};

} // namespace RsWall
