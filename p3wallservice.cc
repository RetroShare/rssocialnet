#include "p3wallservice.h"

#include <retroshare/rsidentity.h>
#include <algorithm>
/*
idea: cache requests
this would reduce load if many request which return the same data
are made in a short time
a task has another member time_t expiresOn;
if this member is zero the request can not be cached
the task sets this member on completion
and then the task will be kept for this time
*/

// ********************* helper classes **************************

/*
rules for task handling
- the service creates and sets the public token, and updates its status
- doWork() is called when (time() > task.mNextTickTS)
  and all tokens in task.mPendingTokens have state RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE
- task.mPendingTokens is cleared by the service before task.doWork() is called
- if any of the pending tokens failed, the task is considered failed an will get deleted
- the task has to set mResult to FAIL or COMPLETE if all work is done
*/
namespace RsWall{
class WallServiceTask
{
public:
    WallServiceTask():
        mResult(NO_RESULT_YET),
        mWallService(NULL),
        mPublicToken(0)/*, mNextTickTS(0)*//*, mStopped(false)*/{}

    // overload this to do work
    virtual void doWork() = 0;

    enum Result { NO_RESULT_YET, FAIL, COMPLETE};

    // modified by task
    Result mResult;

    // set by service
    p3WallService* mWallService;
    // set by service
    uint32_t mPublicToken; // the public token of this task
    //time_t mNextTickTS; // set to 0 if not waiting for time

    // filled by task and cleared by service
    std::vector<uint32_t> mPendingTokens; // clear if not waiting for tokens

    //bool mStopped; // set to true if this task is done
};

class PostMsgTask: public WallServiceTask
{
public:
    PostMsgTask(const PostMsg &pm):
        WallServiceTask(), mState(BEGIN),
        mGroupToken(0), mMsgToken(0), mReferenceMsgToken(0), mPostMsg(pm)
    {}

    enum State { BEGIN, WAITING_GRP, WAITING_MSG, WAITING_REFERENCE, COMPLETED};
    State mState;
    // these are own tokens we got from the core
    uint32_t mGroupToken;// create group
    uint32_t mMsgToken;// create msg in group
    uint32_t mReferenceMsgToken; // create reference on own wall
    PostMsg mPostMsg;
    RsGxsGroupId mPostGrpId;

    virtual void doWork()
    {
        std::cerr << "PostMsgTask::doWork() token=" << mPublicToken << std::endl;
        switch(mState){
        case BEGIN:
        {
            // create the group where the post gets stored
            PostGroupItem* grpItem = new PostGroupItem();
            grpItem->meta.mAuthorId = mPostMsg.mMeta.mAuthorId;
            mWallService->publishGroup(mGroupToken, grpItem);

            mState = WAITING_GRP;
            mPendingTokens.push_back(mGroupToken);
        }
            break;
        case WAITING_GRP:
        {
            // create the msg in the new group
            mWallService->acknowledgeTokenGrp(mGroupToken, mPostGrpId); // todo: safe grp id for reference creation
            PostMsgItem* msgItem = new PostMsgItem();
            msgItem->mPostMsg = mPostMsg;
            msgItem->meta.mAuthorId = mPostMsg.mMeta.mAuthorId;
            msgItem->meta.mGroupId = mPostGrpId;
            mWallService->publishMsg(mMsgToken, msgItem);

            mState = WAITING_MSG;
            mPendingTokens.push_back(mMsgToken);
        }
            break;
        case WAITING_MSG:
        {
            // ackn msg token to get msg id (is the msg-id needed? i think not)

            // create reference on target wall group
            // can outsource the job of creating a reference msg to ReferenceMsgTask
            ReferenceMsg refMsg;
            refMsg.mType = 0; // todo: define this
            refMsg.mMeta.mAuthorId = mPostMsg.mMeta.mAuthorId;
            refMsg.mMeta.mGroupId = mPostMsg.mMeta.mGroupId;
            refMsg.mReferencedGroup = mPostGrpId;
            mWallService->createPostReferenceMsg(mReferenceMsgToken, refMsg);

            mState = WAITING_REFERENCE;
            mPendingTokens.push_back(mReferenceMsgToken);
        }
            break;
        case WAITING_REFERENCE:
            mState = COMPLETED;
            mResult = COMPLETE;
            break;
        case COMPLETED:
            break;
        }
    }
};

// this is a 1:1 copy of PostMsgTask
// except one detail: the target wall is determined by a PostReferenceParams object
//   this task uses ReferenceMsgTask2 to create the reference-msg
// not good to have two classes with the same code
// maybe can remove older class later
class PostMsgTask2: public WallServiceTask
{
public:
    PostMsgTask2(const PostReferenceParams& params, const std::string& postText):
        WallServiceTask(), mState(BEGIN), mPostReferenceParams(params), mPostText(postText),
        mGroupToken(0), mMsgToken(0), mReferenceMsgToken(0)
    {}

    enum State { BEGIN, WAITING_GRP, WAITING_MSG, WAITING_REFERENCE, COMPLETED};
    State mState;
    PostReferenceParams mPostReferenceParams;
    std::string mPostText;
    // these are own tokens we got from the core
    uint32_t mGroupToken;// create group
    uint32_t mMsgToken;// create msg in group
    uint32_t mReferenceMsgToken; // create reference on own wall
    RsGxsGroupId mPostGrpId;

    virtual void doWork()
    {
        std::cerr << "PostMsgTask::doWork() token=" << mPublicToken << std::endl;
        switch(mState){
        case BEGIN:
        {
            // create the group where the post gets stored
            PostGroupItem* grpItem = new PostGroupItem();
            grpItem->meta.mAuthorId = mPostReferenceParams.mAuthor;
            mWallService->publishGroup(mGroupToken, grpItem);

            mState = WAITING_GRP;
            mPendingTokens.push_back(mGroupToken);
        }
            break;
        case WAITING_GRP:
        {
            // create the msg in the new group
            mWallService->acknowledgeTokenGrp(mGroupToken, mPostGrpId);
            PostMsgItem* msgItem = new PostMsgItem();
            msgItem->mPostMsg.mPostText = mPostText;
            msgItem->meta.mAuthorId = mPostReferenceParams.mAuthor;
            msgItem->meta.mGroupId = mPostGrpId;
            mWallService->publishMsg(mMsgToken, msgItem);

            mState = WAITING_MSG;
            mPendingTokens.push_back(mMsgToken);
        }
            break;
        case WAITING_MSG:
        {
            // ackn msg token to get msg id (is the msg-id needed? i think not)

            // create reference on target wall group
            // can outsource the job of creating a reference msg to ReferenceMsgTask
            mPostReferenceParams.mReferencedGroupId = mPostGrpId;
            mWallService->createPostReferenceMsg(mReferenceMsgToken, mPostReferenceParams);

            mState = WAITING_REFERENCE;
            mPendingTokens.push_back(mReferenceMsgToken);
        }
            break;
        case WAITING_REFERENCE:
            mState = COMPLETED;
            mResult = COMPLETE;
            break;
        case COMPLETED:
            break;
        }
    }
};

class ReferenceMsgTask: public WallServiceTask
{
public:
    ReferenceMsgTask(const ReferenceMsg& msg):
        WallServiceTask(), mReferenceMsg(msg), mState(BEGIN), mMsgToken(0){}

