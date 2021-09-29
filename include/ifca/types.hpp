#ifndef TYPES_H
#define TYPES_H

#include <atomic>
#include <future>
#include <list>
#include <memory>
#include <string>

namespace ifca {

using chunk_intype = std::string;

using drain_promise = std::unique_ptr<std::promise<void>>;
using drain_sfuture = std::shared_future<void>;

using chunk_outtype = std::string;

// empty = processing, valid value = finished,
// exception = end (std::future_error)
using chunk_promise = std::promise<chunk_outtype>;
using chunk_future = std::future<chunk_outtype>;
using chunk_sfuture = std::shared_future<chunk_outtype>;

}  // namespace ifca

#endif  // TYPES_H
