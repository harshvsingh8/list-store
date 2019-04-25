#include "PersistentList.h"
#include "PersistentListIterator.h"
#include "leveldb/db.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

std::shared_ptr<PersistentList>
PersistentList::Get(std::shared_ptr<leveldb::DB> db,
                    const std::string &listName) {
  return std::shared_ptr<PersistentList>(new PersistentList(db, listName));
}

PersistentList::PersistentList(std::shared_ptr<leveldb::DB> db,
                               const std::string &listName)
    : mDB(db), mListName(listName) {
  using namespace leveldb;

  string idKey(KEY_PREFIX + mListName + "/id");
  string idValue;
  Status s = mDB->Get(mReadOptions, idKey, &idValue);

  if (s.IsNotFound()) {
    // cout << "PersistentList: Creating new list for:" << listName << endl;
    string nextListIdKey(string(KEY_PREFIX) +  "next_id");
    string listId;
    s = mDB->Get(mReadOptions, nextListIdKey, &listId);

    if (s.IsNotFound()) {
      listId = "0"; // this is the first persistent list
      mDB->Put(mWriteOptions, nextListIdKey, listId);
    }
    // cerr << listId << endl;
    string nextListId = to_string(stoi(listId) + 1);
    mDB->Put(mWriteOptions, nextListIdKey, nextListId);

    mDB->Put(mWriteOptions, idKey, listId);
    mListId = listId;
    mKeyPrefix = KEY_PREFIX + mListId + "/"; // needed by GetKey()
    mDB->Put(mWriteOptions, GetKey(string(1, START_SYM)), "42");
    mDB->Put(mWriteOptions, GetKey(string(1, END_SYM)), "42");
    // cout << "PersistentList: Created new list with Id:" << idValue << endl;
  } else {
    mListId = idValue;
  }
  mKeyPrefix = KEY_PREFIX + mListId + "/";
  mHeadKey = GetKey(string(1, START_SYM));
  mTailKey = GetKey(string(1, END_SYM));
}

std::string PersistentList::Name() const { return mListName; }

int PersistentList::Size() const {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  int count = 0;

  while (true) {
    iter->Next();
    string nextKey = iter->key().ToString();

    if (mTailKey.compare(nextKey) == 0)
      break;

    count++;
  }
  return count;
}

std::string PersistentList::PushFront(const std::string &value) {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  iter->Next();
  string firstKey = iter->key().ToString();
  string prevKey;

  if (mTailKey.compare(firstKey) == 0) {
    prevKey = GetKey(INIT_KEY_SEQ);
  } else {
    prevKey = PrevKey(firstKey);
  }
  mDB->Put(mWriteOptions, prevKey, value);
  return prevKey;
}

std::string PersistentList::PushBack(const std::string &value) {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mTailKey);
  iter->Prev();
  string lastKey = iter->key().ToString();
  string nextKey;

  if (mHeadKey.compare(lastKey) == 0) {
    nextKey = GetKey(INIT_KEY_SEQ);
  } else {
    nextKey = NextKey(lastKey);
  }
  mDB->Put(mWriteOptions, nextKey, value);
  return nextKey;
}

std::pair<bool, std::string> PersistentList::Front() const {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  iter->Next();
  string firstKey = iter->key().ToString();

  if (mTailKey.compare(firstKey) != 0) {
    return pair<bool, string>(true, iter->value().ToString());
  } else {
    return pair<bool, std::string>(false, "");
  }
}

std::pair<bool, std::string> PersistentList::Back() const {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mTailKey);
  iter->Prev();
  string lastKey = iter->key().ToString();

  if (mHeadKey.compare(lastKey) != 0) {
    return pair<bool, string>(true, iter->value().ToString());
  } else {
    return pair<bool, std::string>(false, "");
  }
}

bool PersistentList::PopFront() {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  iter->Next();
  string firstKey = iter->key().ToString();

  if (mTailKey.compare(firstKey) != 0) {
    leveldb::Status s = mDB->Delete(mWriteOptions, firstKey);
    return s.ok();
  } else {
    return false;
  }
}

bool PersistentList::PopBack() {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mTailKey);
  iter->Prev();
  string lastKey = iter->key().ToString();

  if (mHeadKey.compare(lastKey) != 0) {
    leveldb::Status s = mDB->Delete(mWriteOptions, lastKey);
    return s.ok();
  } else {
    return false;
  }
}

bool PersistentList::PopKey(const std::string &key) {
  leveldb::Status s = mDB->Delete(mWriteOptions, key);
  return s.ok();
}

bool PersistentList::PopValue(const std::string &value) {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  bool deleted = false;
  while (true) {
    iter->Next();
    string nextKey = iter->key().ToString();

    if (mTailKey.compare(nextKey) == 0)
      break;

    if (value.compare(iter->value().ToString()) == 0) {
      mDB->Delete(mWriteOptions, nextKey);
      deleted = true;
    }
  }
  return deleted;
}

