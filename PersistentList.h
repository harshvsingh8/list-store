#include <leveldb/db.h>
#include <memory>
#include <utility>

#pragma once

class PersistentListIterator;

class PersistentList {
public:
  static std::shared_ptr<PersistentList> Get(std::shared_ptr<leveldb::DB> db,
                                             const std::string &listName);

  virtual ~PersistentList();

  inline std::string Id() const { return mListId; }

  std::string Name() const;

  int Size() const;

  std::string PushFront(const std::string &value);
  std::string PushBack(const std::string &value);

  std::string InsertAt(const PersistentListIterator *iter,
                       const std::string &value);

  // TODO - move to optional in C++ 17
  std::pair<bool, std::string> Front() const;
  std::pair<bool, std::string> Back() const;

  bool PopFront();
  bool PopBack();

  bool PopKey(const std::string &key);
  bool PopValue(const std::string &value);

  void Clear();
  void Delete();

  void Compact();

  // std::shared_ptr<PersistentListIterator> Iterator();

  // The Iterator needs access to the list details
  friend class PersistentListIterator;

private:
  PersistentList(std::shared_ptr<leveldb::DB> db, const std::string &listName);

  PersistentList(const PersistentList &list) = delete;
  PersistentList &operator=(const PersistentList &list) = delete;

  inline std::string GetKey(std::string keySeq) const {
    return mKeyPrefix + keySeq;
  }

  std::string NextKey(const std::string &key) const;
  std::string PrevKey(const std::string &key) const;

public:
  std::string MidKey(const std::string &key1, const std::string &key2) const;

private:
  static constexpr int KEY_BASE = 92;
  static constexpr int KEY_LEN = 8;
  static constexpr char START_SYM = '!';
  static constexpr char END_SYM = '~';
  static constexpr char MIDDLE_SYM = 'N';
  static constexpr int ASCII_OFFSET = 34;
  static constexpr const char *INIT_KEY_SEQ = "NNNNNNNN";

  std::shared_ptr<leveldb::DB> mDB;

  std::string mListName;
  std::string mListId;
  std::string mHeadKey;
  std::string mTailKey;
  std::string mKeyPrefix;

  // default read/write options
  leveldb::WriteOptions mWriteOptions;
  leveldb::ReadOptions mReadOptions;
};
