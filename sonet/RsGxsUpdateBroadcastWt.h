#pragma once

#include <Wt/WObject>
#include <Wt/WSignal>
#include <list>
#include <retroshare/rsgxsifacehelper.h>
#include <util/rsthreads.h>

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
  ubc->grpsChanged().connect(this, &YourClass::yourMethod);

  // in the application destructor
  RsGxsUpdateBroadcastWt::unregisterApplication();

  // in another thread:
  RsGxsUpdateBroadcastWt::tick();

  What RsGxsUpdateBroadcastWidgetWt does and why this is so complicated
  - get events from different event sources (RsGxsIfaceHelpers)
  - distribute events to different listeners in different Wt::WApplications
  - the listeners can be in different threads (because different applications can have different threads)

  the static functions maintain one RsGxsUpdateBroadcastWt object per event source
  the object itself maintains one Wt::Signal per Wt::WApplication instance

  */
namespace RsWall{

typedef Wt::Signal<std::list<RsGxsGroupId> >    GrpsChangedSignal;
typedef Wt::Signal<GxsMsgIdResult>              MsgsChangedSignal;

class RsGxsUpdateBroadcastWt/*: public Wt::WObject*/
{
public:
    /**
      get a RsGxsUpdateBroadcast object for the given RsGxsIfaceHelper
      call this from an application thread where Wt::WApplication::instance() is valid
    */
    static RsGxsUpdateBroadcastWt* get(RsGxsIfaceHelper* ifaceImpl);

    /**
      check for updates
      call this from another thread
    */
    static void tick();
    /**
      unregister an applictaion
      call this in the application destructor
      note: this function doesn't take an argument,
            because the application is determined by Wt::WApplication::instance()
    */
    static void unregisterApplication();

    // only call cleanup at the end, when all applications are destroyed
    static void cleanup();

    // old
    //Wt::Signal<std::list<RsGxsGroupId> >& grpsChanged(){ return _mGrpsChangedSignal; }
    //Wt::Signal<GxsMsgIdResult>& msgsChanged(){ return _mMsgsChangedSignal; }
    GrpsChangedSignal& grpsChanged();
    MsgsChangedSignal& msgsChanged();

private:
    RsGxsUpdateBroadcastWt(RsGxsIfaceHelper* ifaceImpl);

    // old
    //void onTimer();
    void _tick();
    void _unregisterApplication();

    // old
    //Wt::Signal<std::list<RsGxsGroupId> > _mGrpsChangedSignal;
    //Wt::Signal<GxsMsgIdResult> _mMsgsChangedSignal;

    RsMutex _mMtx;
    std::map<Wt::WApplication*, GrpsChangedSignal* >_mGrpsChangedSignals;
    std::map<Wt::WApplication*, MsgsChangedSignal* >_mMsgsChangedSignals;

    RsGxsIfaceHelper* mIfaceImpl;
};
}//namespace RsWall
