#include "WebUITimer.h"

#include <Wt/WServer>

namespace WebUITimerHelper{
// function object to trigger an update in the application thread
class TriggerUpdate{
public:
    void operator()()
    {
        Wt::WApplication::instance()->triggerUpdate();
    }
};
}

void WebUITimer::tick()
{
    RsStackMutex stack(mMtx);

    if(mServer == NULL){
        std::cerr << "WebUITimer::tick(): wrong usage error. Please call setServer() before ticking the timer." << std::endl;
        return;
    }
    std::vector<std::list<Event>::iterator > eventsToDelete;
    for(std::list<Event>::iterator lit = mEvents.begin(); lit != mEvents.end(); lit++)
    {
        Event& event = *lit;
        if(event.when <= time(NULL))
        {
            mServer->post(event.sessionId, event.functionToCall);
            // trigger an update
            // this propagates the changes to the browser
            // do it here, then all the receiving objects don't have to worry about it
            // this makes WebUITimer behave very similar as Wt::WTimer
            mServer->post(event.sessionId, WebUITimerHelper::TriggerUpdate());
            eventsToDelete.push_back(lit);
        }
    }
    // erase handled events
    std::vector<std::list<Event>::iterator >::iterator vit;
    for(vit = eventsToDelete.begin(); vit != eventsToDelete.end(); vit++)
    {
        mEvents.erase(*vit);
    }
}
