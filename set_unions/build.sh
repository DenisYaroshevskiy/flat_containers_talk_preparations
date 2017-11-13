clang++ --std=c++14 -O2 -Werror  -Wall \
         set_unions/baseline.cc  \
         set_unions/common.cc    \
         set_unions/current.cc   \
         set_unions/linear.cc    \
         set_unions/previous.cc  \
  -I /space/flat_containers_presentation        \
  -I /space/google_benchmark/benchmark/include/ \
  -I /space/chromium/src/                       \
   /space/google_benchmark/build/src/libbenchmark.a
