#pragma once

#include <gxs/rsgenexchange.h>
#include <gxs/gxstokenqueue.h>
#include <services/p3gxscommon.h>
#include "rswall.h"
#include "rswallitems.h"

#include <typeinfo>

/*
there will be many different background tasks/processes to work on
idea:
create a process interface like this:
class Process
{
    virtual void doWork();
    virtual bool isDone(); // have to keep the process in list at this point because he holds the result

    std::vector<uint32_t> mTokenSensitivityList; // tokens the Process is waiting for
}
the service would then tick all the processes
this way i just have to maintain one list
should the Process class check for tokens?
  the process has to check for tokens anyway, because it needs to know if the status is complete or fail
or should the service check for the tokens the Process is waiting for
optionally it would be very easy to set timeouts on Processes
if the services checks for tokens, it would be similar to TokenQueue

// it could look like this
// it is much easier to read than the dynamic_cast<>(), switch() call_function() seen at other gxs places
// have everything about one task at one place
// code + data
class ExampleProcess: public Process
{
    ExampleProcess(uint8_t param): mState(BEGIN), someVar(param) {}
    State mState;
    uint8_t someVar;
    virtual void doWork(bool noError){ // idea: signal error condition(=failed token) with a bool
        if(noError)
        {
            switch(mState){
            case BEGIN:
                uint32_t token;
                request_something(token);
                mState = TWO;
                waitForToken(token); // this fn does not really wait,
                                     // it just tells the service for what we are waiting
                // could also do something like this:
                // waitms(1000); // tell the service that we want the next tick in 1000ms
                break;
            case TWO:
                ...
            }
        }else{ // error occured
            // cleanup
            finishProcess();
        }
    }
}
*/

// begin old code
/*
class PostMsgTask{
public:
    enum State { WAITING_GRP, WAITING_MSG, WAITING_REFERENCE, COMPLETED};
    State state;
    // this Token was send to the client
    uint32_t publicToken;
    // these are own tokens we got from the core
    uint32_t groupToken;// create group
    uint32_t msgToken;// create msg in group
    // todo
    // uint32_t referenceToken; // create reference on own wall
    PostMsg pm;
};
*/
// end old code

/*
processing of messages
the wallservice has to read grps and msgs to learn which grps should be subscribed
gxs has flags in grp/msg meta to signal unprocessed/processed
but requesting only unprocessed grps/msgs is expensive because gxs loads all msgs and removes the uninteresting ones
  i wonder how fast dataaccess/sqllite can load the metadata
maybe request the unprocessed grps/msgs only at start
  (in case retroshare crashed and could not process all incoming messages)
  will this result in heavy disk load at start of rs? heavy disk load is bad
    maybe can optimize dataaccess later
later process only the grps/msgs from notify
if the grp/msg could not be processed immediately, put id into a queue and don't mark as processed

how many metas fit into the ram?
assuming grp meta is similar to msg meta which is about 500bytes serialized
for meta of 1kb and allowed memory consumtion of 100MB: 100*10^6 / 10^3 = 100*10^3 metas
this is 300*300, so can download 300grps/day for one year
this does not last forever, but long enaugh for first
as first memory optimisation could request only parts of the metas and filter them

dataaccess can't filter for group status?
see rsgxsdataaccess:1611

is there a way to create a service-specific index for grps/msgs?
for first linear search is ok, but once there are to many grps/msgs it won't work
think about this later
*/

class WallServiceTask;

// have two threads: ui thread, rsgenexchange thread
// have to be careful when they cross their ways
class p3WallService: public RsWall, public RsGenExchange, public GxsTokenQueue{
public:
    p3WallService(RsGeneralDataService* gds, RsNetworkExchangeService* nes, RsGixs* gixs);
    virtual ~p3WallService();

    // from RsGenExchange
    virtual RsServiceInfo getServiceInfo();

    static uint32_t wallAuthPolicy();