    ReferenceMsg mReferenceMsg;
    enum State {BEGIN, WAITING_MSG};
    State mState;
    uint32_t mMsgToken;
    virtual void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            ReferenceMsgItem* item = new ReferenceMsgItem();
            item->meta = mReferenceMsg.mMeta;
            item->mReferenceMsg = mReferenceMsg;
            mWallService->publishMsg(mMsgToken, item);

            mState = WAITING_MSG;
            mPendingTokens.push_back(mMsgToken);
        }
            break;
        case WAITING_MSG:
        {
            // could acknwledge msg to get the id
            //   maybe later
            mResult = COMPLETE;
        }
            break;
        }
    }
};

// this task takes a PostreferenceParams object to find the desired wall
// this task can create wall-grps if target wall author is self
class ReferenceMsgTask2: public WallServiceTask
{
public:
    ReferenceMsgTask2(const PostReferenceParams& params):
        WallServiceTask(), mState(BEGIN), mParams(params)
    {}
    enum State { BEGIN, WAITING_METAS, WAITING_CREATEWALL, CREATE_REF_MSG, WAITING_MSG };
    State mState;
    PostReferenceParams mParams;
    uint32_t mGrpMetasToken;
    RsGxsGroupId mTargetWallGrpId;
    uint32_t mWallGrpToken;
    uint32_t mRefMsgToken;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            // get wall grps for this this target wall owner
            mWallService->requestWallGroupMetas(mGrpMetasToken, mParams.mTargetWallOwner);
            mState = WAITING_METAS;
            mPendingTokens.push_back(mGrpMetasToken);
        }
            break;
        case WAITING_METAS:
        {
            std::vector<RsGroupMetaData> grpMetas;
            mWallService->getWallGroupMetas(mGrpMetasToken, grpMetas);
            // search a wall-grp with the right circle settings
            for(std::vector<RsGroupMetaData>::iterator vit = grpMetas.begin(); vit != grpMetas.end(); vit++)
            {
                if(vit->mCircleId == mParams.mCircle)
                {
                    mTargetWallGrpId = vit->mGroupId;
                }
            }
            if(mTargetWallGrpId.isNull())
            {
                // check if this is our own wall
                // then create it
                // if this is not our wall, then sharing fails
                // have to think about creating wall groups for others
                std::list<RsGxsId> ownIds;
                mWallService->getRsIdentity()->getOwnIds(ownIds);
                if(std::find(ownIds.begin(), ownIds.end(), mParams.mTargetWallOwner) != ownIds.end())
                {
                    // this is our own wall, we can create a group for it
                    WallGroup grp;
                    grp.mMeta.mAuthorId = mParams.mTargetWallOwner;
                    mWallService->createWallGroup(mWallGrpToken, grp);
                    mState = WAITING_CREATEWALL;
                    mPendingTokens.push_back(mWallGrpToken);
                }
                else
                {
                    std::cerr << "FAIL in ReferenceMsgTask2::doWork mState = WAITING_METAS: can't find a wall-grp, can't make a wall-grp for others" << std::endl;
                    mResult = FAIL;
                }
            }
            else
            {
                // skip createwall
                mState = CREATE_REF_MSG;
            }
        }
            break;
        case WAITING_CREATEWALL:
        {
            mWallService->acknowledgeTokenGrp(mWallGrpToken, mTargetWallGrpId);
            mState = CREATE_REF_MSG;
        }
            break;
        // same as old createRefMsgTask
        case CREATE_REF_MSG:
        {
            ReferenceMsgItem* item = new ReferenceMsgItem();
            item->meta.mAuthorId = mParams.mAuthor;
            item->meta.mGroupId = mTargetWallGrpId;
            item->mReferenceMsg.mReferencedGroup = mParams.mReferencedGroupId;
            item->mReferenceMsg.mType = mParams.mType;
            mWallService->publishMsg(mRefMsgToken, item);

            mState = WAITING_MSG;
            mPendingTokens.push_back(mRefMsgToken);
        }
            break;
        case WAITING_MSG:
        {
            // could acknwledge msg to get the id
            //   maybe later
            mResult = COMPLETE;
        }
            break;
        }
    }
};

class SearchWallGroupMetasTask: public WallServiceTask
{
public:
    SearchWallGroupMetasTask(const RsGxsId &identity):
        WallServiceTask(), mIdentity(identity), mState(BEGIN), mGrpMetaToken(0)
    {}
    RsGxsId mIdentity;
    enum State{BEGIN, WAITING_METAS};
    State mState;
    uint32_t mGrpMetaToken;
    std::vector<RsGroupMetaData> mResultData;
    virtual void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            // requesting all metas to find those with the matching author id
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(mGrpMetaToken, RS_TOKREQ_ANSTYPE_SUMMARY, opts);

            mState = WAITING_METAS;
            mPendingTokens.push_back(mGrpMetaToken);
        }
            break;
        case WAITING_METAS:
        {
            std::list<RsGroupMetaData> groupInfo;
            mWallService->getGroupMeta(mGrpMetaToken, groupInfo);
            // filter metas where author-id == wanted-id
            for(std::list<RsGroupMetaData>::iterator it = groupInfo.begin(); it != groupInfo.end(); it++)
            {
                RsGroupMetaData& grpMeta = *it;
                // check if this is a wall-grp
                if((grpMeta.mGroupStatus>>24)==RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM)
                {
                    // allow all metas if identity is not set
                    if(mIdentity.isNull())
                    {
                        mResultData.push_back(*it);
                    }
                    else if(grpMeta.mAuthorId == mIdentity)
                    {
                        mResultData.push_back(*it);
                    }
                }
            }
            mResult = COMPLETE;
        }
            break;
        }
    }
};

class SearchWallGroupDataTask: public WallServiceTask
{
public:
    SearchWallGroupDataTask(const RsGxsId &identity):
        WallServiceTask(), mIdentity(identity), mState(BEGIN), mGrpMetaToken(0), mGrpDataToken(0){}

