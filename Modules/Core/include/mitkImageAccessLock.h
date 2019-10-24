#pragma once

#include "mitkImageRegionAccessor.h"

namespace mitk {

class MITKCORE_EXPORT ImageAccessLock
{
  friend Image;

public:
  ImageAccessLock(ImageRegionAccessor* accessor, bool writeAccess = false);
  virtual ~ImageAccessLock();

  bool getWriteAccess();
  ImageRegionAccessor* getAccessor();

protected:
  ImageRegionAccessor* m_RegionAccessor;
  bool m_WriteAccess;

private:
  ImageAccessLock& operator=(const ImageAccessLock&);
  ImageAccessLock(const ImageAccessLock&);
};

}

