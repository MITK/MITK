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

#include "mitkImageReadAccessor.h"

#include "mitkImage.h"

mitk::ImageReadAccessor::ImageReadAccessor(
    ImageConstPointer image,
    const mitk::ImageDataItem* iDI,
    int OptionFlags)
  : ImageAccessorBase(image,iDI,OptionFlags)
  , m_Image(image)
{
  if(!(OptionFlags & ImageAccessorBase::IgnoreLock))
  {
    try
    {
      OrganizeReadAccess();
    }
    catch (...)
    {
      delete m_WaitLock;
      throw;
    }
  }
}

mitk::ImageReadAccessor::ImageReadAccessor(
    ImagePointer image,
    const mitk::ImageDataItem* iDI,
    int OptionFlags)
  : ImageAccessorBase(image.GetPointer(), iDI, OptionFlags)
  , m_Image(image.GetPointer())
{
  if(!(OptionFlags & ImageAccessorBase::IgnoreLock))
  {
    try
    {
      OrganizeReadAccess();
    }
    catch (...)
    {
      delete m_WaitLock;
      throw;
    }
  }
}

mitk::ImageReadAccessor::ImageReadAccessor(const mitk::Image* image, const ImageDataItem* iDI)
  : ImageAccessorBase(image, iDI, ImageAccessorBase::DefaultBehavior)
  , m_Image(image)
{
  OrganizeReadAccess();
}

mitk::ImageReadAccessor::~ImageReadAccessor()
{
  if(!(m_Options & ImageAccessorBase::IgnoreLock))
  {
    // Future work: In case of non-coherent memory, copied area needs to be deleted

    m_Image->m_ReadWriteLock.Lock();

    // delete self from list of ImageReadAccessors in Image
    std::vector<ImageAccessorBase*>::iterator it = std::find(m_Image->m_Readers.begin(),m_Image->m_Readers.end(),this);
    m_Image->m_Readers.erase(it);

    // delete lock, if there are no waiting ImageAccessors
    if(m_WaitLock->m_WaiterCount <= 0)
    {
      m_WaitLock->m_Mutex.Unlock();
      delete m_WaitLock;
    }
    else
    {
      m_WaitLock->m_Mutex.Unlock();
    }

    m_Image->m_ReadWriteLock.Unlock();
  }
  else
  {
    delete m_WaitLock;
  }
}

const mitk::Image* mitk::ImageReadAccessor::GetImage() const
{
  return m_Image.GetPointer();
}

void mitk::ImageReadAccessor::OrganizeReadAccess()
{
  m_Image->m_ReadWriteLock.Lock();

  // Check, if there is any Write-Access going on
  if(m_Image->m_Writers.size() > 0)
  {
    // Check for every WriteAccessors, if the Region of this ImageAccessors overlaps
    // make sure this iterator is not used, when m_ReadWriteLock is Unlocked!
    std::vector<ImageAccessorBase*>::iterator it = m_Image->m_Writers.begin();

    for(; it!=m_Image->m_Writers.end(); ++it)
    {
      ImageAccessorBase* w = *it;
      if( Overlap(w) )
      {
        // An Overlap was detected. There are two possibilities to deal with this situation:
        // Throw an exception or wait for the WriteAccessor w until it is released and start again with the request afterwards.
        if(!(m_Options & ExceptionIfLocked))
        {
          PreventRecursiveMutexLock(w);

          // WAIT
          w->Increment();
          m_Image->m_ReadWriteLock.Unlock();
          ImageAccessorBase::WaitForReleaseOf(w->m_WaitLock);

          //after waiting for the WriteAccessor w, start this method again
          OrganizeReadAccess();
          return;
        }
        else
        {
          // THROW EXCEPTION
          m_Image->m_ReadWriteLock.Unlock();
          mitkThrowException(mitk::MemoryIsLockedException) << "The image part being ordered by the ImageAccessor is already in use and locked";
          return;
        }
      } // if
    } // for
  } // if

  // Now, we know, that there is no conflict with a Write-Access
  // Lock the Mutex in ImageAccessorBase, to make sure that every other ImageAccessor has to wait if it locks the mutex
  m_WaitLock->m_Mutex.Lock();

  // insert self into readers list in Image
  m_Image->m_Readers.push_back(this);

  //printf("ReadAccess %d %d\n",(int) m_Image->m_Readers.size(),(int) m_Image->m_Writers.size());
  //fflush(0);
  m_Image->m_ReadWriteLock.Unlock();
}
