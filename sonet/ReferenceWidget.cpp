#include "ReferenceWidget.h"
#include <Wt/WDateTime>
namespace RsWall {

ReferenceWidget::ReferenceWidget(Wt::WContainerWidget *parent):
    Wt::WContainerWidget(parent),
    mIdLabel(new IdentityLabelWidget(this)), mMoreText(new Wt::WLabel(this))
{

}

void ReferenceWidget::setReferenceMsg(const ReferenceMsg &msg)
{
    mIdLabel->setIdentity(msg.mMeta.mAuthorId);
    mMoreText->setText(" shared on" + Wt::WDateTime::fromTime_t(msg.mMeta.mPublishTs).toString());
}

}// namespace RsWall
