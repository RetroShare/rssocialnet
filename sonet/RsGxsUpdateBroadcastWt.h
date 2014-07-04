#pragma once

#include <Wt/WObject>
#include <Wt/WSignal>
#include <Wt/WTimer>
#include <retroshare/rsgxsifacehelper.h>
#include <list>

// having troubles with the timer stopping
// testing the advice from http://redmine.webtoolkit.eu/boards/2/topics/2181
// it reccomends to use this safe timer implementation
#include "wtsafetimer.h"
// result: did not work, even this timer stops working
// maybe havt ot replace all usages of Wt::WTimer with WtSafeTimer?
//  or there is a problem somwhere else

// many widgets are only interested in changes from a certain grp or msg type
// but only p3WallService knows the type
// should the service make id lists by type?

// idea:
// create one signal for every listener
// the listener would supply a list of grps he is interested in
// the updatebroadcast would create a signal and store it
// updatebroadcast would then filter the gp/msg ids and emit only wanted signals
// Signal.isConnected() can detect if the signal is not conneted anymore
//   can then clean up unconnected signals
// could also provide different signals for different types of new data

/**
  How to use RsGxsUpdateBroadcastWidgetWt to get updates from a gxs-service

  // get a RsGxsUpdateBroadcastWt object for the service
  RsGxsUpdateBroadcastWt* ubc = RsGxsUpdateBroadcast::get(gxs_service);
  // connect to signals
  ubc->.grpsChanged().connect(this, &YourClass::yourMethod);

  */
namespace RsWall{
class RsGxsUpdateBroadcastWt: public Wt::WObject
{
public:
    /**
      get a RsGxsUpdateBroadcast object for the given RsGxsIfaceHelper
    */
    static RsGxsUpdateBroadcastWt* get(RsGxsIfaceHelper* ifaceImpl);
    static void cleanup();

    Wt::Signal<std::list<RsGxsGroupId> >& grpsChanged(){ return _mGrpsChangedSignal; }
    Wt::Signal<GxsMsgIdResult>& msgsChanged(){ return _mMsgsChangedSignal; }

private:
    RsGxsUpdateBroadcastWt(RsGxsIfaceHelper* ifaceImpl);
    ~RsGxsUpdateBroadcastWt();

    void onTimer();

    Wt::Signal<std::list<RsGxsGroupId> > _mGrpsChangedSignal;
    Wt::Signal<GxsMsgIdResult> _mMsgsChangedSignal;

    RsGxsIfaceHelper* mIfaceImpl;
    //Wt::WTimer mTimer;
    WtSafeTimer* mSafeTimer;
};
}//namespace RsWall
