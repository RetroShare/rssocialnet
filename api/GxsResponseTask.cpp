#include "GxsResponseTask.h"

#include "Operators.h"

namespace resource_api
{

GxsResponseTask::GxsResponseTask(RsIdentity *id_service, RsTokenService *token_service):
    mIdService(id_service), mTokenService(token_service),
    mDone(false)
{

}

bool GxsResponseTask::doWork(Request &req, Response &resp)
{
    bool ready = true;
    // check if gxs requests are ready
    if(!mWaitingTokens.empty())
    {
        for(std::vector<uint32_t>::iterator vit = mWaitingTokens.begin(); vit != mWaitingTokens.end(); ++vit)
        {
            if(  mTokenService->requestStatus(*vit) == RsTokenService::GXS_REQUEST_V2_STATUS_PENDING
               ||mTokenService->requestStatus(*vit) == RsTokenService::GXS_REQUEST_V2_STATUS_PARTIAL)
            {
                ready = false;
            }
        }
    }
    // check if we have identities to fetch
    bool more = true;
    while(!mIdentitiesToFetch.empty() && more)
    {
        RsGxsId id = mIdentitiesToFetch.back();
        RsIdentityDetails details;
        if(mIdService->getIdDetails(id, details))
        {
            mIdentitiesToFetch.pop_back();
            mIdentityDetails.push_back(details);
        }
        else
        {
            more = false; // pause when an id failed, to give the service time tim fetch the data
            ready = false;
        }
    }
    if(!ready)
        return true; // want to continue later

    gxsDoWork(req, resp);

    if(mDone) return false;
    else return true;
}

void GxsResponseTask::addWaitingToken(uint32_t token)
{
    mWaitingTokens.push_back(token);
}

void GxsResponseTask::done()
{
    mDone = true;
}

void GxsResponseTask::requestGxsId(RsGxsId id)
{
    mIdentitiesToFetch.push_back(id);
}

void GxsResponseTask::streamGxsId(RsGxsId id, StreamBase &stream)
{
    // will see if this works or if we have to use an index
    for(std::vector<RsIdentityDetails>::iterator vit = mIdentityDetails.begin();
        vit != mIdentityDetails.end(); ++vit)
    {
        if(vit->mId == id)
        {
            stream << makeKeyValueReference("gxs_id", id)
                   << makeKeyValueReference("is_own", vit->mIsOwnId)
                   << makeKeyValueReference("name", vit->mNickname)
                   << makeKeyValueReference("pgp_linked", vit->mPgpLinked)
                   << makeKeyValueReference("pgp_known", vit->mPgpKnown);
            return;
        }
    }
}

} // namespace resource_api
