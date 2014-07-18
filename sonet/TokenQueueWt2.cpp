#include "TokenQueueWt2.h"

#include "WebUITimer.h"

namespace RsWall{

TokenQueueWt2::TokenQueueWt2(RsTokenService *tokenService):
    mTokenService(tokenService)
{
    WebUITimer::singleShotNextTick(this, &TokenQueueWt2::onTimer);
}

void TokenQueueWt2::onTimer()
{
    std::list<uint32_t>::iterator it;
    for(it = mTokens.begin(); it != mTokens.end(); it++)
    {
        uint32_t token = *it;
        bool ready = false;
        bool ok = false;
        uint32_t tokenStatus = mTokenService->requestStatus(token);
        if(tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
        {
            ready = true;
            ok = true;
        }
        else if (tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
        {
            ready = true;
            ok = false;
        }
        if(ready)
        {
            _mTokenSignal.emit(token, ok);

            if(it == mTokens.begin())
            {
                mTokens.erase(it);
                it = mTokens.begin();
            }
            else
            {
                // this returns the next element
                it = mTokens.erase(it);
                // don't skip he next element
                it--;
            }
        }
    }

    WebUITimer::singleShotNextTick(this, &TokenQueueWt2::onTimer);
}
}//namespace RsWall
