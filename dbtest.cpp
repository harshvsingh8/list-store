#include "leveldb/db.h"
#include "PersistentList.h"
#include "PersistentListIterator.h"
#include "gtest/gtest.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <utility>

class PersistentListTest : public ::testing::Test {
protected:
  void SetUp() override {
    leveldb::DB *db;
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status openStatus = leveldb::DB::Open(options, "./db", &db);
    assert(openStatus.ok());
    spDB.reset(db);
  }

  std::shared_ptr<leveldb::DB> spDB;
  leveldb::WriteOptions writeOptions;
  leveldb::ReadOptions readOptions;
};

TEST_F(PersistentListTest, ListCreate) {
  auto pl1 = PersistentList::Get(spDB, "mylist1");
  auto pl2 = PersistentList::Get(spDB, "mylist2");
  ASSERT_TRUE(pl1->Id() != "");
  ASSERT_TRUE(pl2->Id() != "");

  auto pl11 = PersistentList::Get(spDB, "mylist1");
  auto pl22 = PersistentList::Get(spDB, "mylist2");

  ASSERT_TRUE(pl1->Id() == pl11->Id());
  ASSERT_TRUE(pl2->Id() == pl22->Id());
}

TEST_F(PersistentListTest, CheckName) {
  auto plX = PersistentList::Get(spDB, "mylist");
  EXPECT_EQ(plX->Name(), "mylist");
  auto plY = PersistentList::Get(spDB, "mylist");
  EXPECT_EQ(plX->Name(), plY->Name());
}

TEST_F(PersistentListTest, CheckBackAPI) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at back
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushBack(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  // Check front - access/pop API
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    auto atFront = pl->Front();
    ASSERT_TRUE(atFront.first);
    EXPECT_EQ(data, atFront.second);
    pl->PopFront();
  }

  ASSERT_TRUE(pl->Size() == 0);

  // Add again at back
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushBack(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  // Check back access/pop api
  for (int i = max_range - 1; i >= 0; i--) {
    std::string data = std::to_string(i);
    auto atBack = pl->Back();
    ASSERT_TRUE(atBack.first);
    EXPECT_EQ(data, atBack.second);
    pl->PopBack();
  }

  ASSERT_TRUE(pl->Size() == 0);
}

TEST_F(PersistentListTest, CheckFrontAPI) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at front
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushFront(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  // Check front - access/pop API
  for (int i = max_range - 1; i >= 0; i--) {
    std::string data = std::to_string(i);
    auto atFront = pl->Front();
    ASSERT_TRUE(atFront.first);
    EXPECT_EQ(data, atFront.second);
    pl->PopFront();
  }

  ASSERT_TRUE(pl->Size() == 0);

  // Add again at front
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushFront(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  // Check back access/pop api
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    auto atBack = pl->Back();
    ASSERT_TRUE(atBack.first);
    EXPECT_EQ(data, atBack.second);
    pl->PopBack();
  }

  ASSERT_TRUE(pl->Size() == 0);
}

TEST_F(PersistentListTest, CheckMixAPI) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at front
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    if (i % 2 == 0) {
      pl->PushFront(data);
    } else {
      pl->PushBack(data);
    }
  }

  ASSERT_TRUE(pl->Size() == max_range);

  for (int i = max_range - 1; i >= 0; i--) {
    std::string i_str = std::to_string(i);
    pair<bool, string> data;
    if (i % 2 == 0) {
      data = pl->Front();
      pl->PopFront();
    } else {
      data = pl->Back();
      pl->PopBack();
    }
    ASSERT_TRUE(data.first);
    EXPECT_EQ(i_str, data.second);
  }

  ASSERT_TRUE(pl->Size() == 0);
}

