#include "WallHandler.h"

#include "Operators.h"
#include "ApiTypes.h"

#include "rswall.h"
#include <retroshare/rsidentity.h>
#include <util/radix64.h>
#include "GxsResponseTask.h"

#include <algorithm>
#include <unistd.h>
#include <time.h>

namespace resource_api
{

//using namespace RsWall;

// returns the first wall for a given gxs identity
class WallTask: public GxsResponseTask
{
public:
    WallTask(RsWall::RsWall* wall, RsIdentity* identities, RsGxsId id):
        GxsResponseTask(identities, wall->getTokenService()),
        mWall(wall), mId(id), mState(BEGIN){}
private:
    RsWall::RsWall* mWall;
    RsGxsId mId;
    enum State {BEGIN, WAITING};
    State mState;
    uint32_t mToken;
protected:
    virtual void gxsDoWork(Request &req, Response &resp)
    {
        switch(mState)
        {
        case BEGIN:
        {
            mWall->requestWallGroups(mToken, mId);
            addWaitingToken(mToken);
            requestGxsId(mId);
            mState = WAITING;
            break;
        }
        case WAITING:
        {
            std::vector<RsWall::WallGroup> wgs;
            mWall->getWallGroups(mToken, wgs);
            if(wgs.empty())
            {
                resp.setOk();// not sure if should set ok, fail or warning. the response is ok, but it is empty
                resp.mDebug << "WallPostsTask::gxsDoWork() Warning: no wall group for author id=" << mId.toStdString() << " found." << std::endl;
                done();
                break;
            }
            // for now only the first group
            RsWall::WallGroup& wg = wgs.front();

            // make base63 string out of images
            // TODO: find and implement better solution
            // TODO: use correct mime type, don't assume jpeg
            std::string wi_b64;
            Radix64::encode((const char *)wg.mWallImage.mData.data(),wg.mWallImage.mData.size(),wi_b64);
            wi_b64 = "data:image/jpeg;base64," + wi_b64;

            std::string ai_b64;
            Radix64::encode((const char *)wg.mAvatarImage.mData.data(),wg.mAvatarImage.mData.size(),ai_b64);
            ai_b64 = "data:image/jpeg;base64," + ai_b64;

            streamGxsId(mId, resp.mDataStream.getStreamToMember("identity"));
            resp.mDataStream << makeKeyValueReference("profile_text", wg.mProfileText)
                             << makeKeyValueReference("wall_image", wi_b64)
                             << makeKeyValueReference("avatar_image", ai_b64);
            resp.setOk();
            done();
            break;
        }
        }
    }
};

class ActivitiesTask: public GxsResponseTask
{
public:
    ActivitiesTask(RsWall::RsWall* wall, RsIdentity* identities):
        GxsResponseTask(identities, wall->getTokenService()),
        mWall(wall), mState(BEGIN), mPostGroupToken(0){}
private:
    RsWall::RsWall* mWall;
    enum State {BEGIN, WAITING_POST_GROUP, WAITING_POST_MSGS, WAITING_IDENTITIES};
    State mState;

    std::vector<RsWall::Activity> mActivities;

