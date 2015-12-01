#==============================================================================
# Apple specific configuration.
#
# Handles both OSX and iOS.
#==============================================================================

# The CUDA nvcc does not support clang for host compilation. To that end, we need
# to explicitly specify a different compiler (gcc). This may have implications
# with C++ 11 compatibility as we are only checking the primary compiler for that.
# That is to say, gcc must have the same C++ 11 compatibility or C++ 11 features
# need to be avoided in the CU files.
# See http://stackoverflow.com/questions/16286588/cuda-5-0-cmake-and-make-failing-on-osx-10-8-3
#if (NOT DEFINED CUDA_HOST_COMPILER AND CMAKE_C_COMPILER_ID STREQUAL "Clang" AND EXISTS /usr/bin/gcc)
#  set(CUDA_HOST_COMPILER /usr/bin/gcc CACHE FILEPATH "Host side compiler used by NVCC")
#  message(STATUS "Setting CMAKE_HOST_COMPILER to /usr/bin/gcc instead of ${CMAKE_C_COMPILER}.")
#endif()

#set(MACOSX_RPATH ON CACHE INTERNAL "")

# Apple compiler: Clang/LLVM
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE CXX_VERSION)
if(CXX_VERSION MATCHES "Apple (clang)|(LLVM).*")
  # Disable spurious warnings for Clang.
  add_definitions(-Wno-logical-op-parentheses)
endif()

# # Set up use of g++ compiler, but link to CLANG libraries.
# function(gcc_link_to_clang)
#   if (CMAKE_COMPILER_IS_GNUCXX)
#     #set()
#     set(XCODE_TOOL_CHAIN "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain")
#     set(XCODE_LIBCXX "/usr/lib/libc++.dylib /usr/lib/libc++abi.dylib /usr/lib/libSystem.dylib")
#     set(XCODE_LIBC "/usr/lib/libc.dylib")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc -nostdlib -I${XCODE_TOOL_CHAIN}/usr/lib/c++/v1 -I${XCODE_TOOL_CHAIN}/usr/lib/clang/5.1/include -I/usr/include" PARENT_SCOPE)
#     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib -std=c++11 ${XCODE_LIBCXX} ${XCODE_LIBC}" PARENT_SCOPE)
#     set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib -std=c++11 ${XCODE_LIBCXX} ${XCODE_LIBC}" PARENT_SCOPE)
#     set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -nostdlib -std=c++11" PARENT_SCOPE)
#     # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -nostdinc++ -nostdlib -I${XCODE_TOOL_CHAIN}/usr/lib/c++/v1" PARENT_SCOPE)
#     # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib ${XCODE_LIBCXX} ${XCODE_LIBC}" PARENT_SCOPE)
#     # set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -nostdlib ${XCODE_LIBCXX} ${XCODE_LIBC}" PARENT_SCOPE)
#   endif(CMAKE_COMPILER_IS_GNUCXX)
# endfunction(gcc_link_to_clang)

# # Note: Clang and GCC use different settings for 64 bit setup.
# macro(set_osx_architecture)
#   make_option(MACOSX_I386 "Build for i386 architecture?" NO)
#   if(MACOSX_I386)
#     execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE GCC_VERSION_STRING)
#     if(GCC_VERSION_STRING MATCHES "LLVM")
#       # Clang force 32 bit.
#       set(CMAKE_OSX_ARCHITECTURES i386) # force 32 bit on OSX
#       message("-arch i386")
#     else(GCC_VERSION_STRING MATCHES "LLVM")
#       # GCC force 32 bit.
#       add_definitions("-march=i386 -m32")
#       set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -march=i386 -m32")
#     endif(GCC_VERSION_STRING MATCHES "LLVM")
#     set(CSF_BUILD64 NO)
#   else(MACOSX_I386)
#     set(CSF_BUILD64 YES)
#   endif(MACOSX_I386)
# endmacro(set_osx_architecture)