void PersistentList::Clear() {
  auto iter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  iter->Seek(mHeadKey);
  bool deleted = false;
  while (true) {
    iter->Next();
    string nextKey = iter->key().ToString();

    if (mTailKey.compare(nextKey) == 0)
      break;

    mDB->Delete(mWriteOptions, nextKey);
  }
}

void PersistentList::Compact() {
  leveldb::Slice rangeStart(mHeadKey);
  leveldb::Slice rangeEnd(mTailKey);
  mDB->CompactRange(&rangeStart, &rangeEnd);
}

std::string PersistentList::InsertAt(const PersistentListIterator *iter,
                                     const std::string &value) {
  using namespace std;
  assert(iter->Valid());
  assert(iter->ListId().compare(mListId) == 0);

  string nextKey = iter->Key();
  auto dbIter = unique_ptr<leveldb::Iterator>(mDB->NewIterator(mReadOptions));
  dbIter->Seek(nextKey);
  dbIter->Prev();
  string prevKey = dbIter->key().ToString();

  if (mHeadKey.compare(prevKey) == 0) {
    return PushFront(value);
  }
  string middleKey = MidKey(prevKey, nextKey);
  mDB->Put(mWriteOptions, middleKey, value);
  return middleKey;
}

std::string PersistentList::NextKey(const std::string &key) const {
  const char *keyData = string(key, mKeyPrefix.length()).c_str();
  char nextKeyData[KEY_LEN];

  for (int i = 0; i < KEY_LEN; i++)
    nextKeyData[i] = keyData[i];

  char carry = 0;

  for (int i = KEY_LEN - 1; i >= 0; i--) {
    const char c = nextKeyData[i];
    char next_c = c + carry + (char)((i == KEY_LEN - 1) ? 1 : 0);
    carry = 0;

    if (next_c == END_SYM) {
      next_c = START_SYM + 1;
      carry = 1;
    }
    nextKeyData[i] = next_c;

    if (carry == 0)
      break;
  }
  return mKeyPrefix + string(nextKeyData, KEY_LEN);
}

std::string PersistentList::PrevKey(const std::string &key) const {
  const char *keyData = string(key, mKeyPrefix.length()).c_str();
  char nextKeyData[KEY_LEN];

  for (int i = 0; i < KEY_LEN; i++)
    nextKeyData[i] = keyData[i];

  char carry = 0;

  for (int i = KEY_LEN - 1; i >= 0; i--) {
    const char c = nextKeyData[i];
    char next_c = c - carry - (char)((i == KEY_LEN - 1) ? 1 : 0);
    carry = 0;

    if (next_c == START_SYM) {
      next_c = END_SYM - 1;
      carry = 1;
    }
    nextKeyData[i] = next_c;

    if (carry == 0)
      break;
  }
  return mKeyPrefix + string(nextKeyData, KEY_LEN);
}

std::string PersistentList::MidKey(const std::string &key1,
                                   const std::string &key2) const {
  using namespace std;
  string key1Seq = string(key1, mKeyPrefix.length());
  string key2Seq = string(key2, mKeyPrefix.length());

  int maxKeyLen = max(key1Seq.length(), key2Seq.length());

  vector<int> key1Data(maxKeyLen, 0);
  vector<int> key2Data(maxKeyLen, 0);

  for (int i = 0; i < key1Seq.length(); i++) {
    key1Data[i] = (int)(key1Seq[i] - START_SYM - 1);
  }
  for (int i = 0; i < key2Seq.length(); i++) {
    key2Data[i] = (int)(key2Seq[i] - START_SYM - 1);
  }

  long long diff = 0;
  long long posVal = 1;
  for (int i = maxKeyLen - 1, carry = 0; i >= 0; i--) {
    int val = key2Data[i] - key1Data[i] - carry;
    carry = 0;
    if (val < 0) {
      val += KEY_BASE;
      carry = 1;
    }
    diff += val * posVal;
    posVal *= KEY_BASE;
  }

  long long offset = diff / 2;
  // cout<<"offset:"<<offset<<endl;

  if (offset == 0) {
    // there is no space, expand the key and return
    return key1 + MIDDLE_SYM;
  }

  vector<int> offsetData(maxKeyLen, 0);

  for (int i = maxKeyLen - 1; i >= 0 && offset > 0; i--) {
    int val = offset % KEY_BASE;
    offsetData[i] = val;
    offset /= KEY_BASE;
  }

  vector<int> keyData(maxKeyLen, 0);
  for (int i = maxKeyLen - 1, carry = 0; i >= 0; i--) {
    int val = key1Data[i] + offsetData[i] + carry;
    carry = 0;
    if (val >= KEY_BASE) {
      val -= KEY_BASE;
      carry = 1;
    }
    keyData[i] = val;
  }
  string middleKey(maxKeyLen, START_SYM + 1);
  for (int i = 0; i < maxKeyLen; i++)
    middleKey[i] = (char)(keyData[i] + START_SYM + 1);

  return mKeyPrefix + middleKey;
}

PersistentList::~PersistentList() {}
