#include "SonetUtil.h"

std::string SonetUtil::formatGxsId(const RsIdentityDetails &detail)
{
    std::string anon;
    if(detail.mPgpLinked){
        anon = "[pgp]";
    } else {
        anon = "[anon]";
    }
    return detail.mNickname + "[" + detail.mId.toStdString().substr(0, 5) + "]" + anon;
}
