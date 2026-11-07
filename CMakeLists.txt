cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "VITASDK не задан")
  endif()
endif()

project(volctl C)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2 -std=gnu99 -nostdlib")

add_executable(volctl src/main.c)
target_link_libraries(volctl
  SceLibKernel_stub
  SceKernelThreadMgr_stub
  SceCtrl_stub
  SceAVConfig_stub
)

vita_create_self(volctl.suprx volctl CONFIG ${CMAKE_SOURCE_DIR}/exports.yml UNSAFE)
