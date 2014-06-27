#include "IdentityLabelWidget.h"

IdentityLabelWidget::IdentityLabelWidget(Wt::WContainerWidget *parent):
    WLabel(parent)
{
    mTimer.setInterval(100);
    mTimer.timeout().connect(this, &IdentityLabelWidget::tryLoadIdentity);
    mTimer.setSingleShot(true);
}

void IdentityLabelWidget::setIdentity(const RsGxsId &id)
{
    mIdentity = id;
    mTimer.start();
    setText(id.toStdString().substr(0,5));
}

void IdentityLabelWidget::tryLoadIdentity()
{
    RsIdentityDetails details;
    if(rsIdentity->getIdDetails(mIdentity, details))
    {
        setText(details.mNickname);
    }
    else
    {
        // id not cached, try again later
        mTimer.start();
    }
}
