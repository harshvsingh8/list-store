cmake_minimum_required(VERSION 3.12)

project (dbtest)

include_directories(
  /home/harshvs/github/leveldb/include
  /home/harshvs/github/googletest/googletest/include
  /home/harshvs/github/googletest/googlemock/include)

set(CMAKE_CXX_STANDARD 11)

add_executable (dbtest
  dbtest.cpp
  PersistentList.cpp
  PersistentListIterator.cpp)

target_link_libraries(dbtest
  /home/harshvs/github/leveldb/build/libleveldb.a
  /home/harshvs/github/googletest/build/lib/libgmock.a
  /home/harshvs/github/googletest/build/lib/libgtest.a
  pthread)

