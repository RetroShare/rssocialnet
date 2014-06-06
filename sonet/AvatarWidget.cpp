#include "AvatarWidget.h"

AvatarWidget::AvatarWidget(bool small, Wt::WContainerWidget *parent):
    WContainerWidget(parent)
{
    _mNameLabel = new Wt::WLabel("no id set", this);
    _mAvatarImage = new Wt::WImage(this);
    if(small){
        _mAvatarImage->resize(40, 40);
    }else{
        _mAvatarImage->resize(80, 80);
    }
}

void AvatarWidget::setIdentity(RsGxsId &identity)
{
    // get identity info from rsindetities
    //   repeat this step if the info is not cached yet

    // get image from wallservice and display it
    //   do it the same way rsidentities does it?
    //   or use the token system?

    // load image from memory
    //Wt::WMemoryResource();

    _mNameLabel->setText(identity.toStdString());
}
