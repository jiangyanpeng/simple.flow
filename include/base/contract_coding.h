#ifndef SIMPLE_FLOW_CONTRACT_CODING_H_
#define SIMPLE_FLOW_CONTRACT_CODING_H_

#include <cassert>

#ifdef DEBUG
#define SIMPLE_ASSERT(cond) assert(cond)
#else
#define SIMPLE_ASSERT(cond)
#endif


#endif // SIMPLE_FLOW_CONTRACT_CODING_H_
