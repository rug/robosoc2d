#ifndef R2S_DEBUG_PRINT_H
#define R2S_DEBUG_PRINT_H

#include <iostream>

//#define _R2S_DEBUG  // you should not need to #define it: it's definible by CMakeLists.txt
//-On MSVC _DEBUG is already defined in debug compilings
//-On CMAKE _R2S_DEBUG is set in this project's CMakeLists.txt to be automatically #defined in debug compilings
//-if not using either CMAKE or MSVC, you should manually #define it in debug compilings, or "g++ -D _R2S_DEBUG"

#if defined(_DEBUG) || defined(_R2S_DEBUG)  
#define DEBUG_OUT(x) do { std::cout << x; } while (0)
#define DEBUG_ERR(x) do { std::cerr << x; } while (0)

template <typename T> // useful for debugging, just like in Scott Meyers' Effective Modern C++, item 4
void tellType(const T& param)
{
  std::cout << "T =   " << typeid(T).name() << "\n";
  std::cout << "param =   " << typeid(param).name() << "\n";
}

#else
#define DEBUG_OUT(x)
#define DEBUG_ERR(x)
#define tellType(x)
#endif

#endif // DEBUG_PRINT_H 