#include "PeersHandler.h"

#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <util/radix64.h>

#include <algorithm>

#include "Operators.h"
#include "ApiTypes.h"

namespace resource_api
{

// todo: groups, add friend, remove friend, permissions

void peerDetailsToStream(StreamBase& stream, RsPeerDetails& details)
{
    stream
        << makeKeyValueReference("peer_id", details.id)
        << makeKeyValueReference("name", details.name)
        << makeKeyValueReference("location", details.location)
        << makeKeyValueReference("pgp_id", details.gpg_id)
            ;
}

bool peerInfoToStream(StreamBase& stream, RsPeerId id, RsPeers* peers, std::list<RsGroupInfo>& grpInfo)
{
    bool ok = true;
    RsPeerDetails details;
    ok &= peers->getPeerDetails(id, details);
    peerDetailsToStream(stream, details);
    stream << makeKeyValue("is_online", peers->isOnline(id));

    std::string avatar_address = "/"+id.toStdString()+"/avatar_image";
    stream << makeKeyValue("avatar_address", avatar_address);

    StreamBase& grpStream = stream.getStreamToMember("groups");

    for(std::list<RsGroupInfo>::iterator lit = grpInfo.begin(); lit != grpInfo.end(); lit++)
    {
        RsGroupInfo& grp = *lit;
        if(std::find(grp.peerIds.begin(), grp.peerIds.end(), details.gpg_id) != grp.peerIds.end())
        {
            grpStream.getStreamToMember()
                    << makeKeyValueReference("group_name", grp.name)
                    << makeKeyValueReference("group_id", grp.id);
        }
    }

    return ok;
}

PeersHandler::PeersHandler(RsPeers *peers, RsMsgs* msgs):
    mRsPeers(peers), mRsMsgs(msgs)
{
    addResourceHandler("*", this, &PeersHandler::handleWildcard);
    addResourceHandler("examine_cert", this, &PeersHandler::handleExamineCert);
}

std::string PeersHandler::help()
{
    std::string help =
            "GET /\n"
            "returns a list of objects with the following members:\n"
            "peer_id:   string\n"
            "name:      string\n"
            "location:  string\n"
            "pgp_id:    string\n"
            "is_online: bool\n"
            "avatar_address: string with an address where to get the avatar image\n"
            "groups: array of objects, objects have two string members: group_name and group_id\n"
            "\n"
            "POST /examine_cert\n"
            "<cert string>\n"
            "returns an object with the following string members: peer_id, name, location, pgp_id\n"
            "\n"
            "DELETE /<peer_id>\n"
            "remove friend from friendslist\n"
            "\n"
            "POST /\n"
            "<cert string>\n"
            "adds a friend to keyring and friendslist\n"
    ;
    return help;
}

void PeersHandler::handleWildcard(Request &req, Response &resp)
{
    bool ok = false;
    if(!req.mPath.empty())
    {
        std::string str = req.mPath.top();
        req.mPath.pop();
        if(str != "")
        {
            // assume the path element is a peer id
            // sometimes it is a peer id for location info
            // another time it is a pgp id
            // this will confuse the client developer
            if(!req.mPath.empty() && req.mPath.top() == "avatar_image")
            {
                // the avatar image
                // better have this extra, else have to load all avatar images
                // only to see who is online
                unsigned char *data = NULL ;
                int size = 0 ;
                mRsMsgs->getAvatarData(RsPeerId(str),data,size) ;
                std::vector<uint8_t> avatar(data, data+size);
                delete[] data;
                resp.mStream << avatar;
            }
            else if(req.mMethod == Request::DELETE_AA)
            {
                mRsPeers->removeFriend(RsPgpId(str));
            }
            else
            {
                std::list<RsGroupInfo> grpInfo;
                mRsPeers->getGroupInfoList(grpInfo);
                ok = peerInfoToStream(resp.mStream, RsPeerId(str), mRsPeers, grpInfo);
            }
        }
    }
    else
    {
        // no more path element
        if(req.mMethod == Request::GET)
        {
            // list all peers
            ok = true;
            std::list<RsPeerId> peers;
            ok &= mRsPeers->getFriendList(peers);
            std::list<RsGroupInfo> grpInfo;
            mRsPeers->getGroupInfoList(grpInfo);
            for(std::list<RsPeerId>::iterator lit = peers.begin(); lit != peers.end(); lit++)
            {
                peerInfoToStream(resp.mStream.getStreamToMember(),*lit, mRsPeers, grpInfo);
            }
        }
        else if(req.mMethod == Request::POST)
        {
            std::vector<uint8_t> bytes;
            req.mStream << bytes;
            std::string certString(bytes.begin(), bytes.end());
            RsPeerId peer_id;
            RsPgpId pgp_id;
            std::string error_string;
            mRsPeers->loadCertificateFromString(certString,peer_id, pgp_id, error_string);
            mRsPeers->addFriend(peer_id, pgp_id);
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

void PeersHandler::handleExamineCert(Request &req, Response &resp)
{
    std::vector<uint8_t> bytes;
    req.mStream << bytes;
    std::string certString(bytes.begin(), bytes.end());

    bool ok = false;
    RsPeerDetails details;
    uint32_t error_code;
    if(mRsPeers->loadDetailsFromStringCert(certString, details, error_code))
    {
        peerDetailsToStream(resp.mStream, details);
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

} // namespace resource_api