    uint32_t mPostGroupToken;
    std::list<RsGroupMetaData> mPostGrpMetas;
    std::vector<uint32_t> mPostMsgTokens;
    std::vector<RsWall::PostMsg> mPostMsgs;
protected:
    virtual void gxsDoWork(Request &req, Response &resp)
    {
        switch(mState)
        {
        case BEGIN:
        {
            mWall->getCurrentActivities(mActivities);

            if(mActivities.empty())
            {
                // mark result as list and return
                resp.mDataStream.getStreamToMember();
                resp.setOk();
                done();
            }

            // collect ids of referenced post grps
            std::list<RsGxsGroupId> postGrpIds;
            for(std::vector<RsWall::Activity>::iterator vit = mActivities.begin(); vit != mActivities.end(); vit++)
            {
                RsWall::Activity& activity = *vit;
                postGrpIds.push_back(activity.mReferencedGroup);
            }
            // request post grps
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;
            mWall->getTokenService()->requestGroupInfo(mPostGroupToken, RS_TOKREQ_ANSTYPE_SUMMARY, opts, postGrpIds);
            addWaitingToken(mPostGroupToken);
            mState = WAITING_POST_GROUP;
            break;
        }
        case WAITING_POST_GROUP:
        {
            mWall->getGroupSummary(mPostGroupToken, mPostGrpMetas);

            // request the msgs
            for(std::vector<RsWall::Activity>::iterator vit = mActivities.begin(); vit != mActivities.end(); vit++)
            {
                RsWall::Activity& activity = *vit;

                RsTokReqOptions opts;
                opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
                uint32_t token;
                std::list<RsGxsGroupId> grpIdList;
                grpIdList.push_back(activity.mReferencedGroup);
                mWall->getTokenService()->requestMsgInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, grpIdList);
                mPostMsgTokens.push_back(token);
                addWaitingToken(token);
            }
            mState = WAITING_POST_MSGS;
            break;
        }
        case WAITING_POST_MSGS:
        {
            // get the msgs
            // it could happend that a mesage is not available
            // maybe should handle this
            for(std::vector<uint32_t>::iterator vit = mPostMsgTokens.begin(); vit != mPostMsgTokens.end(); vit++)
            {
                RsWall::PostMsg pm;
                mWall->getPostMsg(*vit, pm);
                mPostMsgs.push_back(pm);
            }

            // request the names for the involved author ids
            for(std::vector<RsWall::Activity>::iterator vit = mActivities.begin(); vit != mActivities.end(); vit++)
            {
                RsWall::Activity& activity = *vit;
                for(std::vector<RsGxsId>::iterator vit = activity.mShared.begin(); vit != activity.mShared.end(); vit++)
                {
                    requestGxsId(*vit);
                }
                for(std::vector<RsGxsId>::iterator vit = activity.mCommented.begin(); vit != activity.mCommented.end(); vit++)
                {
                    requestGxsId(*vit);
                }
            }
            for(std::list<RsGroupMetaData>::iterator lit = mPostGrpMetas.begin(); lit != mPostGrpMetas.end(); lit++)
            {
                RsGroupMetaData& meta = *lit;
                requestGxsId(meta.mAuthorId);
            }
            mState = WAITING_IDENTITIES;
            break;
        }
        case WAITING_IDENTITIES:
        {
            // put everything together in one response
            for(std::vector<RsWall::Activity>::iterator vit = mActivities.begin(); vit != mActivities.end(); vit++)
            {
                StreamBase& stream = resp.mDataStream.getStreamToMember();
                RsWall::Activity& activity = *vit;
                StreamBase& stream_shared = stream.getStreamToMember("shared");
                stream_shared.getStreamToMember();
                for(std::vector<RsGxsId>::iterator vit = activity.mShared.begin(); vit != activity.mShared.end(); vit++)
                {
                    streamGxsId(*vit, stream_shared.getStreamToMember());
                }
                StreamBase& stream_commented = stream.getStreamToMember("commented");
                stream_commented.getStreamToMember();
                for(std::vector<RsGxsId>::iterator vit = activity.mCommented.begin(); vit != activity.mCommented.end(); vit++)
                {
                    streamGxsId(*vit, stream_commented.getStreamToMember());
                }

                RsGroupMetaData postGrpMeta;
                for(std::list<RsGroupMetaData>::iterator postGrpMetaIt = mPostGrpMetas.begin(); postGrpMetaIt != mPostGrpMetas.end(); postGrpMetaIt++)
                {
                    if(postGrpMetaIt->mGroupId == activity.mReferencedGroup)
                    {
                        postGrpMeta = *postGrpMetaIt;
                    }
                }
                RsWall::PostMsg pm;
                for(std::vector<RsWall::PostMsg>::iterator pmvit = mPostMsgs.begin(); pmvit != mPostMsgs.end(); pmvit++)
                {
                    RsWall::PostMsg& pmref = *pmvit;
                    if(pmref.mMeta.mGroupId == activity.mReferencedGroup)
                    {
                        pm = pmref;
                    }
                }
                StreamBase& stream_post = stream.getStreamToMember("post");
                streamGxsId(postGrpMeta.mAuthorId, stream_post.getStreamToMember("author"));
                stream_post << makeKeyValueReference("post_text", pm.mPostText)
                            << makeKeyValueReference("id", postGrpMeta.mGroupId);
                resp.setOk();
                done();
            }
        }
        }
    }
};

// this task does not make sense for normal operation
// but is may be usefull for debugging
// to see what walls are in the database
class ListWallsTask: public GxsResponseTask
{
public:
    ListWallsTask(RsWall::RsWall* wall, RsIdentity* identities):
        GxsResponseTask(identities, wall->getTokenService()),
        mWall(wall), mState(BEGIN), mToken(0){}
private:
    RsWall::RsWall* mWall;
    enum State {BEGIN, WAITING_METAS, WAITING_IDENTITIES};
    State mState;

