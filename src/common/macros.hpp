#ifndef VSCODE
// clang-format off
 #pragma GCC optimize("Ofast")
 #ifndef FLAME_GRAPH
 #pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx")
 #pragma GCC optimize("O3")
 #pragma GCC optimize("omit-frame-pointer")
 #pragma GCC optimize("inline")
 #pragma GCC optimize("unroll-loops")
 #define NDEBUG
 #endif
// clang-format on
#endif
