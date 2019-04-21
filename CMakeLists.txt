cmake_minimum_required(VERSION 3.12)

project (dbtest)

include_directories(
  /home/harshvs/github/leveldb/1.21/leveldb-1.21/include
  /home/harshvs/projs/gtest/googletest-release-1.8.1/googletest/include
  /home/harshvs/projs/gtest/googletest-release-1.8.1/googlemock/include)

set(CMAKE_CXX_STANDARD 11)

add_executable (dbtest
  dbtest.cpp
  PersistentList.cpp
  PersistentListIterator.cpp)

target_link_libraries(dbtest
  /home/harshvs/github/leveldb/1.21/leveldb-1.21/build/libleveldb.a
  /home/harshvs/projs/gtest/googletest-release-1.8.1/build/googlemock/libgmock.a
  /home/harshvs/projs/gtest/googletest-release-1.8.1/build/googlemock/gtest/libgtest.a
  pthread)

