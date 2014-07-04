#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>
#include <Wt/WPushButton>

#include <retroshare/rsgxscommon.h>
#include <retroshare/rsgxsifacehelper.h>

#include "TokenQueueWt2.h"

#include "GxsIdChooserWt.h"

namespace RsWall{
// generic widget to show existing comments and to create a new one
// it uses the RsGxsCommentService
// todo: handle events, display only the last x comments
class CommentContainerWidget: public Wt::WContainerWidget
{
public:
    CommentContainerWidget(RsGxsCommentService* commentService, RsGxsIfaceHelper* ifaceHelper,
                           Wt::WContainerWidget* parent = 0);

    void setGrpMsgId(RsGxsGrpMsgIdPair id);

private:
    void requestComments();
    void onTokenReady(uint32_t token, bool ok);
    void onSubmitComment();
    RsGxsIfaceHelper* _IfaceHelper;
    RsGxsCommentService* _CommentService;
    RsGxsGrpMsgIdPair _GrpMsgId;
    TokenQueueWt2 _TokenQueue;
    Wt::WTextArea* _TextArea;
    GxsIdChooserWt* _IdChooser;
    Wt::WPushButton* _SubmitButton;
};

} // namespace RsWall
