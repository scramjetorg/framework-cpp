#include <future>
#include <ifca/ifca.h>

using namespace std; 

namespace ifca {

  template <typename T, typename S> IFCA<T,S>::IFCA(int _maxParallel, ifca::operation<T,S>* op)
    : m_maxParallel{_maxParallel}, f_op{op}
  {};

}
