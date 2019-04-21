
#include <leveldb/db.h>
#include <memory>

#pragma once

class PersistentList;

class PersistentListIterator {
public:
  PersistentListIterator(std::shared_ptr<PersistentList> list);

  virtual ~PersistentListIterator();

  bool Valid() const;

  std::string Key() const;
  std::string Value() const;

  bool Next();
  bool Prev();

  void SeekFront();
  void SeekBack();

  std::string ListId() const;

private:
  PersistentListIterator(const PersistentListIterator &) = delete;
  PersistentListIterator &operator=(const PersistentListIterator &) = delete;

private:
  bool mValid;
  std::shared_ptr<PersistentList> mList;
  std::unique_ptr<leveldb::Iterator> mIter;
};
