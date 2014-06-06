#include "p3wallservice.h"

RsWall *rsWall = NULL;

const uint32_t WALL_MSG_STORE_PERIOD = 30*24*60*60; // in seconds

p3WallService::p3WallService(RsGeneralDataService *gds, RsNetworkExchangeService *nes, RsGixs *gixs):
    RsWall(this),
    // RS_SERVICE_TYPE_WALL looks redundant, because the serialiser already knows this
    RsGenExchange(gds, nes, new WallSerialiser(), RS_SERVICE_TYPE_WALL, gixs, wallAuthPolicy(), WALL_MSG_STORE_PERIOD),
    GxsTokenQueue(this),
    _mPostTaskMtx("p3WallService _mPostTaskMtx"),
    mCommentService(new p3GxsCommentService(this, RS_SERVICE_TYPE_WALL))
{

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
    info.mServiceType = RS_SERVICE_TYPE_WALL;
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

    // forward the changes
    RsGxsIfaceHelper::receiveChanges(changes);
}

// called by RsGenExchange
void p3WallService::service_tick()
{
    //std::cerr << "p3WallService::service_tick()" << std::endl;
    mCommentService->comment_tick();
    GxsTokenQueue::checkRequests();
    _processPostMsgTasks();
}

// called by GxsTokenQueue::checkRequests()
// it gets called when a token request is ready
void p3WallService::handleResponse(uint32_t token, uint32_t req_type)
{
    // hand over data to the ui by updating token status
}

void p3WallService::getNewNewsfeedEntries(std::list<NewsfeedEntry> &feeds)
{
    // tuwas
}

void p3WallService::createWallGroup(uint32_t &token, const WallGroup &grp)
{
    // tuwas
}

void p3WallService::updateWallGroup(uint32_t &token, const WallGroup &grp)
{
    // tuwas
}

void p3WallService::createPost(uint32_t &token, const PostMsg &msg)
{
    std::cerr << "p3WallService::createPost()" << std::endl;
    std::cerr << "msg.mMeta.mAuthorId = " << msg.mMeta.mAuthorId.toStdString() << std::endl;
    std::cerr << "msg.mPostText = " << msg.mPostText << std::endl;
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
}

void p3WallService::acknowledgeCreatePost(uint32_t &token)
{
    // use this fn to dispose the token?
    // or dispose old tokens?
    // does the ui need the ide of the new grp/msg?
    //   no, the ui only wants to know ok/fail
    //   the ui will leran about the new grp/msg from notification
}

void p3WallService::createPostReferenceMsg(uint32_t &token, const ReferenceMsg &refMsg)
{
    // mehr
}

void p3WallService::requestWallGroupMetas(uint32_t &token, const RsGxsId &identity)
{
    // have to search all grp-metas for author id
    // so this is a expensive operation
    // maybe want to cache the grp-ids of the wall grps
    // std::map<RsGxsId, std::vector<RsGxsGroupId> >
}

void p3WallService::getWallGroupMetas(const uint32_t &token, std::vector<RsGroupMetaData> &grpMeta)
{
    //
}

void p3WallService::requestWallGroups(uint32_t &token, const RsGxsId &identity)
{
    //
}

void p3WallService::getWallGroups(const uint32_t &token, std::vector<WallGroup> &wgs)
{
    //
}

void p3WallService::getPostGroup(const uint32_t &token, PostGroup &pg)
{
    //
}

void p3WallService::getPostReferenceMsg(const uint32_t &token, ReferenceMsg &refMsg)
{
    //
}

void p3WallService::getPostMsg(const uint32_t &token, PostMsg &pm)
{
    //
}

void p3WallService::requestAvatarImage(uint32_t &token, const RsGxsId &identity)
{
    //
}

bool p3WallService::getAvatarImage(const uint32_t &token, Image &image)
{
    // maybe want to cache the avatar image, because:
    // - it will be requested often (if post many posts of the same author appear)
    // - it is expensive to extract
    // - it is small, i estimate 1-2kb
    // (can use rsmemcache.h)
    return false;
}

void p3WallService::_checkSubscribe(std::vector<RsGxsNotify*> &changes)
{
    //
}

void p3WallService::_filterNews(std::vector<RsGxsNotify*> &changes)
{
    //
}

void p3WallService::_processPostMsgTasks()
{
    // create all the necessary grps/msgs for a post
    // - creation of PostGroup was already requested by createPost()
    // - create PostMsg
    // - create reference on wall

    // currently there is no timeout. so things could go wrong in a way where the user can not create posts anymore
    RsStackMutex stack(_mPostTaskMtx);
    bool removeTask = false;
    if(!_mPostMsgTasks.empty()){
        // currently a fifo queue: only start a new task if the previous is done
        // process the first task in the queue
        PostMsgTask& task = _mPostMsgTasks.front();
        std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << std::endl;
        switch(task.state){
        case PostMsgTask::WAITING_GRP:
        {
            uint32_t tokenStatus = RsGenExchange::getTokenService()->requestStatus(task.groupToken);
            std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << " case PostMsgTask::WAITING_GRP: tokenStatus=" << tokenStatus << std::endl;

            // tokenStatus will always be failed or complete at the end
            if(tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_FAILED){
                updatePublicRequestStatus(task.publicToken, RsTokenService::GXS_REQUEST_V2_STATUS_FAILED);
                removeTask = true;
                std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << " create grp failed" << std::endl;
            }
            if(tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE){
                RsGxsGroupId groupId;
                acknowledgeTokenGrp(task.groupToken, groupId);
                task.pm.mMeta.mGroupId = groupId;
                PostMsgItem* msgItem = new PostMsgItem();
                msgItem->mPostMsg = task.pm;
                msgItem->meta = task.pm.mMeta;
                publishMsg(task.msgToken, msgItem);
                task.state = PostMsgTask::WAITING_MSG;
                std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << " create grp ok" << std::endl;
            }
        }
            break;
        case PostMsgTask::WAITING_MSG:
        {
            uint32_t tokenStatus = RsGenExchange::getTokenService()->requestStatus(task.msgToken);
            if(tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_FAILED){
                updatePublicRequestStatus(task.publicToken, RsTokenService::GXS_REQUEST_V2_STATUS_FAILED);
                removeTask = true;
                std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << " create msg failed" << std::endl;
            }
            if(tokenStatus == RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE){
                updatePublicRequestStatus(task.publicToken, RsTokenService::GXS_REQUEST_V2_STATUS_COMPLETE);
                removeTask = true;
                std::cerr << "p3WallService::processPostMsgTasks() token=" << task.publicToken << " create msg ok" << std::endl;
                // todo: create reference on own wall
            }
        }
            break;
        case PostMsgTask::WAITING_REFERENCE:
            break;
        case PostMsgTask::COMPLETED:
            // nothing to do
            // when to remove task from list?
            // maybe move to another list where tasks time out if they don't get acknowledged
            break;
        }
    }

    if(removeTask){
        _mPostMsgTasks.pop_front();
    }
}
