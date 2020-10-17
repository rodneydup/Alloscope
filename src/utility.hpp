#include <iostream>
#include <vector>

// custom ringbuffer class for storing scope values (needed to draw the trail)
class RingBuffer {
 public:
  RingBuffer(unsigned maxSize) : mMaxSize(maxSize) {
    mBuffer.resize(mMaxSize);
    mTail = -1;
    mPrevSample = 0;
  }

  unsigned getMaxSize() const { return mMaxSize; }

  void resize(unsigned maxSize) {
    mMaxSize = maxSize;
    mBuffer.resize(mMaxSize);
  }

  void push_back(float value) {
    mTail = (mTail + 1) % mMaxSize;
    mBuffer[mTail] = value;
  }

  unsigned getTail() const { return mTail; }

  float at(unsigned index) {
    if (index >= mMaxSize) {
      std::cerr << "RingBuffer index out of range." << std::endl;
      index = index % mMaxSize;
    }
    mPrevSample = mBuffer.at(index);
    return mPrevSample;
  }

  float operator[](unsigned index) { return this->at(index); }

  std::vector<float> getArray(unsigned lookBack) {
    std::vector<float> array(lookBack, 0);
    int start = mTail - lookBack;
    if (start < 0) start = mMaxSize + start;
    for (unsigned i = 0; i < lookBack; i++) array[i] = mBuffer[(start + i) % mMaxSize];
    return array;
  }

 private:
  std::vector<float> mBuffer;
  unsigned mMaxSize;
  int mTail;
  float mPrevSample;
};