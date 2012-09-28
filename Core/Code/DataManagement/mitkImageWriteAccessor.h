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

#ifndef MITKIMAGEWRITEACCESSOR_H
#define MITKIMAGEWRITEACCESSOR_H

#include "mitkImageAccessorBase.h"
#include "mitkImage.h"

namespace mitk {

//##Documentation
//## @brief ImageWriteAccessor class to get locked write-access for a particular image part.
//## @ingroup Data
class ImageWriteAccessor : public ImageAccessorBase
{

public:
  /** \brief Orders write access for a slice, volume or 4D-Image
     *  \param Image::Pointer specifies the associated Image
     *  \param ImageDataItem* specifies the allocated image part
     *  \param OptionFlags properties from mitk::ImageAccessorBase::Options can be chosen and assembled with bitwise unification.
     *  \throws mitk::Exception if the Constructor was created inappropriately
     *  \throws mitk::MemoryIsLockedException if requested image area is exclusively locked and mitk::ImageAccessorBase::ExceptionIfLocked is set in OptionFlags
     */
  ImageWriteAccessor(
      ImagePointer iP,
      ImageDataItem* iDI = NULL,
      int OptionFlags = ImageAccessorBase::DefaultBehavior
      ) :
    ImageAccessorBase(iP , iDI, OptionFlags)

  {
    OrganizeWriteAccess();
  }

/** \brief Gives full data access. */
    virtual inline void * GetData()
    {
      return m_AddressBegin;
    }

/** \brief informs Image to unlock the represented image part */
  virtual ~ImageWriteAccessor()
  {
    // In case of non-coherent memory, copied area needs to be written back
    // TODO

    m_Image->m_ReadWriteLock.Lock();

    // delete self from list of ImageReadAccessors in Image
    std::vector<ImageAccessorBase*>::iterator it = std::find(m_Image->m_Writers.begin(),m_Image->m_Writers.end(),this);
    m_Image->m_Writers.erase(it);

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

private:

  /** \brief manages a consistent write access and locks the ordered image part */
  void OrganizeWriteAccess()
  {
    m_Image->m_ReadWriteLock.Lock();

    bool readOverlap = false;
    bool writeOverlap = false;

    ImageAccessorWaitLock* overlapLock = 0;

    // Check, if there is any Read-Access going on
    if(m_Image->m_Readers.size() > 0)
    {

      // Check for every ReadAccessor, if the Region of this ImageAccessors overlaps
      // make sure this iterator is not used, when m_ReadWriteLock is Unlocked!
      std::vector<ImageAccessorBase*>::iterator it = m_Image->m_Readers.begin();

      for(; it!=m_Image->m_Readers.end(); ++it)
      {
        ImageAccessorBase* r = *it;

        if(/*r->m_Active &&*/  Overlap(r))
        {
          // An Overlap was detected.
          readOverlap = true;
          overlapLock = r->m_WaitLock;
          break;
        } // if
      } // for
    } // if

    // Check, if there is any Write-Access going on
    if(m_Image->m_Writers.size() > 0)
    {

      // Check for every WriteAccessor, if the Region of this ImageAccessors overlaps
      // make sure this iterator is not used, when m_ReadWriteLock is Unlocked!
      std::vector<ImageAccessorBase*>::iterator it = m_Image->m_Writers.begin();

      for(; it!=m_Image->m_Writers.end(); ++it)
      {
        ImageAccessorBase* w = *it;

        if(/*w->m_Active &&*/  Overlap(w))
        {
          // An Overlap was detected.
          writeOverlap = true;
          overlapLock = w->m_WaitLock;
          break;
        } // if
      } // for
    } // if

    if(readOverlap || writeOverlap) {
      // Throw an exception or wait for the WriteAccessor w until it is released and start again with the request afterwards.
      if(!(m_Options & ExceptionIfLocked))
      {
        // WAIT
        overlapLock->m_WaiterCount += 1;
        m_Image->m_ReadWriteLock.Unlock();
        ImageAccessorBase::WaitForReleaseOf(overlapLock);

        //after waiting for the ImageAccessor, start this method again
        OrganizeWriteAccess();
        return;
      }
      else
      {
        // THROW EXCEPTION
        m_Image->m_ReadWriteLock.Unlock();
        mitkThrowException(mitk::MemoryIsLockedException) << "The image part being ordered by the ImageAccessor is already in use and locked";
        //MITK_ERROR("Speicherbereich belegt");
        return;
      }
    }

    // Now, we know, that there is no conflict with a Read- or Write-Access
    // Lock the Mutex in ImageAccessorBase, to make sure that every other ImageAccessor has to wait
    m_WaitLock->m_Mutex.Lock();

    // insert self into Writers list in Image
    m_Image->m_Writers.push_back(this);

    //printf("WriteAccess %d %d\n",(int) m_Image->m_Readers.size(),(int) m_Image->m_Writers.size());
    //fflush(0);
    m_Image->m_ReadWriteLock.Unlock();

  }

};

}
#endif // MITKIMAGEWRITEACCESSOR_H