    RsGxsId mIdentity;
    enum State {BEGIN, WAITING_METAS, WAITING_DATA};
    State mState;
    uint32_t mGrpMetaToken;
    uint32_t mGrpDataToken;
    std::vector<WallGroup> mResultData;
    virtual void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            // first request another task to search the metas for us
            // we need the grp-ids from the metas
            mWallService->requestWallGroupMetas(mGrpMetaToken, mIdentity);

            mState = WAITING_METAS;
            mPendingTokens.push_back(mGrpMetaToken);
        }
            break;
        case WAITING_METAS:
        {
            std::vector<RsGroupMetaData> groupInfo;
            mWallService->getWallGroupMetas(mGrpMetaToken, groupInfo);
            std::list<RsGxsGroupId> grpIds;
            for(std::vector<RsGroupMetaData>::iterator it = groupInfo.begin(); it != groupInfo.end(); it++)
            {
                grpIds.push_back(it->mGroupId);
            }
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(mGrpDataToken, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);

            mState = WAITING_DATA;
            mPendingTokens.push_back(mGrpDataToken);
        }
            break;
        case WAITING_DATA:
        {
            std::vector<WallGroupItem*> grpItems;
            // this can really give us wall-grps only, because getGroupDataT can filter items
            mWallService->getGroupDataT<WallGroupItem>(mGrpDataToken, grpItems);
            for(std::vector<WallGroupItem*>::iterator it = grpItems.begin(); it != grpItems.end(); it++)
            {
                WallGroupItem* item = *it;
                WallGroup wg = item->mWallGroup;
                wg.mMeta = item->meta;
                mResultData.push_back(wg);
                delete item;
            }

            mResult = COMPLETE;
        }
            break;
        }
    }
};


// could optimize this task to not get a public token assigned
// because no one is waiting for this task

// this task copies the grp-item type into the first 8 bits of the group status
// it checks if grp should be subscribed and subscribes if needed
// it then marks the grp as processed
class ProcessGrpTask: public WallServiceTask
{
public:
    ProcessGrpTask(RsGxsGroupId grpId):
        mGrpId(grpId), mState(BEGIN), grpToken(0), isPostGrp(false)
    {}

    enum State { BEGIN, WAITING_META, WAITING_SETBITS, WAITING_SUBSCRIBE, WAITING_PROCESSED };
    RsGxsGroupId mGrpId;
    State mState;
    uint32_t grpToken;
    RsGxsId authorId;
    bool isPostGrp;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
            std::list<RsGxsGroupId> grpIds;
            grpIds.push_back(mGrpId);
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(grpToken, RS_TOKREQ_ANSTYPE_DATA, opts, grpIds);

            mState = WAITING_META;
            mPendingTokens.push_back(grpToken);
        }
            break;
        case WAITING_META:
        {
            std::vector<RsGxsGrpItem*> grpItems;
            mWallService->getGroupData(grpToken, grpItems);
            if(grpItems.empty())
            {
                std::cerr << "Error in ProcessGrpTask WAITING_META: not grp Data" << std::endl;
                mResult = FAIL;
                return;
            }
            RsGxsGrpItem* item = grpItems.front();

            authorId = item->meta.mAuthorId;

            uint8_t packetType = item->PacketSubType();
            // gxs handles status and mask in this way:
            // value = (currValue & ~mask) | (value & mask);
            uint32_t status = (packetType<<24);
            uint32_t mask = 0xFF000000;
            uint32_t grpStatusToken;
            mWallService->setGroupStatusFlags(grpStatusToken, mGrpId, status, mask);

            if(dynamic_cast<PostGroupItem*>(item))
            {
                isPostGrp = true;
            }

            std::vector<RsGxsGrpItem*>::iterator vit;
            for(vit = grpItems.begin(); vit != grpItems.end(); vit++)
            {
                delete *vit;
            }

            mState = WAITING_SETBITS;
            mPendingTokens.push_back(grpStatusToken);
        }
            break;
        case WAITING_SETBITS:
        {
            if(isPostGrp)
            {
                if(mWallService->wantedGrps.find(mGrpId) != mWallService->wantedGrps.end())
                {
                    mWallService->wantedGrps.erase(mGrpId);
                    uint32_t token;
                    mWallService->RsGenExchange::subscribeToGroup(token, mGrpId, true);
                    mPendingTokens.push_back(token);
                }
                mState = WAITING_SUBSCRIBE;
            }
            else
            {
                // this is a wall-grp
                bool subscribe = false;
                if(mWallService->shouldSubscribeTo(authorId, subscribe))
                {
                    if(subscribe)
                    {
                        uint32_t token;
                        mWallService->RsGenExchange::subscribeToGroup(token, mGrpId, true);
                        mPendingTokens.push_back(token);
                    }
                    mState = WAITING_SUBSCRIBE;
                }
                else
                {
                    // interesting authors not loaded
                    // try again later
                    mState = WAITING_SETBITS;
                }
            }

        }
            break;
        case WAITING_SUBSCRIBE:
        {
            uint32_t status = ~GXS_SERV::GXS_GRP_STATUS_UNPROCESSED;
            uint32_t mask = GXS_SERV::GXS_GRP_STATUS_UNPROCESSED;
            uint32_t token;
            mWallService->setGroupStatusFlags(token, mGrpId, status, mask);

            mState = WAITING_PROCESSED;
            mPendingTokens.push_back(token);
        }
            break;
        case WAITING_PROCESSED:
            mResult = COMPLETE;
            break;
        }
    }
};

class ProcessMsgTask: public WallServiceTask
{
public:
    ProcessMsgTask(const RsGxsGrpMsgIdPair& id): WallServiceTask(), mState(BEGIN), mGrpMsgId(id){}
    enum State { BEGIN, WAITING_MSG_DATA, WAITING_SUBSCRIBE };
    State mState;
    RsGxsGrpMsgIdPair mGrpMsgId;
    uint32_t mMsgDataToken;
    RsGxsGroupId mReferencedGroup;
    uint32_t mGrpToken;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
            GxsMsgReq msgIds;
            std::vector<RsGxsMessageId> msgIdVec;
            msgIdVec.push_back(mGrpMsgId.second);
            msgIds[mGrpMsgId.first] = msgIdVec;
            mWallService->RsGenExchange::getTokenService()->requestMsgInfo(mMsgDataToken, RS_TOKREQ_ANSTYPE_DATA, opts, msgIds);

