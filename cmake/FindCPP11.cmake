#==============================================================================
# Information about C++ 11 support.
# Also allows enabling C++ 11 support.
#
# Variables indicating the status of various C++ 11 features.
# This covers only a small set of more critical features. Some features
# can be assumed to be present if any C++ 11 support is available.
# Assume the following are present if C++ 11 is enabled:
# - nullptr
#
# Provides cache variables which declare support for the following features (if set):
# - @c CPP11_FOUND : Any C++11 features supported?
# - @c CPP11_FLAGS : Compiler flag(s) required to build with C++11 support enabled. May be empty.
# - @c CPP11_ATOMICS : atomics library
# - @c CPP11_AUTO : auto keyword support
# - @c CPP11_CONSTEXPR : @c constexpr keyword
# - @c CPP11_DECLTYPE : @c decltype keyword
# - @c CPP11_DEFAULT : @c default keyword
# - @c CPP11_INIT_LISTS : initialiser lists
# - @c CPP11_LAMBDA : lambda expressions
# - @c CPP11_OVERRIDE_FINAL : @c override and @c final keywords
# - @c CPP11_NON_STATIC_INIT : initialisation of non-static members within the class declaration 
# - @c CPP11_RANGE_BASED_FOR : ranged based for loops
# - @c CPP11_RVALUES : r-value references
# - @c CPP11_STATIC_ASSERT : @c static_assert support
# - @c CPP11_THREADS : threads library, also uses chrono
# - @c CPP11_THREAD_LOCAL : @c thread_local keyword
# - @c CPP11_VARIADIC_TEMPLATES : variadic templtes
#==============================================================================

include(CMakeParseArguments)

#-------------------------------------------------------------------------
# Try and compile CODE with CPP11 flags enabled.
#
# The CPP11_FLAGS must already be resolved - using resolve_cpp11_flag() -
# to support CODE.
#
# Otherwise, this macro has no special C++11 semantics, but writes
# CODE to FILENAME in a temporary directory and attempts to compile.
#
# Success or failure is indicated by the value of VAR after the attempt.
# It is TRUE on success, FALSE on failure.
#-------------------------------------------------------------------------
macro(test_cpp11_code VAR FILENAME CODE)
  CMAKE_PARSE_ARGUMENTS(CXT "" "FLAGS;MESSAGE" "" "${ARGN}")
  if(NOT DEFINED ${VAR})
    file(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/${FILENAME} "${CODE}")
    if(CXT_MESSAGE)
      message(STATUS "${CXT_MESSAGE}")
    endif(CXT_MESSAGE)
    try_compile(__CPP11_COMPILE ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/${FILENAME}
      COMPILE_DEFINITIONS "${CXT_FLAGS} ${CPP11_FLAGS}"
      OUTPUT_VARIABLE __CPP11_COMPILER_OUTPUT)
    set(${VAR} ${__CPP11_COMPILE})
    if(CXT_MESSAGE)
      if(__CPP11_COMPILE)
        message(STATUS "... ok")
      else(__CPP11_COMPILE)
        message(STATUS "... failed")
      endif(__CPP11_COMPILE)
    endif(CXT_MESSAGE)
  endif(NOT DEFINED ${VAR})
endmacro(test_cpp11_code)

