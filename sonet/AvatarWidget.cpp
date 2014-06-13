#include "AvatarWidget.h"

#include <Wt/WRasterImage>
#include <Wt/WRectF>
#include <Wt/WBreak>

AvatarWidget::AvatarWidget(bool small, Wt::WContainerWidget *parent):
    WContainerWidget(parent), _mImg(NULL)
{
    // only for testing
    _mNameLabel = new Wt::WLabel("no id", this);
    RsGxsId id;
    setIdentity(id);

    new Wt::WBreak(this);

    _mAvatarImage = new Wt::WImage(this);
    if(small){
        // resize has impact on the sourounding widgets
        // have to find another way to define size of this
        //_mAvatarImage->resize(40, 40);
        // set the html attribute for the img tag
        // this has the same effekt: it destroys the layout of other widgets
        //_mAvatarImage->setAttributeValue("height","40");
        //_mAvatarImage->setAttributeValue("width","40");
        // have to resize the image properly using image functions

        //for testing: limt the size
        //setHeight(40);
        //setWidth(40);
    }else{
        //_mAvatarImage->resize(80, 80);
        //_mAvatarImage->setAttributeValue("height","80");
        //_mAvatarImage->setAttributeValue("width","80");

        // for testing: limit the size
        //setHeight(80);
        //setWidth(80);
    }
}

void AvatarWidget::setIdentity(RsGxsId &identity)
{
    // create image from id for testing

    // probably won't work because of missing graphic libraries
    /*
    if(_mImg)
    {
        _mAvatarImage->setImageLink(Wt::WLink());
        delete _mImg;
    }
    _mImg = new Wt::WRasterImage("png", 40, 40);
    _mImg->drawText(Wt::WRectF(0, 0, 60, 60), Wt::AlignLeft, Wt::TextSingleLine, identity.toStdString());

    _mAvatarImage->setImageLink(Wt::WLink(_mImg));
    */

    // get identity info from rsindetities
    //   repeat this step if the info is not cached yet

    // get image from wallservice and display it
    //   do it the same way rsidentities does it?
    //   or use the token system?

    // load image from memory
    //Wt::WMemoryResource();

    // fill label with truncated id string
    _mNameLabel->setText(identity.toStdString().substr(0,5));
}
