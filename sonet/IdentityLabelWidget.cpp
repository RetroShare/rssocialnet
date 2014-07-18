#include "IdentityLabelWidget.h"

#include "RSWApplication.h"
#include "WebUITimer.h"

namespace RsWall{

IdentityLabelWidget::IdentityLabelWidget(Wt::WContainerWidget *parent):
    WLabel(parent)
{
    // does not make sense to load identity until the id is set
    //tryLoadIdentity();
}

void IdentityLabelWidget::setIdentity(const RsGxsId &id)
{
    mIdentity = id;
    setText(id.toStdString().substr(0,5));
    setToolTip("id="+id.toStdString());
    tryLoadIdentity();
}

void IdentityLabelWidget::tryLoadIdentity()
{
    RsIdentityDetails details;
    if(RSWApplication::ifaces().mIdentity->getIdDetails(mIdentity, details))
    {
        setText(details.mNickname);
    }
    else
    {
        // id not cached, try again later
        WebUITimer::singleShotNextTick(this, &IdentityLabelWidget::tryLoadIdentity);
    }
}
}//namespace RsWall
