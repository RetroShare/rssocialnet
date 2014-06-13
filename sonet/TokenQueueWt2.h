#pragma once

#include <Wt/WTimer>
#include <Wt/WSignal>
#include <retroshare/rsgxsiface.h>

// the idea for this class is roughly stolen from TokenQueue and GxsTokenQueue
// but this class has a different callback:
//   the handling function gets called with the token and the tokenstatus as parameters
// the callback is implemented as Wt::Signal<>
// this class has only a list where it stores tokens
// the tokens are checked for their status
// if the token status is complete or fail, then the callback is invoked

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
    Wt::WTimer mTimer;
};