TEST_F(PersistentListTest, CheckFwdIterAPI) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at back
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushBack(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  auto iter =
      std::unique_ptr<PersistentListIterator>(new PersistentListIterator(pl));
  int count = 0;
  iter->SeekFront();
  EXPECT_FALSE(iter->Valid());
  while (iter->Next()) {
    EXPECT_TRUE(iter->Valid());
    string value = iter->Value();
    EXPECT_EQ(to_string(count), value);
    count++;
  }
  EXPECT_FALSE(iter->Valid());
}

TEST_F(PersistentListTest, CheckBackIterAPI) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at back
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushBack(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  auto iter =
      std::unique_ptr<PersistentListIterator>(new PersistentListIterator(pl));
  int count = 255;
  iter->SeekBack();
  EXPECT_FALSE(iter->Valid());
  while (iter->Prev()) {
    EXPECT_TRUE(iter->Valid());
    string value = iter->Value();
    EXPECT_EQ(to_string(count), value);
    count--;
  }
  EXPECT_FALSE(iter->Valid());
}

TEST_F(PersistentListTest, CheckInsertAt) {
  using namespace std;

  const int max_range = 256;
  auto pl = PersistentList::Get(spDB, "mylist");

  pl->Clear();
  ASSERT_TRUE(pl->Size() == 0);

  // Add at back
  for (int i = 0; i < max_range; i++) {
    std::string data = std::to_string(i);
    pl->PushBack(data);
  }

  ASSERT_TRUE(pl->Size() == max_range);

  auto iter = std::unique_ptr<PersistentListIterator>(new PersistentListIterator(pl));
  int count = 255;
  iter->SeekFront();
  EXPECT_FALSE(iter->Valid());
  while (iter->Next()) {
    EXPECT_TRUE(iter->Valid());
    string value = iter->Value();
    pl->InsertAt(iter.get(), to_string(count));
    count--;
  }
  EXPECT_FALSE(iter->Valid());


  // iter =  std::unique_ptr<PersistentListIterator>(new PersistentListIterator(pl));
  // iter->SeekFront();
  // while(iter->Next()) {
  //   cout<<iter->Key()<<" -> "<<iter->Value()<<endl;
  // }
  
  
  iter =  std::unique_ptr<PersistentListIterator>(new PersistentListIterator(pl));
  iter->SeekFront();
  int i = 0;
  int j = 255;
  while(iter->Next()) {
    string valF = iter->Value();
    EXPECT_TRUE(iter->Next());
    string valS = iter->Value();
    EXPECT_EQ(valF, to_string(j));
    EXPECT_EQ(valS, to_string(i));
    i++;
    j--;
  }
}


TEST_F(PersistentListTest, CheckMidKeyAPI_1) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AABBCC", "pl/"+ id +"/AABBCE");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/AABBCD");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_2) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AABBCC", "pl/"+ id +"/AABBCD");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/AABBCCN");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_3) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AABBCC", "pl/"+ id +"/AABBCCN");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/AABBCC8");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_4) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AABBCC8", "pl/"+ id +"/AABBCCN");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/AABBCCC");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_5) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AABBCC", "pl/"+ id +"/ACBBCC");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/ABBBCC");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_6) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/AA", "pl/"+ id +"/CCCCC");
  // cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/BB2``");
}

TEST_F(PersistentListTest, CheckMidKeyAPI_7) {
  using namespace std;

  auto pl = PersistentList::Get(spDB, "mylist");

  string id = pl->Id();
  string key = pl->MidKey("pl/" + id +"/NNNNNNNN", "pl/"+ id +"/NNNNNNNNN");
  cerr<<"Mid:"<<key<<endl;
  EXPECT_EQ(key, "pl/"+ id +"/NNNNNNNN8");
}


TEST_F(PersistentListTest, CheckDB) {
  // leveldb::Iterator *iter = spDB->NewIterator(readOptions);
  // for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
  //   std::string key = iter->key().ToString();
  //   std::string value = iter->value().ToString();
  //   std::cout << key << " -> " << value << std::endl;
  // }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
  return 0;
}