    // from RsGenExchange
    // this is called by RsGenExchange to signal group/msg changes
    // handle auto subscribe here
    // can get group meta from RsDataService::retrieveGxsGrpMetaData
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes);

    // from RsGenExchange
    // this is called by RsGenExchange runner thread
    virtual void service_tick();

    // from GxsTokenQueue
    virtual void handleResponse(uint32_t token, uint32_t req_type);

    // *********** begin RsWall ***********
    // wall iface here
    // ********** interface *************************
        // important: newsfeed handling
        // the ui has to ask if the newsfeed has new entries
        //   what is NewsfeedEntry? probably a pair (RsGxsGroupId,RsGxsMsgId)
        //   RsGxsMsgId can be null to signal that only the RsGxsGroupId is important for the newsfeed
        //     have to provide type info here, so the ui knows which fn to use to retrieve data
        virtual void getNewNewsfeedEntries(std::list<NewsfeedEntry> &feeds);
        // the Retroshare newsfeed has no state across start/stop of retroshare
        // do we need to save a newsfeed state to disk?

        virtual void createWallGroup(uint32_t &token, const WallGroup &grp);
        virtual void updateWallGroup(uint32_t &token, const WallGroup &grp);

        // how to handle selection of target wall?
        //   maybe we have different walls private/public
        // who fills in the cirlce id?
        //   probably the service, because he has time and knowledge, and can cache things
        virtual void createPost(uint32_t &token, const PostMsg &msg);
    virtual void acknowledgeCreatePost(uint32_t &token);

        // this handles sharing on own or friends wall, and like
        virtual void createPostReferenceMsg(uint32_t &token, const ReferenceMsg &refMsg);

        // use this if you only know the identity of the wall owner
        // use request in gxsifacehelper if you want to list all known walls
        // (this will be expensive, because have to request all gxs-groups)
        // (better have a flag in meta, so have to request meta only?)
        // (would be cool if gxs could filter by RsItem-type)
        // maybe should return a list of walls, because of public wall and private wall?
        //   ->merge results from both walls

        // TODO, don't use
        virtual void requestWallGroupMetas(uint32_t &token, const RsGxsId &identity); // empty id to get all wall groups
        virtual void getWallGroupMetas(const uint32_t &token, std::vector<RsGroupMetaData>& grpMeta);

        //
        virtual void requestWallGroups(uint32_t &token, const RsGxsId &identity);
        virtual void getWallGroups(const uint32_t &token, std::vector<WallGroup> &wgs);

        virtual void getPostGroup(const uint32_t &token, PostGroup &pg);
        virtual void getPostReferenceMsgs(const uint32_t &token, std::vector<ReferenceMsg> &refMsgs);
        virtual void getPostMsg(const uint32_t &token, PostMsg &pm);

        virtual void requestAvatarImage(uint32_t &token, const RsGxsId &identity);
        virtual bool getAvatarImage(const uint32_t &token, Image &image);

        // functions for comment service
        // (just a forward to p3GxsCommentService like in p3GxsChannels)

    // ************ end RsWall ************

    // this code is copied from p3GxsChannels
    /* Comment service - Provide RsGxsCommentService - redirect to p3GxsCommentService */
virtual bool getCommentData(const uint32_t &token, std::vector<RsGxsComment> &msgs)
    {
            return mCommentService->getGxsCommentData(token, msgs);
    }

virtual bool getRelatedComments(const uint32_t &token, std::vector<RsGxsComment> &msgs)
    {
        return mCommentService->getGxsRelatedComments(token, msgs);
    }

virtual bool createComment(uint32_t &token, RsGxsComment &msg)
    {
        return mCommentService->createGxsComment(token, msg);
    }

virtual bool createVote(uint32_t &token, RsGxsVote &msg)
    {
        return mCommentService->createGxsVote(token, msg);
    }

virtual bool acknowledgeComment(const uint32_t& token, std::pair<RsGxsGroupId, RsGxsMessageId>& msgId)
    {
        return acknowledgeMsg(token, msgId);
    }


virtual bool acknowledgeVote(const uint32_t& token, std::pair<RsGxsGroupId, RsGxsMessageId>& msgId)
    {
        if (mCommentService->acknowledgeVote(token, msgId))
        {
            return true;
        }
        return acknowledgeMsg(token, msgId);
    }



    // give tasks access to protected RsGenExchange::publishGroup()
    void publishGroup(uint32_t &token, RsGxsGrpItem *grpItem)
    {
        RsGenExchange::publishGroup(token, grpItem);
    }
    template<class GrpType> bool getGroupDataT(const uint32_t token, std::vector<GrpType*> &grpItem)
    {
        return RsGenExchange::getGroupDataT(token, grpItem);
    }

private:
    // check if we want to subscribe to incoming groups
    // detect interesting group-ids in messages
    virtual void _checkSubscribe(std::vector<RsGxsNotify*>& changes);

    // set to collect wanted groups
    std::set<RsGxsGroupId> _mGroupsToSubscribe;

    // collect info about incoming groups and messages
    // generate newsfeed entries from it
    // first this will just record interesting things by by timestamp
    // later we maybe want a filter with for example 5 mins delay
    // and then rate all messages in this period and filter some out
    // lets see how much content a user receives,
    // if its to much think about filters
    // the user probably wants to tune the filter
    virtual void _filterNews(std::vector<RsGxsNotify*>& changes);

    // don't use these variables directly
    // use the functions below, becaus they provide proper locking
    RsMutex _mTaskMtx;
    std::map<uint32_t, WallServiceTask*> _mTasks;
    std::vector<uint32_t> _mTaskToDelete;
    void _startTask(uint32_t &token, WallServiceTask* newTask);
    void _doTasks();
    void _markTaskForDeletion(const uint32_t& token);
    template<class T> T* _getTask(const uint32_t& token)
    {
        RsStackMutex stack(_mTaskMtx);
        // should make double check if the token is ready?
        std::map<uint32_t, WallServiceTask*>::iterator mit = _mTasks.find(token);
        if(mit == _mTasks.end())
        {
            std::cerr << "p3WallService::_getTask<" << typeid(T).name() << ">() Error: token not found in map." << std::endl;
            return NULL;
        }
        WallServiceTask* p1 = mit->second;
        T* p2 = dynamic_cast<T*>(p1);
        if(p2 == NULL)
        {
            std::cerr << "p3WallService::_getTask<" << typeid(T).name() << ">() Error: task has wrong type." << std::endl;
            return NULL;
        }
        // should check if the task itself says it is ready?
        return p2;
    }

    // begin old code
    /*
    std::list<PostMsgTask> _mPostMsgTasks;
    RsMutex _mPostTaskMtx;
    virtual void _processPostMsgTasks();
    */
    // end old code

    p3GxsCommentService *mCommentService;
};
