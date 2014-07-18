#pragma once

#include <Wt/WSignal>
#include <retroshare/rsgxsiface.h>

/**
  TokenQueueWt2 checks tokens for their state.
  When the state is COMPLETE or FAILED, then a callback function is invoked.

  usage:

    // init
    TokenQueueWt2* tokenQueue = new TokenQueueWt2(gxs_service.getTokenService());
    tokenQueue->tokenReady().connect(this, &MyClass::onTokenReady();

    // now queue a token
    uint32_t token;
    gxs_service.getTokenService().requestSomething(token, params);
    tokenQueue->queueToken(token);

*/
// the idea for this class is roughly stolen from TokenQueue and GxsTokenQueue
// but this class has a different callback:
//   the handling function gets called with the token and the tokenstatus as parameters
// the callback is implemented as Wt::Signal<>
// this class has only a list where it stores tokens
// the tokens are checked for their status
// if the token status is complete or fail, then the callback is invoked

namespace RsWall{
// could optimize this class to let the timer only run when there are tokens
class TokenQueueWt2: public Wt::WObject
{
public:
    TokenQueueWt2(RsTokenService* tokenService);

    void queueToken(uint32_t token){ mTokens.push_back(token); }
    bool tokensWaiting(){ return !mTokens.empty(); }
    // uint32_t token, bool success
    Wt::Signal<uint32_t, bool>& tokenReady(){ return _mTokenSignal; }

private:
    void onTimer();

    std::list<uint32_t> mTokens;
    Wt::Signal<uint32_t, bool> _mTokenSignal;
    RsTokenService* mTokenService;
};
}//namespace RsWall