            mPendingTokens.push_back(mMsgDataToken);
            mState = WAITING_MSG_DATA;
        }
            break;
        case WAITING_MSG_DATA:
        {
            // this is really crazy, have a vector inside a map to just get one message
            GxsMsgDataMap msgItemMap;
            mWallService->getMsgData(mMsgDataToken, msgItemMap);
            GxsMsgDataMap::iterator mit;
            RsGxsMsgItem* item = NULL;
            for( mit = msgItemMap.begin(); mit != msgItemMap.end(); mit++)
            {
                const RsGxsGroupId& grpId = mit->first;
                std::vector<RsGxsMsgItem*>& vec = mit->second;
                std::vector<RsGxsMsgItem*>::iterator vit;
                for(vit = vec.begin(); vit != vec.end(); vit++)
                {
                    if(item == NULL){
                        item = *vit;
                    } else {
                        std::cerr << "ProcessMsgTask WAITING_MSG_DATA: more than one msg item in result. "
                                     "This should not happen." << std::endl;
                        delete *vit;
                    }
                }
            }
            if(item == NULL)
            {
                std::cerr << "ProcessMsgTask WAITING_MSG_DATA: no item" << std::endl;
                mResult = FAIL;
                break;
            }
            ReferenceMsgItem* refMsg = dynamic_cast<ReferenceMsgItem*>(item);
            if(refMsg)
            {
                mReferencedGroup = refMsg->mReferenceMsg.mReferencedGroup;
                // add grp id to map
                mWallService->wantedGrps.insert(mReferencedGroup);
                // then try to subscribe
                // if the subscribe fails, then this task gets deleted
                // in this case, the grpId stays in the list of wanted grps
                // if subscribe was successful, then we can delete the id from the list of wanted grps later
                mWallService->RsGenExchange::subscribeToGroup(mGrpToken, mReferencedGroup, true);

                mPendingTokens.push_back(mGrpToken);
                mState = WAITING_SUBSCRIBE;
            }
            // don't know if have to do something with this
            //PostMsgItem* postMsg = dynamic_cast<PostMsgItem*>(item);

            // safe msg type in status bits
            // don't know if we need to have msg type in meta?

            delete item;
        }
            break;
        case WAITING_SUBSCRIBE:
        {
            // TODO: mark msg as processed
            // subscribe was successful, can remove grp-id from wanted list
            mWallService->wantedGrps.erase(mReferencedGroup);
            mResult = COMPLETE;
        }
            break;
        }
    }
};

// room for optimisation:
// request new groups only once an then pass the data to different tasks
// currently the subscribe check and this filter task request grp and msg data on their own
class FilterActivitiesGrpChangeTask: public WallServiceTask
{
public:
    FilterActivitiesGrpChangeTask(const RsGxsGroupId& grpId):
        WallServiceTask(), mState(BEGIN), mGrpId(grpId), mGrpDataToken(0) {}
    enum State { BEGIN, WAITING_DATA };
    State mState;
    RsGxsGroupId mGrpId;
    uint32_t mGrpDataToken;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_DATA;
            std::list<RsGxsGroupId> groupIds;
            groupIds.push_back(mGrpId);
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(mGrpDataToken, RS_TOKREQ_ANSTYPE_DATA, opts, groupIds);
            mPendingTokens.push_back(mGrpDataToken);
            mState = WAITING_DATA;
        }
            break;
        case WAITING_DATA:
        {
            std::vector<RsGxsGrpItem*> grpItems;
            mWallService->getGroupData(mGrpDataToken, grpItems);
            std::vector<RsGxsGrpItem*>::iterator vit;
            for(vit = grpItems.begin(); vit != grpItems.end(); vit++)
            {
                RsGxsGrpItem* item = *vit;
                WallGroupItem* wgItem = dynamic_cast<WallGroupItem*>(item);
                if(wgItem)
                {
                    // do nothing with a wall group
                    // maybe display a notice if thes wall group is from a new author?
                    // like: "new wall found"
                }
                PostGroupItem* pgItem = dynamic_cast<PostGroupItem*>(item);
                if(pgItem)
                {
                    // do nothing with a post group
                    // then this Task is completely useless?
                    // currently yes
                    // maybe i find something to do later
                }
                delete item;
            }
            mResult = COMPLETE;
        }
            break;
        }
    }
};

class FilterActivitiesMsgChangeTask: public WallServiceTask
{
public:
    FilterActivitiesMsgChangeTask(const RsGxsGrpMsgIdPair& grpMsgId):
        WallServiceTask(), mState(BEGIN), mGrpMsgId(grpMsgId), mMsgDataToken(0) {}
    enum State { BEGIN, WAITING_DATA };
    State mState;
    RsGxsGrpMsgIdPair mGrpMsgId;
    uint32_t mMsgDataToken;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_MSG_DATA;
            GxsMsgReq msgIds;
            std::vector<RsGxsMessageId> msgIdVec;
            msgIdVec.push_back(mGrpMsgId.second);
            msgIds[mGrpMsgId.first] = msgIdVec;
            mWallService->RsGenExchange::getTokenService()->requestMsgInfo(mMsgDataToken, RS_TOKREQ_ANSTYPE_DATA, opts, msgIds);

