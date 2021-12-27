// #include <doctest/doctest.h>

// // #include <functional>
// #include <iostream>
// #include <iterator>
// #include <string>
// // #include <type_traits>
// // #include <typeinfo>

// #include "helpers/FWD.hpp"
// #include "helpers/Logger/logger.hpp"
// #include "helpers/crtp.hpp"
// #include "ifca/transform/each.hpp"
// #include "ifca/transform/filter.hpp"
// #include "ifca/transform/into.hpp"
// #include "ifca/transform/orderedExpression.hpp"
// #include "ifca/transform/outStream.hpp"
// #include "ifca/transform/transformChain.hpp"
// #include "ifca/transform/transformExpression.hpp"

// namespace ifca {

// template <typename T>
// struct TestClass {};

// TEST_CASE("Transform") {
//   std::function f1 = [](int& chunk) {
//     LOG_DEBUG() << "function1 call " << chunk << "\n";
//     return chunk * 2;
//   };
//   std::function f2 = [](int& chunk) {
//     LOG_DEBUG() << "function2 call " << chunk << "\n";
//     return chunk + 2;
//   };
//   std::function f3 = [](int& chunk) {
//     LOG_DEBUG() << "function3 call " << chunk << "\n";
//     return chunk - 2;
//   };
//   std::function fil1 = [](int& chunk) {
//     LOG_DEBUG() << "filter call " << chunk << "\n";
//     return chunk % 2;
//   };
//   std::function into = [](int& chunk) {
//     LOG_DEBUG() << "into call " << chunk << "\n";
//   };
//   auto&& filter = Filter(fil1);
//   auto&& t1 = Each(f1);
//   // auto&& t2 = Each(f2);
//   // auto&& t3 = Each(f3);
//   auto&& t4 = OutStream(std::cout);
//   // auto&& in = Into(into);
//   // int&& result = 0;
//   // auto&& res = FwdResultTransform<int>(result);

//   // auto c2 = filter >>= t1 >>= t2 >>= t3 >>= t4;
//   // auto c2 = filter >>= t1 >>= t2 >>= t3;
//   // auto a = TransformChain<decltype(t1), decltype(filter)>(FWD(t1),
//   // FWD(filter)); a >>= t4; auto c2 = t1 >>= filter >>= t4;
//   auto a = t1 >>= filter >>= t4;
//   a >>= t4;
//   // auto c2 = t1 >>= filter >>= t4;
//   // c2(10);
//   // c2(11);
//   // auto res = 5;
//   // res << c2(11);

//   // std::cout << std::boolalpha << "\nis_crtp_interface_of: \n"
//   //           << is_crtp_interface_of<TransformExpression,
//   //                                   IntoTransform<decltype(in)>>::value
//   //           << "\n"
//   //           << is_crtp_interface_of<OrderedExpression,
//   //                                   IntoTransform<decltype(in)>>::value
//   //           << "\n"
//   //           << is_crtp_interface_of<TransformExpression,
//   //                                   EachTransform<decltype(t1)>>::value
//   //           << "\n"
//   //           << is_crtp_interface_of<OrderedExpression,
//   //                                   EachTransform<decltype(t1)>>::value
//   //           << "\n"
//   //           << is_crtp_interface_of<TransformExpression,
//   //           TestClass<void>>::value
//   //           << "\n"
//   //           << is_crtp_interface_of<OrderedExpression,
//   //           TestClass<void>>::value
//   //           << "\n";
// }

// }  // namespace ifca