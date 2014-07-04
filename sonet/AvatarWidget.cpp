#include "AvatarWidget.h"

#include <Wt/WRasterImage>
#include <Wt/WRectF>
#include <Wt/WBreak>
#include <Wt/WMemoryResource>

#include "../RSWApplication.h"
#include "rswall.h"
#include "RsGxsUpdateBroadcastWt.h"

namespace RsWall{

AvatarWidgetWt::AvatarWidgetWt(bool small, Wt::WContainerWidget *parent):
    WContainerWidget(parent), _mImg(NULL), _mTokenQueue(rsWall->getTokenService())
{
    _mTokenQueue.tokenReady().connect(this, &AvatarWidgetWt::onTokenReady);
    RsGxsUpdateBroadcastWt::get(rsWall)->grpsChanged().connect(this, &AvatarWidgetWt::onGrpsChanged);

    _mIdentityLabel = new IdentityLabelWidget(this);
    _mIdentityLabel->clicked().connect(this, &AvatarWidgetWt::onLabelClicked);

    new Wt::WBreak(this);
    _mAvatarImage = new Wt::WImage(this);
    _mAvatarImageRessource = new Wt::WMemoryResource(this);
    _mAvatarImage->clicked().connect(this, &AvatarWidgetWt::onLabelClicked);

    new Wt::WBreak(this);
    Wt::WLabel* moretext = new Wt::WLabel("more text on mouse over", this);
    //moretext->setHiddenKeepsGeometry(true);
    moretext->hide();
    _mIdentityLabel->mouseWentOver().connect(moretext, &Wt::WLabel::show);
    _mIdentityLabel->mouseWentOut().connect(moretext, &Wt::WLabel::hide);

    _mAvatarImage->mouseWentOver().connect(moretext, &Wt::WLabel::show);
    _mAvatarImage->mouseWentOut().connect(moretext, &Wt::WLabel::hide);

    RsGxsId id;
    setIdentity(id);
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

        //_mAvatarImage->setMinimumSize(40, 40);
        //_mAvatarImage->setMaximumSize(40, 40);
    }else{
        //_mAvatarImage->resize(80, 80);
        //_mAvatarImage->setAttributeValue("height","80");
        //_mAvatarImage->setAttributeValue("width","80");

        // for testing: limit the size
        //setHeight(80);
        //setWidth(80);

        //_mAvatarImage->setMinimumSize(40, 40);
        //_mAvatarImage->setMaximumSize(40, 40);
    }
}

void AvatarWidgetWt::setIdentity(RsGxsId &identity)
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
    //     this is now outsourced to IdentityLabelWidget

    // get image from wallservice and display it
    //   do it the same way rsidentities does it?
    //   or use the token system?
    //     use token system, because these functions are already there atm
    uint32_t token;
    rsWall->requestWallGroups(token, identity);
    _mTokenQueue.queueToken(token);

    _mIdentityLabel->setIdentity(identity);
    _mId = identity;
}

void AvatarWidgetWt::onLabelClicked()
{
    RSWApplication::instance()->showWall(_mId);
}

void AvatarWidgetWt::onTokenReady(uint32_t token, bool ok)
{
    if(ok)
    {
        // load image from memory
        std::vector<WallGroup> grps;
        rsWall->getWallGroups(token, grps);
        // TODO: handle multiple wall-grps
        if(grps.empty()){ std::cerr << "PROBLEM in AvatarWidgetWt::onTokenReady(): no wall-grps for author" << std::endl; return;}

        WallGroup& grp = grps[0];
        _mAvatarImageRessource->setData(grp.mAvatarImage.mData);
        _mAvatarImage->setImageLink(Wt::WLink(_mAvatarImageRessource));
    }
    else
    {
        std::cerr << "token FAILED in AvatarImageWt::onTokenReady()" << std::cerr;
    }
}

void AvatarWidgetWt::onGrpsChanged(const std::list<RsGxsGroupId>& /*grpIds*/)
{
    // optimisation: remember from which grp the avatar image came and only update if this grp changed
    if(!_mId.isNull())
    {
        uint32_t token;
        rsWall->requestWallGroups(token, _mId);
        _mTokenQueue.queueToken(token);
    }
}
}//namespace RsWall