    uint32_t mToken;
    std::vector<RsGroupMetaData> mMetas;
protected:
    virtual void gxsDoWork(Request &req, Response &resp)
    {
        switch(mState)
        {
        case BEGIN:
        {
            // list walls
            mWall->requestWallGroupMetas(mToken, RsGxsId());
            addWaitingToken(mToken);
            mState = WAITING_METAS;
            break;
        }
        case WAITING_METAS:
        {
            mWall->getWallGroupMetas(mToken, mMetas);
            for(std::vector<RsGroupMetaData>::iterator vit = mMetas.begin(); vit != mMetas.end(); vit++)
            {
                requestGxsId(vit->mAuthorId);
            }
            mState = WAITING_IDENTITIES;
            break;
        }
        case WAITING_IDENTITIES:
        {
            for(std::vector<RsGroupMetaData>::iterator vit = mMetas.begin(); vit != mMetas.end(); vit++)
            {
                RsGroupMetaData& meta = *vit;
                StreamBase& stream = resp.mDataStream.getStreamToMember();

                KeyValueReference<RsGxsGroupId> wallId("wall_id", meta.mGroupId);
                KeyValueReference<RsGxsCircleId> circleId("todo_circle_id", meta.mCircleId);
                stream << wallId << circleId;
                streamGxsId(meta.mAuthorId, stream.getStreamToMember("owner"));
            }
            resp.setOk();
            done();
            break;
        }
        }
    }
};

class WallPostsTask: public GxsResponseTask
{
public:
    WallPostsTask(RsWall::RsWall* wall, RsIdentity* identities, RsGxsId id):
        GxsResponseTask(identities, wall->getTokenService()),
        mWall(wall), mState(BEGIN), mId(id), mWallGrpsToken(0), mReferenceMsgsToken(0){}
private:
    RsWall::RsWall* mWall;
    enum State {BEGIN, WAITING_WALL_GRPS, WAITING_REF_MSGS, WAITING_POST_MSGS, WAITING_IDENTITIES};
    State mState;

    RsGxsId mId;

    uint32_t mWallGrpsToken;
    uint32_t mReferenceMsgsToken;
    std::vector<RsWall::ReferenceMsg> mRefMsgs;
    uint32_t mPostMsgsToken;
    std::vector<RsWall::PostMsg> mPostMsgs;
protected:
    virtual void gxsDoWork(Request &req, Response &resp)
    {
        switch(mState)
        {
        case BEGIN:
        {
            mWall->requestWallGroups(mWallGrpsToken, mId);
            addWaitingToken(mWallGrpsToken);
            requestGxsId(mId);
            mState = WAITING_WALL_GRPS;
            break;
        }
        case WAITING_WALL_GRPS:
        {

            std::vector<RsWall::WallGroup> wgs;
            mWall->getWallGroups(mWallGrpsToken, wgs);
            if(wgs.empty())
            {
                resp.setOk();// not sure if should set ok, fail or warning. the response is ok, but it is empty
                resp.mDebug << "WallPostsTask::gxsDoWork() Warning: no wall group for author id=" << mId.toStdString() << " found." << std::endl;
                done();
                break;
            }
            RsWall::WallGroup& wg = wgs.front();

            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
            std::list<RsGxsGroupId> grpIds;
            grpIds.push_back(wg.mMeta.mGroupId);
            mWall->getTokenService()->requestMsgInfo(mReferenceMsgsToken, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);
            addWaitingToken(mReferenceMsgsToken);
            mState = WAITING_REF_MSGS;
            break;
        }
        case WAITING_REF_MSGS:
        {
            mWall->getPostReferenceMsgs(mReferenceMsgsToken, mRefMsgs);
            std::list<RsGxsGroupId> postGrpsToFetch;
            for(std::vector<RsWall::ReferenceMsg>::iterator vit = mRefMsgs.begin(); vit != mRefMsgs.end(); ++vit)
            {
                RsWall::ReferenceMsg& rmsg = *vit;
                postGrpsToFetch.push_back(rmsg.mReferencedGroup);
                requestGxsId(rmsg.mMeta.mAuthorId);
            }
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
            mWall->getTokenService()->requestMsgInfo(mPostMsgsToken, RS_TOKREQ_ANSTYPE_DATA, opts, postGrpsToFetch);
            addWaitingToken(mPostMsgsToken);
            mState = WAITING_POST_MSGS;
            break;
        }
        case WAITING_POST_MSGS:
        {
            mWall->getPostMsgs(mPostMsgsToken, mPostMsgs);
            for(std::vector<RsWall::PostMsg>::iterator vit = mPostMsgs.begin(); vit != mPostMsgs.end(); ++vit)
            {
                RsWall::PostMsg& pm = *vit;
                requestGxsId(pm.mMeta.mAuthorId);
            }
            // TODO: what about comments? load comments in extra call? oder insert here?

            mState = WAITING_IDENTITIES;
            break;
        }
        case WAITING_IDENTITIES:
        {
            resp.mDataStream.getStreamToMember();
            for(std::vector<RsWall::ReferenceMsg>::iterator vit = mRefMsgs.begin(); vit != mRefMsgs.end(); ++vit)
            {
                StreamBase& refMsgStream = resp.mDataStream.getStreamToMember();
                RsWall::ReferenceMsg& rmsg = *vit;
                refMsgStream << makeKeyValueReference("id", rmsg.mMeta.mMsgId);
                streamGxsId(rmsg.mMeta.mAuthorId, refMsgStream.getStreamToMember("author"));
                StreamBase& pmstream = refMsgStream.getStreamToMember("post");
                RsWall::PostMsg pm;
                // have to search the matching post message
                for(std::vector<RsWall::PostMsg>::iterator vit = mPostMsgs.begin(); vit != mPostMsgs.end(); ++vit)
                {
                    if(vit->mMeta.mGroupId == rmsg.mReferencedGroup)
                        pm = *vit;
                }
                streamGxsId(pm.mMeta.mAuthorId, pmstream.getStreamToMember("author"));
                pmstream << makeKeyValueReference("post_text", pm.mPostText);
            }
            resp.setOk();
            done();
            break;
        }
        }
    }
};

