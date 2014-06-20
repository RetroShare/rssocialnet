#pragma once

#include <Wt/WContainerWidget>
#include <Wt/WTextArea>

#include "TokenQueueWt2.h"
#include "rswall.h"
#include "WallRootPostWidget.h"

class WallWidget: public Wt::WContainerWidget{
public:
    WallWidget(Wt::WContainerWidget* parent = 0);

    void setWallId(const RsGxsGroupId& id);

private:
    void reload();
    void tokenCallback(uint32_t token, bool ok);
    RsGxsGroupId _GrpId;
    TokenQueueWt2 _TokenQueue;
    Wt::WTextArea* _TextArea;
    std::vector<WallRootPostWidget*> _PostWidgets;

};
