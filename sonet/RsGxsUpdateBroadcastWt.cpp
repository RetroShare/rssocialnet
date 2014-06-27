#include "RsGxsUpdateBroadcastWt.h"

#include <map>

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
}

RsGxsUpdateBroadcastWt::RsGxsUpdateBroadcastWt(RsGxsIfaceHelper* ifaceImpl):
    WObject(),// maybe have a parent here to have automated cleanup?
    mIfaceImpl(ifaceImpl), mTimer(this)
{
    mTimer.setInterval(100); // in ms
    mTimer.timeout().connect(this, &RsGxsUpdateBroadcastWt::onTimer);
    mTimer.start();
}
