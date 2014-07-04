#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>

#include "TokenQueueWt2.h"
#include "rswall.h"
#include "WallRootPostWidget.h"
#include "IdentityLabelWidget.h"
#include "AvatarWidget.h"

namespace RsWall{
class WallWidget: public Wt::WContainerWidget{
public:
    WallWidget(Wt::WContainerWidget* parent = 0);

    // this fn was first, for testing
    // and this fn will likely be removed
    void setWallId(const RsGxsGroupId& id);

    // this fn is new
    void setWallByAuthorId(const RsGxsId& id);

private:
    void loadProfile();
    void reload();
    void checkIfOwnWall();
    void checkIfSubscribed();
    void onSubscribeClicked();
    void displaySubscribeStatus();
    void tokenCallback(uint32_t token, bool ok);
    void onEditClicked();
    RsGxsId _AuthorId;
    RsGxsGroupId _GrpId;
    TokenQueueWt2 _TokenQueue;
    AvatarWidgetWt* _AvatarWidget;
    Wt::WContainerWidget* _ProfileContainer;
    Wt::WTextArea* _ProfileEdit;
    Wt::WPushButton* _EditButton;
    // TODO: remove things like id which are in the group
    WallGroup _Grp;
    bool isEditing;
    Wt::WTextArea* _TextArea;
    Wt::WPushButton* _SubscribeButton;
    bool subscribed;
    std::vector<WallRootPostWidget*> _PostWidgets;
    uint32_t _WallGroupToken;
    uint32_t _PostMsgToken;

    void onAvatarImageUploaded();
    Wt::WPushButton* _EditAvatarButton;
    Wt::WFileUpload* _AvatarFileUpload;
    Wt::WPushButton* _UploadAvatarButton;
};
}//namespace RsWall
