#include "uuid.hpp"

namespace Vengine {

uint64_t UUID::m_nextId = 1;
std::list<uint64_t> UUID::m_freeIds = {};

}  // namespace Vengine
