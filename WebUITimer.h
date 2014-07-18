#pragma once

#include <list>
#include <time.h>
#include <boost/function.hpp>
#include <util/rsthreads.h>
namespace Wt{ class WServer; }

#include "RSWApplication.h"

// a server side timer
// Wt::Wtimer is client side
//
// WebUITimer::singeShot() can be used as a drop-in replecedment for Wt::WTimer
// usage:
//   // init
//   Wt::WServer* server = new Wt::WServer(...);
//   WebUITimer timer;
//   timer.setServer(server);
//
//   // in the aplication startup:
//   WApplication::enableUpdates(true);
//
//   // in a widget or else where
//   WebUITimer::singleShot(ms_until_fire, this, &MyClass::MyMember);
//
//   // call tick from any thread
//   timer.tick();
//
// WebUITimer will call WApplication::triggerUpdate(); to cause a ui refresh

class WebUITimer
{
public:
    WebUITimer(): mMtx("WebUITimer Mtx"), mServer(NULL) {}

    // set the server where the evens should be posted to
    // note: it would be possible to get the server from Wt::WApplication::instance()->environment().server()
    // but we can't know when the server gets destroyed, so we have to set and remove the server by hand
    void setServer(Wt::WServer* server)     { mServer = server; }
    void reset()    { RsStackMutex stack(mMtx); mServer = NULL; mEvents.clear();}

    // call this from the WApplication thread to schedule timer events
    // note: this function is static, and can serve as a drop in replacement for Wt::WTimer::singeShot()
    // this function has to run in a Wt::WApplication context where Wt::WApplication::instance() is valid
    template <class T, class V>
    static void singleShot(int msec, T *receiver, void (V::*method)());

    template <class T, class V>
    static void singleShotNextTick(T *receiver, void (V::*method)());

    // call this periodically from a thread
    void tick();

private:
    RsMutex mMtx; // to protect the event list
    Wt::WServer* mServer;

    class Event{
    public:
        time_t when;
        std::string sessionId;
        boost::function<void()> functionToCall;
    };

    std::list<Event> mEvents;
};

// note about Wt::WTimer
// having troubles with the timer stopping
// testing the advice from http://redmine.webtoolkit.eu/boards/2/topics/2181
// it reccomends to use this safe timer implementation
//#include "wtsafetimer.h"
// result: did not work, even this timer stops working
// maybe hav to replace all usages of Wt::WTimer with WtSafeTimer?
//  or there is a problem somwhere else

template <class T, class V>
void WebUITimer::singleShot(int msec, T *receiver, void (V::*method)())
{
    WebUITimer& timer = *RSWApplication::getTimer();
    Wt::WApplication& application = *Wt::WApplication::instance();

    RsStackMutex stack(timer.mMtx);

    // for now only full seconds
    int sec = msec / 1000;

    Event event;
    event.when = time(NULL) + sec;
    event.sessionId = application.sessionId();
    // Wt::WApplication::bind() protects the call when the receiving objects gets deleted
    // the call will simply not happen after the receiving object was deleted
    event.functionToCall = application.bind(boost::bind(method, receiver));
    timer.mEvents.push_back(event);
}

template <class T, class V>
void WebUITimer::singleShotNextTick(T *receiver, void (V::*method)())
{
    singleShot(0, receiver, method);
}
