#pragma once

#include <retroshare/rsidentity.h>

namespace SonetUtil{

// make a string like "SomeName [247238][anon]"
std::string formatGxsId(const RsIdentityDetails& details);

}
