#include "mitkImageAccessLock.h"

#include "mitkImage.h"

namespace mitk {

ImageAccessLock::ImageAccessLock(ImageRegionAccessor* accessor, bool writeAccess)
{
  // Make sure, that the Image is initialized properly
  Image::Pointer image = accessor->getImage();
  if(image->m_Initialized==false) {
    if(image->GetSource().IsNull()) {
      mitkThrow() << "ImageAccessor: No image source is defined";
    }
    image->m_ReadWriteLock.Lock();
    if(image->GetSource()->Updating()==false) {
      image->GetSource()->UpdateOutputInformation();
    }
    image->m_ReadWriteLock.Unlock();
  }

  m_RegionAccessor = accessor;
  m_WriteAccess = writeAccess;

  while(true) {
    image->m_ReadWriteLock.Lock();
    if (image->canAddAccessLock(this)) {
      image->addAccessLock(this);
      image->m_ReadWriteLock.Unlock();
      break;
    }
    image->m_ReadWriteLock.Unlock();

    std::unique_lock<std::mutex> lk(image->m_ImageAccessLockRemovedMutex);
    image->m_ImageAccessLockRemoved.wait(lk);
  }
}

ImageAccessLock::~ImageAccessLock()
{
  Image::Pointer image = m_RegionAccessor->getImage();
  image->m_ReadWriteLock.Lock();
  image->removeAccessLock(this);
  image->m_ReadWriteLock.Unlock();
}

bool ImageAccessLock::getWriteAccess()
{
  return m_WriteAccess;
}

ImageRegionAccessor* ImageAccessLock::getAccessor()
{
  return m_RegionAccessor;
}

}

