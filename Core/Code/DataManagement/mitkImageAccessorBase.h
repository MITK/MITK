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

#ifndef MITKIMAGEACCESSORBASE_H
#define MITKIMAGEACCESSORBASE_H

#include <itkIndex.h>
#include <itkImageRegion.h>
#include <itkSimpleFastMutexLock.h>
#include <itkSmartPointer.h>
#include <itkMultiThreader.h>

#include "mitkImageDataItem.h"

namespace mitk {

//##Documentation
//## @brief The ImageAccessorBase class provides a lock mechanism for all inheriting image accessors.
//##
//## @ingroup Data

class Image;

/** \brief This struct allows to make ImageAccessors wait for this particular ImageAccessor object*/
struct ImageAccessorWaitLock
{
  /** \brief Holds the number of ImageAccessors, which are waiting until the represented ImageAccessor is released. */
  unsigned int m_WaiterCount;

  /** \brief A mutex that allows other ImageAccessors to wait for the represented ImageAccessor. */
  itk::SimpleFastMutexLock m_Mutex;
};

// Defs to assure dead lock prevention only in case of possible thread handling.
#if defined(ITK_USE_SPROC) || defined(ITK_USE_PTHREADS) || defined(ITK_USE_WIN32_THREADS)
  #define MITK_USE_RECURSIVE_MUTEX_PREVENTION
#endif

class MITK_CORE_EXPORT ImageAccessorBase
{

  friend class Image;

  friend class ImageReadAccessor;
  friend class ImageWriteAccessor;

  template <class TPixel, unsigned int VDimension>
  friend class ImagePixelReadAccessor;

  template <class TPixel, unsigned int VDimension>
  friend class ImagePixelWriteAccessor;

public:

  typedef itk::SmartPointer<const mitk::Image> ImageConstPointer;

  /** \brief defines different flags for the ImageAccessor constructors
    */
  enum Options {
    /** No specific Options ==> Default */
    DefaultBehavior = 0,
    /** Defines if the Constructor waits for locked memory until it is released or not. If not, an exception is thrown.*/
    ExceptionIfLocked = 1,
    /** Defines if requested Memory has to be coherent. If the parameter is true, it is possible that new Memory has to be allocated to arrange this desired condition. Consequently, this parameter can heavily affect computation time.*/
    ForceCoherentMemory = 2,
    /** Ignores the lock mechanism for immediate access. Only possible with read accessors. */
    IgnoreLock = 4
  };

  virtual ~ImageAccessorBase();

  /** \brief Gives const access to the data. */
  inline const void * GetData() const
  {
    return m_AddressBegin;
  }

protected:

  // Define type of thread id
#ifdef ITK_USE_SPROC
typedef int ThreadIDType;
#endif

#ifdef ITK_USE_WIN32_THREADS
typedef DWORD ThreadIDType;
#endif

#ifdef ITK_USE_PTHREADS
typedef pthread_t ThreadIDType;
#endif

  /** \brief Checks validity of given parameters from inheriting classes and stores those parameters in member variables. */
  ImageAccessorBase(ImageConstPointer iP,
      const ImageDataItem* iDI = NULL,
      int OptionFlags = DefaultBehavior
    );

  /** ImageAccessor has access to the image it belongs to. */
  //ImagePointer m_Image;

  /** Contains a SubRegion (always represented in maximal possible dimension) */
  itk::ImageRegion<4>* m_SubRegion;

  /** Points to the beginning of the image part. */
  void* m_AddressBegin;

  /** Contains the first address after the image part. */
  void* m_AddressEnd;

  /** \brief Stores all extended properties of an ImageAccessor.
    * The different flags in mitk::ImageAccessorBase::Options can be unified by bitwise operations.
    */
  int m_Options;

  /** Defines if the accessed image part lies coherently in memory */
  bool m_CoherentMemory;

  /** \brief Pointer to a WaitLock struct, that allows other ImageAccessors to wait for this ImageAccessor */
  ImageAccessorWaitLock* m_WaitLock;

  /** \brief Increments m_WaiterCount. A call of this method is prohibited unless the Mutex m_ReadWriteLock in the mitk::Image class is Locked. */
  inline void Increment()
  {
    m_WaitLock->m_WaiterCount += 1;
  }

  /** \brief Computes if there is an Overlap of the image part between this instantiation and another ImageAccessor object
    * \throws mitk::Exception if memory area is incoherent (not supported yet)
    */
  bool Overlap(const ImageAccessorBase* iAB);

  /** \brief Uses the WaitLock to wait for another ImageAccessor*/
  void WaitForReleaseOf(ImageAccessorWaitLock* wL);

  ThreadIDType m_Thread;

  /** \brief Prevents a recursive mutex lock by comparing thread ids of competing image accessors */
  void PreventRecursiveMutexLock(ImageAccessorBase* iAB);

  virtual const Image* GetImage() const = 0;

private:

  /** \brief System dependend thread method, to prevent recursive mutex access */
  ThreadIDType CurrentThreadHandle();
  /** \brief System dependend thread method, to prevent recursive mutex access */
  inline bool CompareThreadHandles(ThreadIDType, ThreadIDType);

};


class MemoryIsLockedException : public Exception
{
public:
  mitkExceptionClassMacro(MemoryIsLockedException,Exception)
};

}

#endif
