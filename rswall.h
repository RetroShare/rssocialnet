#pragma once

#include <retroshare/rsgxsifacehelper.h>

// next steps
//  ui items for post and replies
//  make listmodel for newsfeed and wall (will have similar model with different parameters)
//  gen identity ui
//  list identity ui, maybe make this first, because backends is there

// things to do on first start:
//  create id
//  create circle for friends with author id set
//    how can we know if a circle is a friendslist or just a circle?
//    can't know this, so have to keep circle id somewhere
//    which name should the circle have?
//    maybe need some hacks in the circle system?
//      can abuse RsGroupMetaData.mServiceString to have a flag which shows "this is a friendslist"
//      better don't make hack.
//      better detect by circle group meta if this can be a friendslist
//      it can be a friendslist, if author is set. can be own friendslist if own autho id set
//        on friends wall, filter circles by friends author id
//      what if A makes a cricle named "friends of B", everyone will believe it is friends of B
//        display circles by athor id like: author -> circle name

// from which service can i steal items and serialiser?
// gxscommentitems look useful
// p3gxscommentservcie has this complicated rating formula
// formula doesn not handle relations like friendship and follow
// how to request comments: GxsCommentTreeWidget::service_requestComments
// want to rate root posts only, display others ordered by time

// want avatars later, how to do this?
// maybe store it with the profile in a author-only group


// about rsitems:
// can't change these later, so design them carefully
// check if tlv items can be extended later
// but try to include all data we want before first release
/*
WallItems

// root Posts
// group contains post itself an replys + votes
class WallPostGroup{
    RsGroupMetaData mMeta;
}
class WallPost{
    RsMsgMetaData mMeta;
    std::string mMsg;
}
class WallVote{
    RsMsgMetaData mMeta;
    uint8_t mVote;
}

// group contains grpIds of grps where we contributed to
class WallPointerGroup{
    RsGroupMetaData mMeta;
}
class WallPointer{
    RsMsgMetaData mMeta;
    uint8_t mType;

    const uint8_t WPTR_TYPE_COMMENT = 0;
    const uint8_t WPTR_TYPE_VOTE_UP = 1;    // want to know if up or downvote
    const uint8_t WPTR_TYPE_VOTE_DOWN = 2;  // don't want to subscibe to downvoted groups
}

class WallProfileGroup{
    RsGroupMetaData mMeta;
}
class WallProfileEntry{
    // want to have profile data splitted into different msgs?
    // this would allow cheap updating of single entrys
    //   this depends on size of msg meta
    //   msg meta has:
    //   4x msg id with 20 bytes
    //   1x grp id with 16 bytes
    //   1x author id with 16 bytes
    //   1x signatures, dont know how large
    //   probably is the textual profile data not so big, so put all text in one item
    //   pictures are big, so one pic per entry item

    // more important: what should be on profile page?
    // look at other social networks
}

*/

class RsWallMsg{
public:
    RsMsgMetaData mMeta;
    std::string mMsg;
};

class RsWallGroup{
public:
    RsGroupMetaData mMeta;
};

/*
message and group changes propagation:

rsgenexchange calls
virtual void notifyChanges(std::vector<RsGxsNotify*>& changes) = 0;

have to implement this in own rsgenexchange derived class
then call
RsGxsIfaceHelper::receiveChanges(changes)

which then calls

void RsGenExchange::receiveChanges(std::vector<RsGxsNotify*>& changes)

why does this go through all layers and not directly?

rsgenexchange makes a list of this changes, and the it can be retrieved from gxsifacehelper, whichs calls genexchange

*/

class RsWall: public RsGxsIfaceHelper{
public:
    virtual bool getGroupData(const uint32_t &token, std::vector<RsWallGroup> &groups) = 0;
    virtual bool getMsgData(const uint32_t &token, std::vector<RsWallMsg> &msgs) = 0;

    virtual void setMessageReadStatus(uint32_t& token, const RsGxsGrpMsgIdPair& msgId, bool read) = 0;

    virtual bool createGroup(uint32_t &token, RsGxsForumGroup &group) = 0;
    virtual bool createMsg(uint32_t &token, RsGxsForumMsg &msg) = 0;

    // need to handle new msgs in groups
    // need to handle new groups, and subscribe if required
    // ui needs to receive new groups

    // ui displays news in a list
    // how to know size of list?
    // want to use dynamic loading of data from listmodel
    // maintain a list of group ids
    // need to sort groups? to make interestig posts first?
    // idea: have a list of all subscribed group ids, have to do this anyways
    // then rank the ids by:
    // last post
    // how many friends commented
    // likes of friends
};