            mPendingTokens.push_back(mMsgDataToken);
            mState = WAITING_DATA;
        }
            break;
        case WAITING_DATA:
        {
            GxsMsgDataMap msgItemMap;
            mWallService->getMsgData(mMsgDataToken, msgItemMap);
            GxsMsgDataMap::iterator mit;
            for(mit = msgItemMap.begin(); mit != msgItemMap.end(); mit++)
            {
                const RsGxsGroupId& grpId = mit->first;
                std::vector<RsGxsMsgItem*>& vec = mit->second;
                std::vector<RsGxsMsgItem*>::iterator vit;
                for(vit = vec.begin(); vit != vec.end(); vit++)
                {
                    RsGxsMsgItem* item = *vit;
                    PostMsgItem* pmItem =dynamic_cast<PostMsgItem*>(item);
                    if(pmItem)
                    {
                        // what could we do here?
                    }
                    ReferenceMsgItem* rmItem = dynamic_cast<ReferenceMsgItem*>(item);
                    if(rmItem)
                    {
                        Activity* activity = NULL;
                        // don't have to lock the activities mutex for read access in the genexchange thread,
                        // because only the rsgenexchange thread can change the activities
                        // check if an activity for this root post exists
                        for(std::vector<Activity>::iterator vit = mWallService->mActivities.begin();
                            vit != mWallService->mActivities.end(); vit++)
                        {
                            if(vit->mReferencedGroup == rmItem->mReferenceMsg.mReferencedGroup)
                            {
                                activity = &(*vit);
                            }
                        }
                        // if not create one
                        if(activity == NULL)
                        {
                            // *************** BEGIN ACTIVITIES LOCKED ************************
                            // WRITE access to activities has to happen in locked state
                            RsStackMutex stack(mWallService->mActivitiesMutex);
                            mWallService->mActivities.push_back(Activity());
                            activity =&mWallService->mActivities.back();
                            activity->mReferencedGroup = rmItem->mReferenceMsg.mReferencedGroup;
                            // *************** END ACTIVITIES LOCKED **************************
                        }
                        if(rmItem->mReferenceMsg.mType == ReferenceMsg::REFTYPE_SHARE)
                        {
                            // check if this author shared it already
                            // (can happen if someone shares the same root post for different circles)
                            bool found = false;
                            std::vector<RsGxsId>::iterator vit;
                            for(vit = activity->mShared.begin(); vit != activity->mShared.end(); vit++)
                            {
                                if(*vit == rmItem->meta.mAuthorId)
                                {
                                    found = true;
                                }
                            }
                            if(!found)
                            {
                                // *************** BEGIN ACTIVITIES LOCKED ************************
                                RsStackMutex stack(mWallService->mActivitiesMutex);
                                activity->mShared.push_back(rmItem->meta.mAuthorId);
                                // *************** END ACTIVITIES LOCKED **************************
                            }
                        }
                        else if(rmItem->mReferenceMsg.mType == ReferenceMsg::REFTYPE_COMMENT)
                        {
                            // same for reftype comment
                            bool found = false;
                            std::vector<RsGxsId>::iterator vit;
                            for(vit = activity->mCommented.begin(); vit != activity->mCommented.end(); vit++)
                            {
                                if(*vit == rmItem->meta.mAuthorId)
                                {
                                    found = true;
                                }
                            }
                            if(!found)
                            {
                                // *************** BEGIN ACTIVITIES LOCKED ************************
                                RsStackMutex stack(mWallService->mActivitiesMutex);
                                activity->mCommented.push_back(rmItem->meta.mAuthorId);
                                // *************** END ACTIVITIES LOCKED **************************
                            }
                        }
                    }
                    delete item;
                }
            }
            mResult = COMPLETE;
        }
            break;
        }
    }
};

class SearchSubscribedAuthorsTask: public WallServiceTask
{
public:
    SearchSubscribedAuthorsTask(): WallServiceTask(), mState(BEGIN), mGrpMetaToken(0) {}
    enum State { BEGIN, WAITING_METAS};
    State mState;
    uint32_t mGrpMetaToken;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            // requesting all grp metas
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;
            // at the moment grp meta does not support flag filtering
            // have to filter later
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(mGrpMetaToken, RS_TOKREQ_ANSTYPE_SUMMARY, opts);

            mState = WAITING_METAS;
            mPendingTokens.push_back(mGrpMetaToken);
        }
            break;
        case WAITING_METAS:
        {
            std::list<RsGroupMetaData> groupInfo;
            mWallService->getGroupMeta(mGrpMetaToken, groupInfo);
            // this is a very expensive operation, because we check author of every group
            // potential improvement: check wall-grps only and ignore post-grps
            for(std::list<RsGroupMetaData>::iterator it = groupInfo.begin(); it != groupInfo.end(); it++)
            {
                RsGroupMetaData& grpMeta = *it;
                // check for null author-id is done in fn
                // check if grp is wall-grp and if grp is subscribed
                // then add only authors of subscribed wall-grps
                if(((grpMeta.mGroupStatus>>24)==RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM)
                        && IS_GROUP_SUBSCRIBED(grpMeta.mSubscribeFlags))
                {
                    mWallService->subscribeToAuthor(grpMeta.mAuthorId, true);
                }
            }
            mWallService->setAuthorsLoaded();
            mResult = COMPLETE;
        }
            break;
        }
    }
};

class SubscribeAllWallGrpsFromAuthorTask: public WallServiceTask
{
public:
    SubscribeAllWallGrpsFromAuthorTask(RsGxsId id, bool subscribe = true):
        mState(BEGIN), mGrpMetaToken(0), authorId(id), subscribe(subscribe)
    {}

    enum State { BEGIN, WAITING_METAS, WAITING_SUBSCRIBE };
    State mState;
    uint32_t mGrpMetaToken;
    RsGxsId authorId;
    bool subscribe;
    void doWork()
    {
        switch(mState)
        {
        case BEGIN:
        {
            // requesting all grp metas
            RsTokReqOptions opts;
            opts.mReqType = GXS_REQUEST_TYPE_GROUP_META;
            mWallService->RsGenExchange::getTokenService()->requestGroupInfo(mGrpMetaToken, RS_TOKREQ_ANSTYPE_SUMMARY, opts);

            mState = WAITING_METAS;
            mPendingTokens.push_back(mGrpMetaToken);
        }
            break;
        case WAITING_METAS:
        {
            std::list<RsGroupMetaData> groupInfo;
            std::vector<RsGxsGroupId> grpsToSubscribe;
            mWallService->getGroupMeta(mGrpMetaToken, groupInfo);
            // this is a very expensive operation, because we check every group
            for(std::list<RsGroupMetaData>::iterator it = groupInfo.begin(); it != groupInfo.end(); it++)
            {
                RsGroupMetaData& grpMeta = *it;
                if(((grpMeta.mGroupStatus>>24)==RS_PKT_SUBTYPE_WALL_WALL_GRP_ITEM)
                        && (subscribe ^ IS_GROUP_SUBSCRIBED(grpMeta.mSubscribeFlags)) //can handle subscribe and unsubscribe
                        && (grpMeta.mAuthorId == authorId))
                {
                    grpsToSubscribe.push_back(grpMeta.mGroupId);
                }
            }

            std::vector<RsGxsGroupId>::iterator vit;
            for(vit = grpsToSubscribe.begin(); vit != grpsToSubscribe.end(); vit++)
            {
                uint32_t token;
                mWallService->RsGenExchange::subscribeToGroup(token, *vit, subscribe);
                mPendingTokens.push_back(token);
            }
            mState = WAITING_SUBSCRIBE;
        }
            break;
        case WAITING_SUBSCRIBE:
            mResult = COMPLETE;
            break;
        }
    }
};

// ********************** p3WallService **************************

RsWall *rsWall = NULL;

const uint32_t WALL_MSG_STORE_PERIOD = 30*24*60*60; // in seconds

p3WallService::p3WallService(RsGeneralDataService *gds, RsNetworkExchangeService *nes, RsGixs *gixs, RsIdentity* identity):
    RsWall(this),
    // RS_SERVICE_TYPE_WALL looks redundant, because the serialiser already knows this
    RsGenExchange(gds, nes, new WallSerialiser(),
                  RS_SERVICE_TYPE_WALL, gixs, wallAuthPolicy(), WALL_MSG_STORE_PERIOD),
    GxsTokenQueue(this),
    /*_mPostTaskMtx("p3WallService _mPostTaskMtx"),*/
    authorsMtx("p3WallService authorsMtx"),
    authorsLoaded(false),
    mActivitiesMutex("p3WallServiceActivitiesMutex"),
    _mTaskMtx("p3WallService _mTaskMtx"),
    mCommentService(new p3GxsCommentService(this, RS_SERVICE_TYPE_WALL)),
    mRsIdentity(identity)
{
    // load the ids of authors we are subscribed to
    uint32_t token;
    _startTask(token, new SearchSubscribedAuthorsTask());
}