// OLD REMOVE
bool waitForTokenOrTimeout(uint32_t token, RsTokenService* tokenService)
{
    time_t start = time(NULL);
    while(   (tokenService->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
          && (tokenService->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
          && (time(NULL) < (start+10))
          )
    {
#ifdef WINDOWS_SYS
        Sleep(500);
#else
        usleep(500*1000) ;
#endif
    }
    if(tokenService->requestStatus(token) == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

WallHandler::WallHandler(StateTokenServer* sts, RsWall::RsWall* wall, RsIdentity* identity):
    mStateTokenServer(sts), mRsWall(wall), mRsIdentity(identity)
{
    addResourceHandler("*", this, &WallHandler::handleWildcard);
    addResourceHandler("activities", this, &WallHandler::handleActivities);
    addResourceHandler("wall", this, &WallHandler::handleWall);
    addResourceHandler("avatar_image", this, &WallHandler::handleAvatarImage);
    addResourceHandler("activities_new", this, &WallHandler::handleActivitiesNew);
}

ResponseTask* WallHandler::handleWildcard(Request &req, Response &resp)
{
    if(req.isGet() && !req.mPath.empty())
    {
        RsGxsId id(req.mPath.top());
        req.mPath.pop();
        if(!id.isNull() && req.mPath.empty())
        {
            return new WallTask(mRsWall, mRsIdentity, id);
        }
        if(!id.isNull() && req.mPath.top() == "posts")
        {
            return new WallPostsTask(mRsWall, mRsIdentity, id);
        }
    }
    else
    {
        return new ListWallsTask(mRsWall, mRsIdentity);
    }
    return 0;
}

ResponseTask* WallHandler::handleActivitiesNew(Request &req, Response &resp)
{
    return new ActivitiesTask(mRsWall, mRsIdentity);
}

// this function is the absolute horror
// it pulls many resources together
// this means the function has to fetch information in many little pieces from different places
// it goes like this.
// activity contains gxs-ids and a gxs-grp-id
//   fetch the names for the gxs-ids
//   fetch the grp meta for the gxs-grp-id
//     fetch the name for the author id in meta
//   fetch the post msg in the grp with the gxs-grp-id
//     fetch the name of the author id
//
// so this function is an example of how it should not be
// many different services have to fetch for example gxs-ids
// need a generic solution to fill in the names for gxs-ids
//
// idea: instead of the resource itself, add a link to the resource to the stream
// a resource resolver would then go through the stream until all links are replaced with their values
// better idea: other handlers like identity handler expose a funktion to write the identity details to a stream
void WallHandler::handleActivities(Request &req, Response &resp)
{
#ifdef REMOVE
    std::vector<RsWall::Activity> activities;
    mRsWall->getCurrentActivities(activities);

    // collect ids of referenced post grps
    std::list<RsGxsGroupId> postGrpIds;
    for(std::vector<RsWall::Activity>::iterator vit = activities.begin(); vit != activities.end(); vit++)
    {
        RsWall::Activity& activity = *vit;
        postGrpIds.push_back(activity.mReferencedGroup);
    }

    // request post grps
    RsTokReqOptions opts;
    opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;
    uint32_t token;
    mRsWall->getTokenService()->requestGroupInfo(token, RS_TOKREQ_ANSTYPE_SUMMARY, opts, postGrpIds);

    // wait for request completion
    time_t start = time(NULL);
    while((mRsWall->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
          &&(mRsWall->getTokenService()->requestStatus(token) != RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
          &&((time(NULL) < (start+10)))
          )
    {
#ifdef WINDOWS_SYS
        Sleep(500);
#else
        usleep(500*1000) ;
#endif
    }

    std::list<RsGroupMetaData> postGrpMetas;
    mRsWall->getGroupSummary(token, postGrpMetas);

    // request the msgs
    std::vector<uint32_t> postMsgTokens;
    for(std::vector<RsWall::Activity>::iterator vit = activities.begin(); vit != activities.end(); vit++)
    {
        RsWall::Activity& activity = *vit;

        RsTokReqOptions opts;
        opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
        uint32_t token;
        std::list<RsGxsGroupId> grpIdList;
        grpIdList.push_back(activity.mReferencedGroup);
        mRsWall->getTokenService()->requestMsgInfo(token, RS_TOKREQ_ANSTYPE_DATA, opts, grpIdList);
        postMsgTokens.push_back(token);
    }

    // wait for post msgs
    bool ready = false;
    time_t start2 = time(NULL);
    while(!ready && (time(NULL)<(start2+10)))
    {
        ready = true;
        for(std::vector<uint32_t>::iterator vit = postMsgTokens.begin(); vit != postMsgTokens.end(); vit++)
        {
            ready &= (mRsWall->getTokenService()->requestStatus(*vit) == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
                    ||(mRsWall->getTokenService()->requestStatus(*vit) == RsTokenService::GXS_REQUEST_V2_STATUS_FAILED);
        }
#ifdef WINDOWS_SYS
        Sleep(500);
#else
        usleep(500*1000) ;
#endif
    }

    // get the msgs
    // it could happend that a mesage is not available
    // maybe should handle this
    std::vector<RsWall::PostMsg> postMsgs;
    for(std::vector<uint32_t>::iterator vit = postMsgTokens.begin(); vit != postMsgTokens.end(); vit++)
    {
        RsWall::PostMsg pm;
        mRsWall->getPostMsg(*vit, pm);
        postMsgs.push_back(pm);
    }

    // collect the involved author ids to get their names
    std::map<RsGxsId, std::string> authors;
    for(std::vector<RsWall::Activity>::iterator vit = activities.begin(); vit != activities.end(); vit++)
    {
        RsWall::Activity& activity = *vit;
        for(std::vector<RsGxsId>::iterator vit = activity.mShared.begin(); vit != activity.mShared.end(); vit++)
        {
            authors[*vit] = "";
        }
        for(std::vector<RsGxsId>::iterator vit = activity.mCommented.begin(); vit != activity.mCommented.end(); vit++)
        {
            authors[*vit] = "";
        }
    }
    for(std::list<RsGroupMetaData>::iterator lit = postGrpMetas.begin(); lit != postGrpMetas.end(); lit++)
    {
        RsGroupMetaData& meta = *lit;
        authors[meta.mAuthorId] = "";
    }

    // now fetch the names
    time_t start3 = time(NULL);
    bool allDone = false;
    while(!allDone && (time(NULL)<(start3+10)))
    {
        allDone = true;
        for(std::map<RsGxsId, std::string>::iterator mit = authors.begin(); mit != authors.end(); mit++)
        {
            if(mit->second == "")
            {
                RsIdentityDetails details;
                if(mRsIdentity->getIdDetails(mit->first, details))
                {
                    mit->second = details.mNickname;
                }
                else
                {
                    allDone = false;
                }
            }
        }
    }

    // put everything together in one response
    for(std::vector<RsWall::Activity>::iterator vit = activities.begin(); vit != activities.end(); vit++)
    {
        StreamBase& stream = resp.mDataStream.getStreamToMember();
        RsWall::Activity& activity = *vit;
        StreamBase& stream_shared = stream.getStreamToMember("names_shared");
        for(std::vector<RsGxsId>::iterator vit = activity.mShared.begin(); vit != activity.mShared.end(); vit++)
        {
            stream_shared << makeValueReference(authors[*vit]);
        }
        StreamBase& stream_commented = stream.getStreamToMember("names_commented");
        for(std::vector<RsGxsId>::iterator vit = activity.mCommented.begin(); vit != activity.mCommented.end(); vit++)
        {
            stream_commented << makeValueReference(authors[*vit]);
        }

        RsGroupMetaData postGrpMeta;
        for(std::list<RsGroupMetaData>::iterator postGrpMetaIt = postGrpMetas.begin(); postGrpMetaIt != postGrpMetas.end(); postGrpMetaIt++)
        {
            if(postGrpMetaIt->mGroupId == activity.mReferencedGroup)
            {
                postGrpMeta = *postGrpMetaIt;
            }
        }
        RsWall::PostMsg pm;
        for(std::vector<RsWall::PostMsg>::iterator pmvit = postMsgs.begin(); pmvit != postMsgs.end(); pmvit++)
        {
            RsWall::PostMsg& pmref = *pmvit;
            if(pmref.mMeta.mGroupId == activity.mReferencedGroup)
            {
                pm = pmref;
            }
        }
        StreamBase& stream_post = stream.getStreamToMember("post");
        stream_post << makeKeyValueReference("author", authors[postGrpMeta.mAuthorId])
                    << makeKeyValueReference("post_text", pm.mPostText);


    }
#endif
}

void WallHandler::handleWall(Request &req, Response &resp)
{
#ifdef REMOVE
    if(!req.mPath.empty())
    {
        RsGxsId author(req.mPath.top());
        // todo
        //mRsWall->requestWallGroups();
    }
    else
    {
        // list walls
        uint32_t token;
        mRsWall->requestWallGroupMetas(token, RsGxsId());

        waitForTokenOrTimeout(token, mRsWall->getTokenService());

        std::vector<RsGroupMetaData> metas;
        mRsWall->getWallGroupMetas(token, metas);

        //
        for(std::vector<RsGroupMetaData>::iterator vit = metas.begin(); vit != metas.end(); vit++)
        {
            RsGroupMetaData& meta = *vit;
            StreamBase& stream = resp.mDataStream.getStreamToMember();

            KeyValueReference<RsGxsGroupId> wallId("wall_id", meta.mGroupId);
            KeyValueReference<RsGxsCircleId> circleId("todo_circle_id", meta.mCircleId);
            stream << wallId << circleId;

            time_t start = time(NULL);
            // have to try to get the identity details multiple times, until they are cached
            bool ok  = false;
            while(!ok && (time(NULL)< (start+10)))
            {
                RsIdentityDetails details;
                // this returns cached data and false if the data is not yet cached
                ok = mRsIdentity->getIdDetails(meta.mAuthorId, details);
                if(ok)
                {
                    KeyValueReference<RsGxsId> id("id", details.mId);
                    KeyValueReference<RsPgpId> pgp_id("pgp_id", details.mPgpId);
                    std::string avatar_address = "/../avatar_image/"+meta.mAuthorId.toStdString();
                    stream.getStreamToMember("author")
                            << makeKeyValueReference("name", details.mNickname)
                            << id
                            << pgp_id
                            << makeKeyValueReference("pgp_linked", details.mPgpLinked)
                            << makeKeyValueReference("own", details.mIsOwnId)
                            << makeKeyValue("avatar_address", avatar_address)
                               ;
                }
#ifdef WINDOWS_SYS
                Sleep(500);
#else
                usleep(500*1000) ;
#endif
            }
        }
    }
#endif
}

void WallHandler::handleAvatarImage(Request &req, Response &resp)
{
    if(!req.mPath.empty())
    {
        RsGxsId author(req.mPath.top());

        uint32_t token;
        mRsWall->requestWallGroups(token, author);

        waitForTokenOrTimeout(token, mRsWall->getTokenService());

        std::vector<RsWall::WallGroup> wgs;
        mRsWall->getWallGroups(token, wgs);

        if(!wgs.empty())
        {
            RsWall::WallGroup& grp = wgs.front();
            resp.mDataStream << grp.mAvatarImage.mData;
        }
    }
}

} // namespace resource_api
