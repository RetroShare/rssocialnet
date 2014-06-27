#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include <retroshare/rsgxsifacetypes.h>
#include "sonet/WallWidget.h"

using namespace Wt;

class RsPlugInInterfaces ;

class RSWApplication: public WApplication
{
public:
    RSWApplication(const WEnvironment& env, const RsPlugInInterfaces& interf);

    void showWall(const RsGxsId &id);

private:
    Wt::WTabWidget *tabW;
    WallWidget* wallWidget;
};