p3WallService::~p3WallService()
{
    // other services like gxschannels don't do this cleanup?
    // or at least i can't find where
    delete mCommentService;
}

RsServiceInfo p3WallService::getServiceInfo()
{
    RsServiceInfo info;
    info.mServiceName = "Wallservice (Social Network Plugin)";
    // careful: this is wrong
    // todo: how can rs warn the developer if he is doing it wrong?
    //info.mServiceType = RS_SERVICE_TYPE_WALL;
    // have to do it this way:
    info.mServiceType = (((uint32_t) RS_PKT_VERSION_SERVICE) << 24) + (((uint32_t) RS_SERVICE_TYPE_WALL) << 8);
    info.mVersionMajor = 0;
    info.mMinVersionMinor = 0;
    info.mMinVersionMajor = 0;
    info.mMinVersionMinor = 0;
    return info;
}

uint32_t p3WallService::wallAuthPolicy()
{
    uint32_t policy = 0;
    uint32_t flag = 0;

    //flag = GXS_SERV::MSG_AUTHEN_ROOT_AUTHOR_SIGN | GXS_SERV::MSG_AUTHEN_CHILD_AUTHOR_SIGN;

    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PUBLIC_GRP_BITS);
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::RESTRICTED_GRP_BITS);
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::PRIVATE_GRP_BITS);

    flag = 0;
    RsGenExchange::setAuthenPolicyFlag(flag, policy, RsGenExchange::GRP_OPTION_BITS);

    return policy;
}

// called by RsGenExchange
void p3WallService::notifyChanges(std::vector<RsGxsNotify*> &changes)
{
    // process new grps/msgs
    // split the changes down to single grpIds and grpMsgId pairs
    // this makes the processing much easier
    std::vector<RsGxsNotify*>::iterator vit;
    for(vit = changes.begin(); vit != changes.end(); vit++)
    {
        RsGxsNotify* change = *vit;

        RsGxsGroupChange* grpChange = dynamic_cast<RsGxsGroupChange*>(change);
        if(grpChange)
        {
            for(std::list<RsGxsGroupId>::iterator lit = grpChange->mGrpIdList.begin(); lit != grpChange->mGrpIdList.end(); lit++)
            {
                _checkSubscribeGrpChange(*lit, change->getType());
                _filterActivitiesGrpChange(*lit, change->getType());
            }
        }

        RsGxsMsgChange* msgChange = dynamic_cast<RsGxsMsgChange*>(change);
        if(msgChange)
        {
            std::map<RsGxsGroupId, std::vector<RsGxsMessageId> >::iterator mit;
            for(mit = msgChange->msgChangeMap.begin(); mit != msgChange->msgChangeMap.end(); mit++)
            {
                const RsGxsGroupId& grpId = mit->first;
                std::vector<RsGxsMessageId>& msgIdVec = mit->second;
                std::vector<RsGxsMessageId>::iterator vit;
                for(vit = msgIdVec.begin(); vit != msgIdVec.end(); vit++)
                {
                    _checkSubscribeMsgChange(RsGxsGrpMsgIdPair(grpId, *vit), change->getType());
                    _filterActivitiesMsgChange(RsGxsGrpMsgIdPair(grpId, *vit), change->getType());
                }
            }
        }
    }


    _filterNews(changes);
    // forward the changes
    RsGxsIfaceHelper::receiveChanges(changes);
}

// called by RsGenExchange
void p3WallService::service_tick()
{
    //std::cerr << "p3WallService::service_tick()" << std::endl;
    mCommentService->comment_tick();
    GxsTokenQueue::checkRequests();

    _doTasks();

    // old code
    //_processPostMsgTasks();
}

// called by GxsTokenQueue::checkRequests()
// it gets called when a token request is ready
void p3WallService::handleResponse(uint32_t token, uint32_t req_type)
{
    // hand over data to the ui by updating token status
}

void p3WallService::getCurrentActivities(std::vector<Activity>& activities)
{
    RsStackMutex stack(mActivitiesMutex);
    activities = mActivities;
}

void p3WallService::getNewNewsfeedEntries(std::list<NewsfeedEntry> &feeds)
{
    // tuwas
}

void p3WallService::createWallGroup(uint32_t &token, const WallGroup &grp)
{
    WallGroupItem* grpItem = new WallGroupItem();
    grpItem->mWallGroup = grp;
    grpItem->meta = grp.mMeta;
    publishGroup(token, grpItem);
}

void p3WallService::updateWallGroup(uint32_t &token, const WallGroup &grp)
{
    WallGroupItem* grpItem = new WallGroupItem();
    grpItem->mWallGroup = grp;
    grpItem->meta = grp.mMeta;
    RsGenExchange::updateGroup(token, grpItem);
}

void p3WallService::createPost(uint32_t &token, const PostMsg &msg)
{
    std::cerr << "p3WallService::createPost()" << std::endl;
    std::cerr << "msg.mMeta.mAuthorId = " << msg.mMeta.mAuthorId.toStdString() << std::endl;
    std::cerr << "msg.mPostText = " << msg.mPostText << std::endl;

    _startTask(token, new PostMsgTask(msg));

    // old code below
    /*
    // not sure how to handle the token thing
    // does the ui need feedback if the post was created?
    // yes in case of an error
    PostMsgTask task;
    task.publicToken = generatePublicToken();
    task.pm = msg;
    PostGroupItem* grpItem = new PostGroupItem();
    grpItem->meta.mAuthorId = msg.mMeta.mAuthorId;
    publishGroup(task.groupToken, grpItem);
    task.state = PostMsgTask::WAITING_GRP;
    {
        RsStackMutex stack(_mPostTaskMtx);
        _mPostMsgTasks.push_back(task);
    }
    */
}

void p3WallService::acknowledgeCreatePost(uint32_t &token)
{
    // use this fn to dispose the token?
    // or dispose old tokens?
    // does the ui need the ide of the new grp/msg?
    //   no, the ui only wants to know ok/fail
    //   the ui will leran about the new grp/msg from notification
}

void p3WallService::createPost(uint32_t &token, const PostReferenceParams &params, const std::string &postText)
{
    _startTask(token, new PostMsgTask2(params, postText));
}

