#ifndef ARCHITECT_CONTRACT_CODING_H
#define ARCHITECT_CONTRACT_CODING_H

#include <cassert>

#ifdef DEBUG
#define MATRIX_ASSERT(cond) assert(cond)
#else
#define MATRIX_ASSERT(cond)
#endif


#endif // ARCHITECT_CONTRACT_CODING_H
