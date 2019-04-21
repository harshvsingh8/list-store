
#include "PersistentListIterator.h"
#include "PersistentList.h"

using namespace std;

PersistentListIterator::PersistentListIterator(
    std::shared_ptr<PersistentList> list)
    : mList(list), mValid(false),
      mIter(unique_ptr<leveldb::Iterator>(
          mList->mDB->NewIterator(mList->mReadOptions))) {}

PersistentListIterator::~PersistentListIterator() {}

bool PersistentListIterator::Valid() const { return mValid; }

std::string PersistentListIterator::ListId() const { return mList->Id(); }

std::string PersistentListIterator::Key() const {
  assert(mValid);
  return mIter->key().ToString();
}

std::string PersistentListIterator::Value() const {
  assert(mValid);
  return mIter->value().ToString();
}

bool PersistentListIterator::Next() {
  assert(mIter->Valid());
  mValid = mList->mTailKey.compare(mIter->key().ToString()) != 0;
  if (mValid) {
    mIter->Next();
    mValid = mList->mTailKey.compare(mIter->key().ToString()) != 0;
  }
  return mValid;
}

bool PersistentListIterator::Prev() {
  assert(mIter->Valid());
  mValid = mList->mHeadKey.compare(mIter->key().ToString()) != 0;
  if (mValid) {
    mIter->Prev();
    mValid = mList->mHeadKey.compare(mIter->key().ToString()) != 0;
  }
  return mValid;
}

void PersistentListIterator::SeekFront() {
  mIter->Seek(mList->mHeadKey);
  mValid = false;
}

void PersistentListIterator::SeekBack() {
  mIter->Seek(mList->mTailKey);
  mValid = false;
}