void p3WallService::createPostReferenceMsg(uint32_t &token, const ReferenceMsg &refMsg)
{
    _startTask(token, new ReferenceMsgTask(refMsg));
}

void p3WallService::createPostReferenceMsg(uint32_t &token, const PostReferenceParams &params)
{
    _startTask(token, new ReferenceMsgTask2(params));
}

void p3WallService::requestWallGroupMetas(uint32_t &token, const RsGxsId &identity)
{
    // have to search all grp-metas for author id
    // so this is a expensive operation
    // maybe want to cache the grp-ids of the wall grps
    // std::map<RsGxsId, std::vector<RsGxsGroupId> >
    //   can do the cachin later
    _startTask(token, new SearchWallGroupMetasTask(identity));
}

void p3WallService::getWallGroupMetas(const uint32_t &token, std::vector<RsGroupMetaData> &grpMeta)
{
    SearchWallGroupMetasTask* task = _getTask<SearchWallGroupMetasTask>(token);
    if(task)
    {
        grpMeta = task->mResultData;
        _markTaskForDeletion(token);
    }
}

void p3WallService::requestWallGroups(uint32_t &token, const RsGxsId &identity)
{
    _startTask(token, new SearchWallGroupDataTask(identity));
}

void p3WallService::getWallGroups(const uint32_t &token, std::vector<WallGroup> &wgs)
{
    SearchWallGroupDataTask* task = _getTask<SearchWallGroupDataTask>(token);
    if(task)
    {
        wgs = task->mResultData;
        _markTaskForDeletion(token);
    }
}

void p3WallService::getPostGroup(const uint32_t &token, PostGroup &pg)
{
    std::vector<PostGroupItem*> grpItems;
    getGroupDataT<PostGroupItem>(token, grpItems);
    // there should be only one resulting item, but better loop to delete them all
    // todo: it would be better to allow getting many grp at once
    for(std::vector<PostGroupItem*>::iterator it = grpItems.begin(); it != grpItems.end(); it++)
    {
        PostGroupItem* item = *it;
        pg = item->mPostGroup;
        pg.mMeta = item->meta;
        delete item;
    }
}

void p3WallService::getPostReferenceMsgs(const uint32_t &token, std::vector<ReferenceMsg> &refMsgs)
{
    std::map<RsGxsGroupId, std::vector<ReferenceMsgItem*> > msgMap;
    getMsgDataT<ReferenceMsgItem>(token, msgMap);
    std::map<RsGxsGroupId, std::vector<ReferenceMsgItem*> >::iterator mit;
    for(mit = msgMap.begin(); mit != msgMap.end(); mit++)
    {
        std::vector<ReferenceMsgItem*>& msgVec = mit->second;
        std::vector<ReferenceMsgItem*>::iterator vit;
        for(vit = msgVec.begin(); vit != msgVec.end(); vit++)
        {
            ReferenceMsgItem* item = *vit;
            item->mReferenceMsg.mMeta = item->meta;
            refMsgs.push_back(item->mReferenceMsg);
            delete item;
        }
    }
}

void p3WallService::getPostMsg(const uint32_t &token, PostMsg &pm)
{
    // todo: allow get many msgs at once
    // are many msgs neede here?
    // i think not, because there is only one msg PotsMsg in the group
    // this is nearly a copy of the fn above
    std::map<RsGxsGroupId, std::vector<PostMsgItem*> > msgMap;
    getMsgDataT<PostMsgItem>(token, msgMap);
    // again there should be only one item, else something went wrong
    std::map<RsGxsGroupId, std::vector<PostMsgItem*> >::iterator mit;
    for(mit = msgMap.begin(); mit != msgMap.end(); mit++)
    {
        std::vector<PostMsgItem*>& msgVec = mit->second;
        std::vector<PostMsgItem*>::iterator vit;
        for(vit = msgVec.begin(); vit != msgVec.end(); vit++)
        {
            PostMsgItem* item = *vit;
            pm = item->mPostMsg;
            pm.mMeta = item->meta;
            delete item;
        }
    }
}

void p3WallService::requestAvatarImage(uint32_t &token, const RsGxsId &identity)
{
    // todo
}

bool p3WallService::getAvatarImage(const uint32_t &token, WallImage &image)
{
    // maybe want to cache the avatar image, because:
    // - it will be requested often (if post many posts of the same author appear)
    // - it is expensive to extract
    // - it is small, i estimate 1-2kb
    // (can use rsmemcache.h)
    return false;
}

bool p3WallService::isAuthorSubscribed(RsGxsId& id, bool& subscribed)
{
    return shouldSubscribeTo(id, subscribed);
}

void p3WallService::subscribeToAuthor(RsGxsId &id, bool subscribe)
{
    if(id.isNull()){
        return;
    }

    RsStackMutex stack(authorsMtx);

    bool found = false;
    std::vector<RsGxsId>::iterator vit;
    for(vit = subscribedAuthors.begin(); vit != subscribedAuthors.end(); vit++)
    {
        if(*vit == id)
        {
            found = true;
        }
    }
    if((!found) && subscribe)
    {
        std::cerr << "p3WallService::subscribeToAuthor() subscribe author-id=" << id.toStdString() << std::endl;
        //subscribe
        subscribedAuthors.push_back(id);
        // trigger search for all wall-grps with author id and subscirbe to them
        // what if one subscribe task is running while the user unsubscribes from the same author?
        uint32_t token;
        _startTask(token, new SubscribeAllWallGrpsFromAuthorTask(id));
    }
    if(found && (!subscribe))
    {
        std::cerr << "p3WallService::subscribeToAuthor() unsubscribe author-id=" << id.toStdString() << std::endl;
        // unsubscribe
        // have to do some sort of garbage collection for unreachable post-grps
        // maybe mark all post-grps
        // then load all reference-msgs, and remove the mark from the referenced grps
        // then delete the marked post-grps
        // not sure if garbage collection is needed because disk space is to cheap
        subscribedAuthors.erase(std::find(subscribedAuthors.begin(), subscribedAuthors.end(),id));
        uint32_t token;
        _startTask(token, new SubscribeAllWallGrpsFromAuthorTask(id, false));
    }
}