#-------------------------------------------------------------------------
# Test for atomic variable support and set the CPP11_ATOMICS cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_atomics)
  test_cpp11_code(CPP11_ATOMICS testCpp11Atomics.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <atomic>
#include <iostream>

int main()
{
  std::atomic<int> var;
  var.store(42);
  std::cout << \"Value: \" << var.load() << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 atomics"
)
  set(CPP11_ATOMICS ${CPP11_ATOMICS} CACHE BOOL "C++11 atomic variables available?")
endmacro(test_cpp11_atomics)

#-------------------------------------------------------------------------
# Test for atomic variable support and set the CPP11_ATOMICS cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_auto)
  test_cpp11_code(CPP11_AUTO testCpp11Auto.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

int main()
{
  auto i = 42;
  std::cout << \"Value: \" << i << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 auto"
)
  set(CPP11_AUTO ${CPP11_AUTO} CACHE BOOL "C++11 auto keyword available?")
endmacro(test_cpp11_auto)

#-------------------------------------------------------------------------
# Test for constexpr support and set the CPP11_CONSTEXPR cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_constexpr)
  test_cpp11_code(CPP11_CONSTEXPR testCpp11Constexpr.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

constexpr float sqr(float f)
{
  return f * f;
}

int main()
{
  std::cout << \"4.2 squared is: \" << sqr(4.2f) << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 constexpr"
)
  set(CPP11_CONSTEXPR ${CPP11_CONSTEXPR} CACHE BOOL "C++11 constexpr available?")
endmacro(test_cpp11_constexpr)

#-------------------------------------------------------------------------
# Test for decltype support and set the CPP11_DECLTYPE cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_decltype)
  test_cpp11_code(CPP11_DECLTYPE testCpp11Decltype.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

struct IsIntArg
{
  bool isInt;

  explicit IsIntArg(int i) : isInt(true) {}
  template <typename T> IsIntArg(T arg) : isInt(false) {}
};

int main()
{
  int i = 42;
  decltype(i) j = i / 2;
  std::cout << \"Using decltype, j is an int? \" << IsIntArg(j).isInt << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 decltype"
)
  set(CPP11_DECLTYPE ${CPP11_DECLTYPE} CACHE BOOL "C++11 decltype available?")
endmacro(test_cpp11_decltype)

#-------------------------------------------------------------------------
# Test for default support and set the CPP11_DEFAULT cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_default)
  test_cpp11_code(CPP11_DEFAULT testCpp11Default.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

struct DStruct
{
  DStruct() = default;
};

int main()
{
  DStruct d;
  return 0;
}
"
MESSAGE "Testing for C++11 default"
)
  set(CPP11_DEFAULT ${CPP11_DEFAULT} CACHE BOOL "C++11 default functions available?")
endmacro(test_cpp11_default)

#-------------------------------------------------------------------------
# Test for initialiser list support and set the CPP11_INIT_LISTS cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_init_lists)
  test_cpp11_code(CPP11_INIT_LISTS testCpp11InitLists.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>
#include <vector>

int main()
{
  std::vector<int> intarray = { 0, 1, 2, 3, 4 };
  std::cout << \"Vector initialised with : \" << intarray.size() << \" elements.\" << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 initialiser lists"
)
  set(CPP11_INIT_LISTS ${CPP11_INIT_LISTS} CACHE BOOL "C++11 initialiser lists available?")
endmacro(test_cpp11_init_lists)

#-------------------------------------------------------------------------
# Test for lambda expression support and set the CPP11_LAMBDA cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_lambda)
  test_cpp11_code(CPP11_LAMBDA testCpp11Lambda.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <functional>
#include <iostream>

int main()
{
  std::function<float (float)> sqrFunc = [] (float x) { return x * x; };
  std::cout << \"4.2 squared is: \" << sqrFunc(4.2f) << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 lambda expressions"
)
  set(CPP11_LAMBDA ${CPP11_LAMBDA} CACHE BOOL "C++11 lambda expressions available?")
endmacro(test_cpp11_lambda)

#-------------------------------------------------------------------------
# Test for lambda expression support and set the CPP11_LAMBDA cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_override_final)
  test_cpp11_code(CPP11_OVERRIDE_FINAL testCpp11OverrideFinal.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

class A
{
public:
  virtual ~A() {}
  virtual const char *name() const { return \"A\"; }
};

class B : public A
{
public:
  const char *name() const override final { return \"B\"; }
};

int main()
{
  A *a0 = new A;
  A *a1 = new B;
  std::cout << \"a0 name: \" << a0->name() << std::endl;
  std::cout << \"a1 name: \" << a1->name() << std::endl;
  delete a0;
  delete a1;
  return 0;
}
"
MESSAGE "Testing for C++11 override/final keywords"
)
  set(CPP11_OVERRIDE_FINAL ${CPP11_OVERRIDE_FINAL} CACHE BOOL "C++11 override and final keywords?")
endmacro(test_cpp11_override_final)

#-------------------------------------------------------------------------
# Test for non-static member initialiser support and set the
# CPP11_NON_STATIC_INIT cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_non_static_init)
  test_cpp11_code(CPP11_NON_STATIC_INIT testCpp11NonStaticInit.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

struct Int
{
  int i = 42;
};

int main()
{
  Int ii;
  std::cout << \"Value: \" << ii.i << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 non-static member initialisers"
)
  set(CPP11_NON_STATIC_INIT ${CPP11_NON_STATIC_INIT} CACHE BOOL "C++11 non-static member initialisation available?")
endmacro(test_cpp11_non_static_init)

#-------------------------------------------------------------------------
# Test for range based for loop support and set the CPP11_RANGE_BASED_FOR cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_range_based_for)
  test_cpp11_code(CPP11_RANGE_BASED_FOR testCpp11RangeBasedFor.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>
#include <vector>

int main()
{
  std::vector<int> vec;
  vec.push_back(0);
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);

  std::cout << \"Items:\" << std::endl;

  for (int i : vec)
  {
    std::cout << \"  \" << i << std::endl;
  }
  std::cout << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 range based for loops"
)
  set(CPP11_RANGE_BASED_FOR ${CPP11_RANGE_BASED_FOR} CACHE BOOL "C++11 atomic variables available?")
endmacro(test_cpp11_range_based_for)

#-------------------------------------------------------------------------
# Test for r-value support and set the CPP11_RVALUES cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_rvalues)
  test_cpp11_code(CPP11_RVALUES testCpp11RValues.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

struct RValueStruct
{
  int i;
  RValueStruct(int i = 0) : i(i) {}
  RValueStruct(RValueStruct &&other) : i(other.i) {}
};

int main()
{
  RValueStruct a(RValueStruct(42));
  std::cout << \"RValue: \" << a.i << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 r-value references"
)
  set(CPP11_RVALUES ${CPP11_RVALUES} CACHE BOOL "C++11 r-values available?")
endmacro(test_cpp11_rvalues)

#-------------------------------------------------------------------------
# Test for static_assert support and set the CPP11_STATIC_ASSERT cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_static_assert)
  test_cpp11_code(CPP11_STATIC_ASSERT testCpp11StaticAssert.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

int main()
{
  static_assert(true, \"Static assert compiler test.\");
  return 0;
}
"
MESSAGE "Testing for C++11 static_assert"
)
  set(CPP11_STATIC_ASSERT ${CPP11_STATIC_ASSERT} CACHE BOOL "C++11 static_assert available?")
endmacro(test_cpp11_static_assert)

#-------------------------------------------------------------------------
# Test for thread support and set the CPP11_THREADS cache variable.
# Also uses chrono.
#-------------------------------------------------------------------------
macro(test_cpp11_threads)
  test_cpp11_code(CPP11_THREADS testCpp11Threads.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>

void threadEntry(int id, std::mutex *mutex)
{
  mutex->lock();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  std::cout << \"Thread \" << id << \" finishing\" << std::endl;
  mutex->unlock();
}

int main()
{
  std::mutex mutex;

  std::thread t0(&threadEntry, 0, &mutex);
  std::thread t1(&threadEntry, 1, &mutex);

  t0.join();
  t1.join();

  return 0;
}
"
MESSAGE "Testing for C++11 threads"
)
  set(CPP11_THREADS ${CPP11_THREADS} CACHE BOOL "C++11 thread and mutex classes available?")
endmacro(test_cpp11_threads)

#-------------------------------------------------------------------------
# Test for thread-local storage support and set the CPP11_THREAD_LOCAL
# cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_thread_local)
  test_cpp11_code(CPP11_THREAD_LOCAL testCpp11ThreadLocal.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

thread_local unsigned int ui = 0;

int main()
{
  ++ui;
  return 0;
}
"
MESSAGE "Testing for C++11 thread local storage"
)
  set(CPP11_THREAD_LOCAL ${CPP11_THREAD_LOCAL} CACHE BOOL "C++11 thread-local storage declarations available?")
endmacro(test_cpp11_thread_local)

#-------------------------------------------------------------------------
# Test for variadic template support and set the CPP11_VARIADIC_TEMPLATES
# cache variable.
#-------------------------------------------------------------------------
macro(test_cpp11_variadic_templates)
  test_cpp11_code(CPP11_VARIADIC_TEMPLATES testCpp11VariadicTemplates.cpp
"#ifndef __cplusplus
# error \"The CMAKE_CXX_COMPILER is set to a C compiler\"
#endif

#include <iostream>

template<typename T, typename... Args>
T sumnumeric(T val)
{
  return val;
}

template<typename T, typename... Args>
T sumnumeric(T first, Args... args)
{
  return first + sumnumeric(args...);
}

int main()
{
  int sum = sumnumeric(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  std::cout << \"Sum [1, 10] = \" << sum << std::endl;
  return 0;
}
"
MESSAGE "Testing for C++11 variadic templates"
)
  set(CPP11_VARIADIC_TEMPLATES ${CPP11_VARIADIC_TEMPLATES} CACHE BOOL "C++11 variadic template support?")
endmacro(test_cpp11_variadic_templates)

#-------------------------------------------------------------------------
# Resolve the appropriate compiler flag used to enable C++11 support.
#
# This tests a set of known, common flags, including no flag (Visual Studio).
# On success, the flag is set a CPP11_FLAGS cache variable. In either case
# the CPP11_FOUND cache variable is set to indicate whether any C++11
# support has been determined.
#-------------------------------------------------------------------------
macro(resolve_cpp11_flag)
  if(NOT CPP11_FOUND)
    # Defines the set of general C++11 flags for the compiler.
    # The empty flag is for Visual Studio and compilers which have C++11 enabled
    # by default. Other flags are for CLANG and G++, from newer flags to older.
    set(CPP11_FLAG_SET
      "-std=c++11 -stdlib=libc++"
      "-std=c++11"
      "-std=c++0x -stdlib=libc++"
      "-std=c++0x"
      "-std=gnu++11 -stdlib=libc++"
      "-std=gnu++11"
      "-std=gnu++0x -stdlib=libc++"
      "-std=gnu++0x"
      )

    set(CODE
"
int main()
{
  int *ptr = nullptr;
  return 0;
}
"
)

    set(TEST_FLAGS "")
    if(APPLE)
      # Force warning as error on MacOS to help detect the required flags.
      set(TEST_FLAGS "-Werror")
    endif(APPLE)
    message(STATUS "Testing for C++11 support")
    # Test with no flag.
    test_cpp11_code(CPP11_FOUND testCpp11.cpp "${CODE}" FLAGS "${TEST_FLAGS}")
    if(NOT CPP11_FOUND)
      # Cycle through the flags with code using nullptr to see which compiles first.
      foreach(CPP_FLAGS ${CPP11_FLAG_SET})
        unset(CPP11_FOUND)
        test_cpp11_code(CPP11_FOUND testCpp11.cpp "${CODE}" FLAGS "-Werror ${CPP_FLAGS}")
        #message("${CPP_FLAGS}: ${CPP11_FOUND}")
        if(CPP11_FOUND)
          # Successfully compiled. Use the current flag value.
          set(CPP11_FLAGS "${CPP_FLAGS}")
          break()
        endif(CPP11_FOUND)
      endforeach(CPP_FLAGS)
    else(NOT CPP11_FOUND)
      set(CPP_FLAGS "")
    endif(NOT CPP11_FOUND)

    if(CPP11_FOUND)
      if(CPP11_FLAGS)
        if(NOT APPLE AND UNIX)
          # Silly workaround for a Ubuntu G++ thread bug.
          # See https://bugs.launchpad.net/ubuntu/+source/gcc-defaults/+bug/1228201
          #find_package(Threads)
          set(CPP11_FLAGS "${CPP11_FLAGS} -lpthread -pthread -Wl,--no-as-needed")
        endif(NOT APPLE AND UNIX)
        message(STATUS "... found using flag: \"${CPP11_FLAGS}\"")
      else(CPP11_FLAGS)
        message(STATUS "... found (default)")
      endif(CPP11_FLAGS)
    else(CPP11_FOUND)
      message(STATUS "... failed")
    endif(CPP11_FOUND)

    # if(APPLE AND CPP11_FOUND)
    #   # Apple CLANG also needs -stdlib=libc++
    #   set(CPP11_FLAGS "${CPP11_FLAGS} -stdlib=libc++")
    # endif(APPLE AND CPP11_FOUND)
    set(CPP11_FOUND "${CPP11_FOUND}" CACHE BOOL "Is C++11 support present?")
  set(CPP11_FLAGS "${CPP11_FLAGS}" CACHE STRING "Compiler flags used to enable C++11. May be empty (as for Visual Studio).")
  endif(NOT CPP11_FOUND)
endmacro(resolve_cpp11_flag)

#-------------------------------------------------------------------------
# Script
#-------------------------------------------------------------------------

# Resolve what flags are required to build C++11 support. Also checks for gross level support.
resolve_cpp11_flag()

if(NOT CPP11_FOUND)
  return()
endif(NOT CPP11_FOUND)

# Now look for specific features.
test_cpp11_atomics()
test_cpp11_auto()
test_cpp11_constexpr()
test_cpp11_decltype()
test_cpp11_default()
test_cpp11_init_lists()
test_cpp11_lambda()
test_cpp11_override_final()
test_cpp11_non_static_init()
test_cpp11_range_based_for()
test_cpp11_rvalues()
test_cpp11_static_assert()
test_cpp11_threads()
test_cpp11_thread_local()
test_cpp11_variadic_templates()
