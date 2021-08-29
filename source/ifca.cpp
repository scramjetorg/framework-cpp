#include <ifca/ifca.h>
#include <ifca/operation.h>

#include <future>

using namespace std;

namespace ifca {
    template <typename T, typename S, typename I>
    inline IFCA<T, S, I>::IFCA(int maxParallel, typename operation<T, S>::synchronous op)
        : n_maxParallel{maxParallel} {}

}  // namespace ifca
