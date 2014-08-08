#include "IdentityHandler.h"

#include <retroshare/rsidentity.h>

#include "Operators.h"
#include "ApiTypes.h"

namespace resource_api
{

IdentityHandler::IdentityHandler(RsIdentity *identity):
    mRsIdentity(identity)
{
    addResourceHandler("*", this, &IdentityHandler::handleWildcard);
    addResourceHandler("own", this, &IdentityHandler::handleOwn);
}

void IdentityHandler::handleWildcard(Request &req, Response &resp)
{
    bool ok = true;

    if(req.mMethod == Request::POST)
    {
        RsIdentityParameters params;
        req.mStream << makeKeyValueReference("name", params.nickname);
        if(req.mStream.isOK())
        {
            uint32_t token;
            mRsIdentity->createIdentity(token, params);
            // not sure if should acknowledge the token
            // for now go the easier way
        }
        else
        {
            ok = false;
        }
    }
    else
    {
        RsTokReqOptions opts;
        opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
        uint32_t token;
        mRsIdentity->getTokenService()->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts);

        time_t start = time(NULL);
        while((mRsIdentity->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
              &&(mRsIdentity->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
              &&((time(NULL) < (start+10)))
              )
        {
            Sleep(500);
        }

        if(mRsIdentity->getTokenService()->requestStatus(token) == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
        {
            std::vector<RsGxsIdGroup> grps;
            ok &= mRsIdentity->getGroupData(token, grps);
            for(std::vector<RsGxsIdGroup>::iterator vit = grps.begin(); vit != grps.end(); vit++)
            {
                RsGxsIdGroup& grp = *vit;
                KeyValueReference<RsGxsGroupId> id("id", grp.mMeta.mGroupId);
                KeyValueReference<RsPgpId> pgp_id("pgp_id",grp.mPgpId );
                // not very happy about this, i think the flags should stay hidden in rsidentities
                bool own = (grp.mMeta.mSubscribeFlags & GXS_SERV::GROUP_SUBSCRIBE_ADMIN);
                bool pgp_linked = (grp.mMeta.mGroupFlags & RSGXSID_GROUPFLAG_REALID);
                resp.mStream.getStreamToMember()
                        << id
                        << pgp_id
                        << makeKeyValueReference("name", grp.mMeta.mGroupName)
                        << makeKeyValueReference("own", own)
                        << makeKeyValueReference("pgp_linked", pgp_linked);
            }
        }
        else
        {
            ok = false;
        }
    }

    if(ok)
    {
        resp.mReturnCode = 0;
    }
    else
    {
        resp.mReturnCode = 1;
    }
}

void IdentityHandler::handleOwn(Request &req, Response &resp)
{
    std::list<RsGxsId> ids;
    mRsIdentity->getOwnIds(ids);
    time_t start = time(NULL);
    bool ok = false;
    // have to try to get the identity details multiple times, until they are cached
    while(!ok && (time(NULL)< (start+10)))
    {
        ok = true;
        for(std::list<RsGxsId>::iterator lit = ids.begin(); lit != ids.end(); lit++)
        {
            RsIdentityDetails details;
            if(!lit->isNull())
            {
                // this returns cached data and false if the data is not yet cached
                ok &= mRsIdentity->getIdDetails(*lit, details);
                if(ok)
                {
                    resp.mStream.getStreamToMember()
                            << makeKeyValueReference("name",    details.mNickname)
                            << makeKeyValueReference("id",      details.mId)
                            << makeKeyValueReference("pgp_id",  details.mPgpId)
                            << makeKeyValueReference("pgp_linked", details.mPgpLinked)
                            << makeKeyValueReference("own",     details.mIsOwnId);
                    // set the id null, to mark it as done
                    lit->clear();
                }
            }
        }
        Sleep(500);
    }
    resp.mReturnCode = 0;
}

} // namespace resource_api
