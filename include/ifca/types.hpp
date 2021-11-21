#ifndef TYPES_H
#define TYPES_H

#include <atomic>
#include <future>
#include <list>
#include <string>

namespace ifca {

using chunk_intype = std::string;

using drain_sfuture = std::shared_future<void>;

using chunk_outtype = std::string;

using chunk_promise = std::promise<chunk_outtype>;
using chunk_future = std::future<chunk_outtype>;
using chunk_sfuture = std::shared_future<chunk_outtype>;

using transform_type = std::function<chunk_outtype(chunk_intype)>;

}  // namespace ifca

#endif  // TYPES_H