bool p3WallService::shouldSubscribeTo(const RsGxsId &id, bool& subscribe)
{
    RsStackMutex stack(authorsMtx);

    if(authorsLoaded)
    {
        subscribe = false;
        std::vector<RsGxsId>::iterator vit;
        for(vit = subscribedAuthors.begin(); vit != subscribedAuthors.end(); vit++)
        {
            if(*vit == id)
            {
                subscribe = true;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void p3WallService::setAuthorsLoaded()
{
    RsStackMutex stack(authorsMtx);
    authorsLoaded = true;
}

bool p3WallService::areAuthorsLoaded()
{
    RsStackMutex stack(authorsMtx);
    return authorsLoaded;
}

void p3WallService::_checkSubscribeGrpChange(const RsGxsGroupId& grpId, RsGxsNotify::NotifyType changeType)
{
    if(changeType == RsGxsNotify::TYPE_RECEIVE)
    {
        _startTask(new ProcessGrpTask(grpId));
    }
}

void p3WallService::_checkSubscribeMsgChange(const RsGxsGrpMsgIdPair &grpMsgId, RsGxsNotify::NotifyType changeType)
{
    if(changeType == RsGxsNotify::TYPE_RECEIVE)
    {
        _startTask(new ProcessMsgTask(grpMsgId));
    }
}

// current activity collection system is very simple:
// - read msgs
// - make a list to keep track who interacted whith which post
void p3WallService::_filterActivitiesGrpChange(const RsGxsGroupId& grpId, RsGxsNotify::NotifyType changeType)
{
    _startTask(new FilterActivitiesGrpChangeTask(grpId));
}

void p3WallService::_filterActivitiesMsgChange(const RsGxsGrpMsgIdPair &grpMsgId, RsGxsNotify::NotifyType changeType)
{
    _startTask(new FilterActivitiesMsgChangeTask(grpMsgId));
}

void p3WallService::_filterNews(std::vector<RsGxsNotify*> &changes)
{
    //
}

void p3WallService::_startTask(WallServiceTask *newTask)
{
    uint32_t token;
    _startTask(token, newTask);
}

void p3WallService::_startTask(uint32_t &token, WallServiceTask *newTask)
{
    if(newTask == NULL)
    {
        std::cerr << "p3WallService::_startTask() Error: wrong usage. Parameter newTask must not be NULL." << std::endl;
    }
    token = generatePublicToken();
    newTask->mPublicToken = token;
    newTask->mWallService = this;
    {
        RsStackMutex stack(_mTaskMtx);
        if(_mTasks.find(token) != _mTasks.end())
        {
            std::cerr << "p3WallService::_startTask() FATAL ERROR: the same token is alread stored in the map."
                         " This should never ever happen." << std::endl;
            delete newTask;
            return;
        }
        _mTasks[token] = newTask;
    }
}

void p3WallService::_doTasks()
{
    // want to have task->doWork() outside of the lock
    //   this allows the tasks to call all functions of p3WallService except _doTasks()
    // example: the tasks can use public functions to create new tasks

    // copy all running tasks into a vector to allow working outside the mutex lock
    std::vector<WallServiceTask*> tasks;
    {
        RsStackMutex stack(_mTaskMtx);
        tasks.reserve(_mTasks.size());
        std::map<uint32_t, WallServiceTask* >::iterator mit;
        for(mit = _mTasks.begin(); mit != _mTasks.end(); mit++)
        {
            tasks.push_back(mit->second);
        }
    }
    std::vector<uint32_t> tasksToDelete;
    for(std::vector<WallServiceTask*>::iterator vit = tasks.begin(); vit != tasks.end(); vit++)
    {
        WallServiceTask* task = *vit;
        switch(task->mResult){
        case WallServiceTask::NO_RESULT_YET:
        {
            bool canWork = true;
            bool allTokensFailed = true;

            // maybe later: check timestamp for next tick

            // check the tokens the task is waiting for
            std::vector<uint32_t>::iterator it;
            for(it = task->mPendingTokens.begin(); it != task->mPendingTokens.end(); it++)
            {
                uint32_t tokenStatus = RsGenExchange::getTokenService()->requestStatus(*it);
                if(tokenStatus != RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE)
                {
                    canWork = false;
                }
                if(tokenStatus != RsTokenService::GXS_REQUEST_V2_STATUS_FAILED)
                {
                    allTokensFailed = false;
                }
            }
            // if there are no tokens, then don't delete it
            if(task->mPendingTokens.empty())
            {
                allTokensFailed = false;
            }
            if(canWork)
            {
                task->mPendingTokens.clear();
                task->doWork();
            }
            if(allTokensFailed)
            {
                updatePublicRequestStatus(task->mPublicToken, RsTokenService::GXS_REQUEST_V2_STATUS_FAILED);
                tasksToDelete.push_back(task->mPublicToken);
            }
        }
            break;
        case WallServiceTask::FAIL:
        {
            updatePublicRequestStatus(task->mPublicToken, RsTokenService::GXS_REQUEST_V2_STATUS_FAILED);
            tasksToDelete.push_back(task->mPublicToken);
        }
            break;
        case WallServiceTask::COMPLETE:
        {
            // don't remove task, because it contains the result
            // maybe remove task after timeout?
            // update public token status
            updatePublicRequestStatus(task->mPublicToken, RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE);
            // todo: think about deletion of successful tasks
        }
            break;
        }
    }
    // make a list of tasks to delete, have to do this inside the lock
    std::vector<WallServiceTask*> taskPointersToDelete;
    {
        RsStackMutex stack(_mTaskMtx);
        // pull in marked tasks from other functions
        tasksToDelete.reserve(tasksToDelete.size() + _mTaskToDelete.size());
        for(std::vector<uint32_t>::iterator vit = _mTaskToDelete.begin(); vit != _mTaskToDelete.end(); vit++)
        {
            tasksToDelete.push_back(*vit);
        }
        _mTaskToDelete.clear();
        // remove them from the map
        // save pointers for later delete
        taskPointersToDelete.reserve(tasksToDelete.size());
        for(std::vector<uint32_t>::iterator vit = tasksToDelete.begin(); vit != tasksToDelete.end(); vit++)
        {
            std::map<uint32_t, WallServiceTask* >::iterator mit = _mTasks.find(*vit);
            if(mit != _mTasks.end())
            {
                taskPointersToDelete.push_back(mit->second);
                _mTasks.erase(mit);
            }
            else
            {
                std::cerr << "p3WallService::_doTasks() Error: task to delete is not in map. This should not happen." << std::endl;
            }
        }
    }
    // delete the tasks outside the lock
    std::vector<WallServiceTask*>::iterator vit;
    for(vit = taskPointersToDelete.begin(); vit != taskPointersToDelete.end(); vit++)
    {
        if(*vit != NULL)
        {
            delete *vit;
        }
        else
        {
            std::cerr << "p3WallService::_doTasks() Error: task to delete is NULL. This can't happen and should never happen." << std::endl;
        }
    }
}

void p3WallService::_markTaskForDeletion(const uint32_t &token)
{
    RsStackMutex stack(_mTaskMtx);
    _mTaskToDelete.push_back(token);
}

}//namespace RsWall
