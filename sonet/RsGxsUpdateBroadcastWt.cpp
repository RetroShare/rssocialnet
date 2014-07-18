#include "RsGxsUpdateBroadcastWt.h"

#include <map>
#include <Wt/WApplication>

//#include "WebUITimer.h"

namespace RsWall{

RsMutex updateBroadcastMapMtx = RsMutex("RsGxsUpdateBroadcastWt::updateBroadcastMapMtx");
std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*> updateBroadcastMap;

RsGxsUpdateBroadcastWt* RsGxsUpdateBroadcastWt::get(RsGxsIfaceHelper *ifaceImpl)
{
    RsStackMutex stack(updateBroadcastMapMtx);

    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator it = updateBroadcastMap.find(ifaceImpl);
    if(it != updateBroadcastMap.end()){
        return it->second;
    }else{
        RsGxsUpdateBroadcastWt* bc = new RsGxsUpdateBroadcastWt(ifaceImpl);
        updateBroadcastMap.insert(std::pair<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>(ifaceImpl, bc));
        return bc;
    }
}

void RsGxsUpdateBroadcastWt::tick()
{
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*> localMap;
    {
        // copy the whole map while the mutex is locked
        RsStackMutex stack(updateBroadcastMapMtx);
        localMap = updateBroadcastMap;
    }
    // don't have to fear deadlocks now
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator mit;
    for(mit = localMap.begin(); mit != localMap.end(); mit++)
    {
        mit->second->_tick();
    }
}

void RsGxsUpdateBroadcastWt::unregisterApplication()
{
    RsStackMutex stack(updateBroadcastMapMtx);
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator mit;
    for(mit = updateBroadcastMap.begin(); mit != updateBroadcastMap.end(); mit++)
    {
        mit->second->_unregisterApplication();
    }
}

// no need for locking here
// because cleanup should only be called at the very end
void RsGxsUpdateBroadcastWt::cleanup()
{
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator it;
    for(it = updateBroadcastMap.begin(); it != updateBroadcastMap.end(); it++)
    {
        delete it->second;
    }
    updateBroadcastMap.clear();
}

GrpsChangedSignal& RsGxsUpdateBroadcastWt::grpsChanged()
{
    RsStackMutex stack(_mMtx);
    GrpsChangedSignal* sig;
    if(_mGrpsChangedSignals.find(Wt::WApplication::instance()) == _mGrpsChangedSignals.end())
    {
        sig = new GrpsChangedSignal();
        _mGrpsChangedSignals[Wt::WApplication::instance()] = sig;
    }
    else
    {
        sig = _mGrpsChangedSignals[Wt::WApplication::instance()];
    }
    return *sig;
}

MsgsChangedSignal& RsGxsUpdateBroadcastWt::msgsChanged()
{
    RsStackMutex stack(_mMtx);
    MsgsChangedSignal* sig;
    if(_mMsgsChangedSignals.find(Wt::WApplication::instance()) == _mMsgsChangedSignals.end())
    {
        sig = new MsgsChangedSignal();
        _mMsgsChangedSignals[Wt::WApplication::instance()] = sig;
    }
    else
    {
        sig = _mMsgsChangedSignals[Wt::WApplication::instance()];
    }
    return *sig;
}

RsGxsUpdateBroadcastWt::RsGxsUpdateBroadcastWt(RsGxsIfaceHelper* ifaceImpl):
    _mMtx("RsGxsUpdateBroadcastWt::_mMtx"), mIfaceImpl(ifaceImpl)
{
}

void RsGxsUpdateBroadcastWt::_tick()
{
    RsStackMutex stack(_mMtx);

    if(mIfaceImpl->updated(true, true)){
        std::list<RsGxsGroupId> grpIds;
        mIfaceImpl->groupsChanged(grpIds);
        if(!grpIds.empty()){
            std::cerr << "RsGxsUpdateBroadcastWt::_tick(): emitting grps Changed" << std::endl;
            std::map<Wt::WApplication*, GrpsChangedSignal* >::iterator mit;
            std::vector<std::map<Wt::WApplication*, GrpsChangedSignal*>::iterator > toDelete;
            for(mit = _mGrpsChangedSignals.begin(); mit != _mGrpsChangedSignals.end(); mit++)
            {
                Wt::WApplication::UpdateLock lock(mit->first);
                if(lock)
                {
                    mit->second->emit(grpIds);
                    mit->first->triggerUpdate();
                }
                else
                {
                    // lock did not suceed because the applications is going to get destroyed
                    // mark this entry for removal
                    toDelete.push_back(mit);
                }
            }
            // this is a really crazy template usage
            std::vector<std::map<Wt::WApplication*, GrpsChangedSignal*>::iterator >::iterator vit;
            for(vit = toDelete.begin(); vit != toDelete.end(); vit++)
            {
                delete (*vit)->second;
                _mGrpsChangedSignals.erase(*vit);
            }
        }

        std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > msgIds;
        mIfaceImpl->msgsChanged(msgIds);
        if(!msgIds.empty()){
            std::cerr << "RsGxsUpdateBroadcastWt::_tick(): emitting msgs Changed" << std::endl;
            std::map<Wt::WApplication*, MsgsChangedSignal*>::iterator mit;
            std::vector<std::map<Wt::WApplication*, MsgsChangedSignal*>::iterator > toDelete;
            for(mit = _mMsgsChangedSignals.begin(); mit != _mMsgsChangedSignals.end(); mit++)
            {
                Wt::WApplication::UpdateLock lock(mit->first);
                if(lock)
                {
                    mit->second->emit(msgIds);
                    mit->first->triggerUpdate();
                }
                else
                {
                    // lock did not suceed because the applications is going to get destroyed
                    // mark this entry for removal
                    toDelete.push_back(mit);
                }
            }
            std::vector<std::map<Wt::WApplication*, MsgsChangedSignal*>::iterator >::iterator vit;
            for(vit = toDelete.begin(); vit != toDelete.end(); vit++)
            {
                delete (*vit)->second;
                _mMsgsChangedSignals.erase(*vit);
            }
        }
    }
}

void RsGxsUpdateBroadcastWt::_unregisterApplication()
{
    RsStackMutex stack(_mMtx);

    std::map<Wt::WApplication*, GrpsChangedSignal* >::iterator mit;
    mit = _mGrpsChangedSignals.find(Wt::WApplication::instance());
    if(mit == _mGrpsChangedSignals.end())
    {
        std::cerr << "RsGxsUpdateBroadcastWt::_unregisterApplication() "
                     "ERROR this application is not in the _mGrpsChangedSignals map, this should not happen" << std::endl;
    }
    else
    {
        delete mit->second;
        _mGrpsChangedSignals.erase(mit);
    }

    std::map<Wt::WApplication*, MsgsChangedSignal* >::iterator mit2;
    mit2 = _mMsgsChangedSignals.find(Wt::WApplication::instance());
    if(mit2 == _mMsgsChangedSignals.end())
    {
        std::cerr << "RsGxsUpdateBroadcastWt::_unregisterApplication() "
                     "ERROR this application is not in the _mMsgsChangedSignals map, this should not happen" << std::endl;
    }
    else
    {
        delete mit2->second;
        _mMsgsChangedSignals.erase(mit2);
    }

}

/*
std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*> updateBroadcastMap;

RsGxsUpdateBroadcastWt* RsGxsUpdateBroadcastWt::get(RsGxsIfaceHelper *ifaceImpl)
{
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator it = updateBroadcastMap.find(ifaceImpl);
    if(it != updateBroadcastMap.end()){
        return it->second;
    }else{
        RsGxsUpdateBroadcastWt* bc = new RsGxsUpdateBroadcastWt(ifaceImpl);
        updateBroadcastMap.insert(std::pair<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>(ifaceImpl, bc));
        return bc;
    }
}

void RsGxsUpdateBroadcastWt::cleanup()
{
    std::map<RsGxsIfaceHelper*, RsGxsUpdateBroadcastWt*>::iterator it;
    for(it = updateBroadcastMap.begin(); it != updateBroadcastMap.end(); it++)
    {
        delete it->second;
    }
    updateBroadcastMap.clear();
}

void RsGxsUpdateBroadcastWt::onTimer()
{
    if(mIfaceImpl->updated(true, true)){
        std::list<RsGxsGroupId> grpIds;
        mIfaceImpl->groupsChanged(grpIds);
        if(!grpIds.empty()){
            std::cerr << "RsGxsUpdateBroadcastWt::onTimer(): emitting grps Changed" << std::endl;
            _mGrpsChangedSignal.emit(grpIds);
        }

        std::map<RsGxsGroupId, std::vector<RsGxsMessageId> > msgIds;
        mIfaceImpl->msgsChanged(msgIds);
        if(!msgIds.empty()){
            std::cerr << "RsGxsUpdateBroadcastWt::onTimer(): emitting msgs Changed" << std::endl;
            _mMsgsChangedSignal.emit(msgIds);
        }
    }

    WebUITimer::singleShotNextTick(this, &RsGxsUpdateBroadcastWt::onTimer);
}

RsGxsUpdateBroadcastWt::RsGxsUpdateBroadcastWt(RsGxsIfaceHelper* ifaceImpl):
    WObject(),
    mIfaceImpl(ifaceImpl)
{
    WebUITimer::singleShotNextTick(this, &RsGxsUpdateBroadcastWt::onTimer);
}
*/
}//namespace RsWall
