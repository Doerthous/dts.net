cmake_minimum_required(VERSION 3.15)

project(dts.net.test)

include_directories(../include ../conf)
add_executable(dts_net_dblk_test dts_net_dblk_test.c ../dts_net_dblk.c)
