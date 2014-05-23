#pragma once

#include <gxs/rsgenexchange.h>

class p3WallService: public RsGenExchange{
public:

    // callback from RsGenexchange to get notified about group/msg changes
    // handle auto subscribe here
    // can get group meta from RsDataService::retrieveGxsGrpMetaData
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes);
};
