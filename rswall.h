#pragma once

#include <retroshare/rsgxsifacehelper.h>

// how to process incoming grps and msgs?
// grps
//   check if grp id is in wanted grps list, then subscribe
// msgs
//   check if referenced grp is subscribed or available
//     if available: subscribe
//     if not available: put into wanted grps list
// if a new post-grp was subscribed:
//   make newsfeed entry
//
// how to maintain newsfeed?
//   sort by time of arrival
//   want to re-sort if new comments or votes arrive?
//   make placeholder function to allow filtering of newsfeed entrys later

// problem: can't browse gxs-grp-content without subscribing to it
//   rs is missing a load-on-demand function to browse content
// but can subscribe to group and subscribe and download only the latest post-grps
// group stores 365 days * 10 referenced posts/day * 500 bytes/referenced post
//   = 1,8 megabytes
// this allows to subscribe to all interesting groups and then pick latest posts
//   this would lead to not all posts available, just the posts at the time the site was interesting
//     can live with this, if the site was really interesting neighbor users would subscribe and download all posts
// subscribe all posts if the user likes the site
// if i learn about 10 sites per day i have to download 18 megabytes per day
// assuming everyone has 400 kbit/s = 50 kbytes/s upstream
//   then have to upload for 6mins
//     this is ok, even if i have to do this 2-3 times it is ok

// dsl rates
// 1024/128
// 2048/192
// 6016/512

// if we store friendslist as a circle
// then we need to edit circles
// how can someone notice if he gets removed from a circle?
// impossible for self restricted circle, because he can't receive the circle anymore
//   idea: add a list of to-be-removed-ids to the circle
//     then first add to remove list and later remove from allowed list
//     the last thing the removed one can see is its own name on the removed list
//     this does not work, because the latest circle-info is used for permission check

// subcircles would allow to send msgs to friends + friends of friends
// future, if the complicated subcircle things is available

// plan:
// wall -> gxs-group
//   connected to a single identity, so this is a gxs-group with author
//   msgs are just group-ids of posts
//   these ids can even be cross-service ids
//     this would allow to have all sort of content on the wall, like a link to a photo album
//       but which service would handle the comments? the wall service?
//   can have different groups:
//     one for friends
//     one for the public
//   if i like something: publish it in wall grp
//   if i share something, make msg with grp-id of content
//   others can make msgs with grp-ids of content
//   subscribe by friendship(automated) or follow
//   who can post?
//   public wall: everyone
//   friends only wall: friends only. this is a implicit rule, because you have to see the gxs-grp to make posts in it
//   don't need a publish sign, because posts are signed by author which is same as grp author in own wall-grp
//   what about a profile?
//     store basic profile info in grp item to have it available without subscribing?
//       i can see comments/posts from fofs on my friends wall
//       i have the fof wall grp item from my friends, but i'm not subscribed
//       so if profile data is stored in grp, then i could see profile of fof
//
// site -> gxs-group
//   msgs contain grp-ids of posts
//   similar to wall
//   not connected to a single identity
//   a site is a public space
//   subscribe by hand
//     can't see content without subscribing
//     two subscribe levels: auto subscribe to gxs-grps which could be interesting
//     and have another user layer subscribe to let the user choose if it is interesting
//     have also to subscribe to content-grps
//     don't want to download all content just because it could be interesting -> problem
//   what about author and publish sign?
//
// root post -> gxs-group
//   gxs-group can have author, but it can't be signed by publish key of other gxs-grp
//      would need a parent grp label which gets signed by parent grps publish key
//      similar to identity system
//   so to verify if a gxs-grp as root post belongs to another grp we have check if it is referenced in this grp
//      but then this has to be a grp with publish sign
//   can also have a anon root post = anon gxs-grp
//
// gxs offers three auth/sign levels:
//   public
//   restricted
//   private     make this to require publish sign
// probably always want author sign
//
// about blocking content
//   want to remove content from wall or pages
//   can publish a signed delete msg
//   good receivers should then unsubscribe from the gxs-grps hosting the content
//     unsubscribed gxs-grps can be garbage collected at some time in the future

// notes:
// how does gxs reputation work? where does mPeerOpinion come from?
// can delete group??? how does this work? tehre is no tombstone, so the group info will we received again?
// can't delete msg
// libretroshare has a tokenqueue: gxstokenqueue
// can get a token for own use from rsgenexchange
//    cool, because this allows us to define own requests which consist of different requestst
// unused?: mSignFlags
// same #defines in rstokenservice.h and rsgxsdataaccess.cc
//

// about fb from b1
// profile page: visible for friends by default
// timeline: visible for friends, friends can post to it,
//           can post to own timeline
//           click "share" at content from others, to import into timeline
//              what i shared is public visible
// comments: can comment every root post
// like: have a personal list of likes, can like every content including comments
//       like on comments is juts for display at the comment
// fb page: one can create a page. this is a public place where you have the right to delete posts
//    others can share and like the page
//    very similar to a blog
//
// thoughts on rs: probably don't want to make a like public

// big question: how to remove/hide posts???
// own posts? posts from others?
// can block entire identities. is it possible to make a blacklist of posts with publish key?

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
//      it can be a friendslist, if author is set. can be own friendslist if own author id set
//        on friends wall, filter circles by friends author id
//      what if A makes a circle named "friends of B", everyone will believe it is friends of B
//        display circles by athor id like: author -> circle name

// from which service can i steal items and serialiser?
// gxscommentitems look useful
// p3gxscommentservice has this complicated rating formula
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
    //   1x signatures, dont know how large    measured for gxs forum post 322 bytes
    //   probably is the textual profile data not so big, so put all text in one item
    //   pictures are big, so one pic per entry item
    //   item header is 2x4=8 byte

    // makes 120 + 322 + 8 = 450 bytes

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

next level is a class which polls gxsifacehelper, where widgets register to receive updates
gxsupdatebroadcast
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
