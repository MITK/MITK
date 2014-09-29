/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImageAccessorBase.h"
#include "mitkImage.h"

mitk::ImageAccessorBase::ThreadIDType mitk::ImageAccessorBase::CurrentThreadHandle()
{
  #ifdef ITK_USE_SPROC
    return GetCurrentThreadId();
  #endif

  #ifdef ITK_USE_PTHREADS
    return pthread_self();
  #endif

  #ifdef ITK_USE_WIN32_THREADS
    return GetCurrentThreadId();
  #endif
}

bool mitk::ImageAccessorBase::CompareThreadHandles(mitk::ImageAccessorBase::ThreadIDType handle1, mitk::ImageAccessorBase::ThreadIDType handle2)
{
  return handle1 == handle2;
}

mitk::ImageAccessorBase::~ImageAccessorBase()
{
}

mitk::ImageAccessorBase::ImageAccessorBase(ImageConstPointer image,
                                           const ImageDataItem* imageDataItem,
                                           int OptionFlags)
  : // m_Image(iP)
//, imageDataItem(iDI)
    m_SubRegion(NULL)
  , m_Options(OptionFlags)
  , m_CoherentMemory(false)
{
  m_Thread = CurrentThreadHandle();

  // Initialize WaitLock
  m_WaitLock = new ImageAccessorWaitLock();
  m_WaitLock->m_WaiterCount = 0;

  // Check validity of ImageAccessor

  // Is there an Image?
  /*
  if(!m_Image)
  {
    mitkThrow() << "Invalid ImageAccessor: No Image was specified in constructor of ImageAccessor";
  }
  */

  if(image)
  {
    // Make sure, that the Image is initialized properly
    if(image->m_Initialized==false)
    {
      if(image->GetSource().IsNull())
      {
        mitkThrow() << "ImageAccessor: No image source is defined";
      }
      image->m_ReadWriteLock.Lock();
      if(image->GetSource()->Updating()==false)
      {
        image->GetSource()->UpdateOutputInformation();
      }
      image->m_ReadWriteLock.Unlock();
    }
  }

  // Investigate 4 cases of possible image parts/regions

  // Case 1: No ImageDataItem and no Subregion => Whole Image is accessed
  if(imageDataItem == NULL && m_SubRegion == NULL)
  {
    m_CoherentMemory = true;

    // Organize first image channel
    image->m_ReadWriteLock.Lock();
    imageDataItem = image->GetChannelData();
    image->m_ReadWriteLock.Unlock();

    // Set memory area
    m_AddressBegin = imageDataItem->m_Data;
    m_AddressEnd   = (unsigned char*) m_AddressBegin + imageDataItem->m_Size;
  }

  // Case 2: ImageDataItem but no Subregion
  if(imageDataItem && m_SubRegion == NULL)
  {
    m_CoherentMemory = true;

    // Set memory area
    m_AddressBegin = imageDataItem->m_Data;
    m_AddressEnd   = (unsigned char*) m_AddressBegin + imageDataItem->m_Size;
  }

  // Case 3: No ImageDataItem but a SubRegion
  if(imageDataItem == NULL && m_SubRegion)
  {
    mitkThrow() << "Invalid ImageAccessor: The use of a SubRegion is not supported (yet).";
  }

  // Case 4: ImageDataItem and SubRegion
  if(imageDataItem == NULL && m_SubRegion)
  {
    mitkThrow() << "Invalid ImageAccessor: The use of a SubRegion is not supported (yet).";
  }

}

/** \brief Computes if there is an Overlap of the image part between this instantiation and another ImageAccessor object
 * \throws mitk::Exception if memory area is incoherent (not supported yet)
 */
bool mitk::ImageAccessorBase::Overlap(const ImageAccessorBase* iAB)
{
  if(m_CoherentMemory)
  {
    if((iAB->m_AddressBegin >= m_AddressBegin && iAB->m_AddressBegin <  m_AddressEnd) ||
       (iAB->m_AddressEnd   >  m_AddressBegin && iAB->m_AddressEnd   <= m_AddressEnd))
    {
      return true;
    }
    if((m_AddressBegin >= iAB->m_AddressBegin && m_AddressBegin <  iAB->m_AddressEnd) ||
       (m_AddressEnd   >  iAB->m_AddressBegin && m_AddressEnd   <= iAB->m_AddressEnd))
    {
      return true;
    }

  }
  else
  {
    GetImage()->m_ReadWriteLock.Unlock();
    mitkThrow() << "ImageAccessor: incoherent memory area is not supported yet";
  }

  return false;
}

/** \brief Uses the WaitLock to wait for another ImageAccessor*/
void mitk::ImageAccessorBase::WaitForReleaseOf(ImageAccessorWaitLock* wL)
{
  wL->m_Mutex.Lock();

  // Decrement
  wL->m_WaiterCount -= 1;

  // If there are no more waiting ImageAccessors, delete the Mutex
  // (Der Letzte macht das Licht aus!)
  if(wL->m_WaiterCount <= 0)
  {
    wL->m_Mutex.Unlock();
    delete wL;
  }
  else
  {
    wL->m_Mutex.Unlock();
  }
}

void mitk::ImageAccessorBase::PreventRecursiveMutexLock(mitk::ImageAccessorBase* iAB)
{
#ifdef MITK_USE_RECURSIVE_MUTEX_PREVENTION
  // Prevent deadlock
  ThreadIDType id = CurrentThreadHandle();
  if(CompareThreadHandles(id,iAB->m_Thread))
  {
    GetImage()->m_ReadWriteLock.Unlock();
    mitkThrow() << "Prohibited image access: the requested image part is already in use and cannot be requested recursively!";
  }
#endif
}
