/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageAccessorBase_h
#define mitkImageAccessorBase_h

#include <itkImageRegion.h>
#include <itkIndex.h>
#include <itkSmartPointer.h>

#include <mitkImageDataItem.h>

#include <mutex>

namespace mitk
{
  /**
   * \brief Base class providing a locking mechanism for all image accessor classes.
   *
   * ImageAccessorBase establishes the lock protocol for concurrent read/write access
   * to mitk::Image data. All concrete accessor classes (ImageReadAccessor,
   * ImageWriteAccessor, ImagePixelReadAccessor, ImagePixelWriteAccessor,
   * ImageVtkReadAccessor, ImageVtkWriteAccessor) inherit from this class.
   *
   * The locking ensures that overlapping image memory regions are not written
   * and read simultaneously from different threads, preventing data races.
   *
   * \sa mitk::ImageReadAccessor, mitk::ImageWriteAccessor
   * \sa mitk::ImagePixelReadAccessor, mitk::ImagePixelWriteAccessor
   * \sa mitk::ImageVtkReadAccessor, mitk::ImageVtkWriteAccessor
   * \ingroup Data
   */

  class Image;

  /**
   * \brief Wait-lock structure allowing ImageAccessors to wait for each other.
   *
   * When an ImageAccessor cannot immediately acquire access because another
   * accessor holds an overlapping region, it increments the waiter count and
   * waits on the mutex until the blocking accessor is released.
   */
  struct ImageAccessorWaitLock
  {
    /** \brief Number of ImageAccessors waiting for this accessor to be released. */
    unsigned int m_WaiterCount;

    /** \brief Mutex that other ImageAccessors lock on to wait for this accessor. */
    std::mutex m_Mutex;
  };

// Defs to assure dead lock prevention only in case of possible thread handling.
#if defined(ITK_USE_SPROC) || defined(ITK_USE_PTHREADS) || defined(ITK_USE_WIN32_THREADS)
#define MITK_USE_RECURSIVE_MUTEX_PREVENTION
#endif

  class MITKCORE_EXPORT ImageAccessorBase
  {
    friend class Image;

    friend class ImageReadAccessor;
    friend class ImageWriteAccessor;

    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelReadAccessor;

    template <class TPixel, unsigned int VDimension>
    friend class ImagePixelWriteAccessor;

  public:
    /** \brief Const smart pointer type for the associated Image. */
    typedef itk::SmartPointer<const mitk::Image> ImageConstPointer;

    /**
     * \brief Option flags that control ImageAccessor behavior.
     *
     * These flags can be combined via bitwise OR and passed to accessor
     * constructors.
     */
    enum Options
    {
      /** \brief Default behavior: wait for locked memory to be released. */
      DefaultBehavior = 0,
      /** \brief Throw MemoryIsLockedException instead of waiting if the memory is locked. */
      ExceptionIfLocked = 1,
      /** \brief Force the accessed memory to be coherent (contiguous). May cause
       *  additional memory allocation and data copying, potentially affecting performance. */
      ForceCoherentMemory = 2,
      /** \brief Bypass the lock mechanism for immediate access. Only valid for read accessors. */
      IgnoreLock = 4
    };

    /** \brief Virtual destructor. Releases the wait lock if no other accessors are waiting. */
    virtual ~ImageAccessorBase();

    /**
     * \brief Get const access to the raw image data.
     *
     * \return Pointer to the beginning of the accessed image memory region.
     */
    inline const void *GetData() const { return m_AddressBegin; }
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

    /** \brief Checks validity of given parameters from inheriting classes and stores those parameters in member
     * variables. */
    ImageAccessorBase(ImageConstPointer iP, const ImageDataItem *iDI = nullptr, int OptionFlags = DefaultBehavior);

    /** ImageAccessor has access to the image it belongs to. */
    // ImagePointer m_Image;

    /** Contains a SubRegion (always represented in maximal possible dimension) */
    itk::ImageRegion<4> *m_SubRegion;

    /** Points to the beginning of the image part. */
    void *m_AddressBegin;

    /** Contains the first address after the image part. */
    void *m_AddressEnd;

    /** \brief Stores all extended properties of an ImageAccessor.
      * The different flags in mitk::ImageAccessorBase::Options can be unified by bitwise operations.
      */
    int m_Options;

    /** Defines if the accessed image part lies coherently in memory */
    bool m_CoherentMemory;

    /** \brief Pointer to a WaitLock struct, that allows other ImageAccessors to wait for this ImageAccessor */
    ImageAccessorWaitLock *m_WaitLock;

    /** \brief Increments m_WaiterCount. A call of this method is prohibited unless the Mutex m_ReadWriteLock in the
     * mitk::Image class is Locked. */
    inline void Increment() { m_WaitLock->m_WaiterCount += 1; }
    /** \brief Computes if there is an Overlap of the image part between this instantiation and another ImageAccessor
     * object
      * \throws mitk::Exception if memory area is incoherent (not supported yet)
      */
    bool Overlap(const ImageAccessorBase *iAB);

    /** \brief Uses the WaitLock to wait for another ImageAccessor*/
    void WaitForReleaseOf(ImageAccessorWaitLock *wL);

    ThreadIDType m_Thread;

    /** \brief Prevents a recursive mutex lock by comparing thread ids of competing image accessors */
    void PreventRecursiveMutexLock(ImageAccessorBase *iAB);

    virtual const Image *GetImage() const = 0;

  private:
    /** \brief System dependent thread method, to prevent recursive mutex access */
    ThreadIDType CurrentThreadHandle();
    /** \brief System dependent thread method, to prevent recursive mutex access */
    inline bool CompareThreadHandles(ThreadIDType, ThreadIDType);
  };

  class MemoryIsLockedException : public Exception
  {
  public:
    mitkExceptionClassMacro(MemoryIsLockedException, Exception)
  };
}

#endif
