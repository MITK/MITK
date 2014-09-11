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


#ifndef IMAGESOURCE_H_HEADER_INCLUDED_C1E7D6EC
#define IMAGESOURCE_H_HEADER_INCLUDED_C1E7D6EC

#include <MitkCoreExports.h>
#include "mitkBaseDataSource.h"
#include "mitkImage.h"


namespace mitk {

/**
 * @brief Superclass of all classes generating Images (instances of class
 * Image) as output.
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
 * for mitk::ImageSource. But there are also three access methods provided
 * that guarantee an up-to-date result (by first calling Update and then
 * returning the result of GetOutput()): GetData(), GetPic() and
 * GetVtkImageData().
 * @ingroup Process
 */
class MITK_CORE_EXPORT ImageSource : public BaseDataSource
{
public:
  mitkClassMacro(ImageSource,BaseDataSource)

  /** @brief Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** @brief Some convenient typedefs. */
  typedef mitk::Image OutputImageType;
  typedef OutputImageType OutputType;
  typedef OutputImageType::Pointer OutputImagePointer;
  typedef SlicedData::RegionType OutputImageRegionType;

  /**
   * @brief Get the output data of this image source object.
   *
   * The output of this
   * function is not valid until an appropriate Update() method has
   * been called, either explicitly or implicitly.  Both the filter
   * itself and the data object have Update() methods, and both
   * methods update the data.  Here are three ways to use
   * GetOutput() and make sure the data is valid.  In these
   * examples, \a image is a pointer to some Image object, and the
   * particular ProcessObjects involved are filters.  The same
   * examples apply to non-image (e.g. Mesh) data as well.
   *
   * \code
   *   anotherFilter->SetInput( someFilter->GetOutput() );
   *   anotherFilter->Update();
   * \endcode
   *
   * In this situation, \a someFilter and \a anotherFilter are said
   * to constitute a \b pipeline.
   *
   * \code
   *   image = someFilter->GetOutput();
   *   image->Update();
   * \endcode
   *
   * \code
   *   someFilter->Update();
   *   image = someFilter->GetOutput();
   * \endcode
   * (In the above example, the two lines of code can be in
   * either order.)
   *
   * Note that Update() is not called automatically except within a
   * pipeline as in the first example.  When \b streaming (using a
   * StreamingImageFilter) is activated, it may be more efficient to
   * use a pipeline than to call Update() once for each filter in
   * turn.
   *
   * For an image, the data generated is for the requested
   * Region, which can be set using ImageBase::SetRequestedRegion().
   * By default, the largest possible region is requested.
   *
   * For Filters which have multiple outputs of different types, the
   * GetOutput() method assumes the output is of OutputImageType. For
   * the GetOutput(DataObjectPointerArraySizeType) method, a dynamic_cast is performed
   * incase the filter has outputs of different types or image
   * types. Derived classes should have named get methods for these
   * outputs.
   */
  mitkBaseDataSourceGetOutputDeclarations

  /** @brief Make a DataObject of the correct type to used as the specified
   * output.
   *
   * Every ProcessObject subclass must be able to create a
   * DataObject that can be used as a specified output. This method
   * is automatically called when DataObject::DisconnectPipeline() is
   * called.  DataObject::DisconnectPipeline, disconnects a data object
   * from being an output of its current source.  When the data object
   * is disconnected, the ProcessObject needs to construct a replacement
   * output data object so that the ProcessObject is in a valid state.
   * So DataObject::DisconnectPipeline eventually calls
   * ProcessObject::MakeOutput. Note that MakeOutput always returns a
   * SmartPointer to a DataObject. If a subclass of ImageSource has
   * multiple outputs of different types, then that class must provide
   * an implementation of MakeOutput(). */
  virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx );

  /**
   * This is a default implementation to make sure we have something.
   * Once all the subclasses of ProcessObject provide an appopriate
   * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
   * virtual.
   */
  virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name);

  virtual vtkImageData* GetVtkImageData();
  virtual const vtkImageData* GetVtkImageData() const;

protected:
  ImageSource();
  virtual ~ImageSource() {}

  /** @brief A version of GenerateData() specific for image processing
   * filters.
   *
   * This implementation will split the processing across
   * multiple threads. The buffer is allocated by this method. Then
   * the BeforeThreadedGenerateData() method is called (if
   * provided). Then, a series of threads are spawned each calling
   * ThreadedGenerateData(). After all the threads have completed
   * processing, the AfterThreadedGenerateData() method is called (if
   * provided). If an image processing filter cannot be threaded, the
   * filter should provide an implementation of GenerateData(). That
   * implementation is responsible for allocating the output buffer.
   * If a filter an be threaded, it should NOT provide a
   * GenerateData() method but should provide a ThreadedGenerateData()
   * instead.
   *
   * \sa ThreadedGenerateData() */
  virtual void GenerateData();

  /** @brief If an imaging filter can be implemented as a multithreaded
   * algorithm, the filter will provide an implementation of
   * ThreadedGenerateData().
   *
   * This superclass will automatically split
   * the output image into a number of pieces, spawn multiple threads,
   * and call ThreadedGenerateData() in each thread. Prior to spawning
   * threads, the BeforeThreadedGenerateData() method is called. After
   * all the threads have completed, the AfterThreadedGenerateData()
   * method is called. If an image processing filter cannot support
   * threading, that filter should provide an implementation of the
   * GenerateData() method instead of providing an implementation of
   * ThreadedGenerateData().  If a filter provides a GenerateData()
   * method as its implementation, then the filter is responsible for
   * allocating the output data.  If a filter provides a
   * ThreadedGenerateData() method as its implementation, then the
   * output memory will allocated automatically by this superclass.
   * The ThreadedGenerateData() method should only produce the output
   * specified by "outputThreadRegion"
   * parameter. ThreadedGenerateData() cannot write to any other
   * portion of the output image (as this is responsibility of a
   * different thread).
   *
   * \sa GenerateData(), SplitRequestedRegion() */
  virtual
  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            itk::ThreadIdType threadId );


  /** @brief This method is intentionally left blank.
   *
   * ImageSource's need not
   * Initialize their containers. The Image::Allocate() method (called
   * from GenerateData()) will resize the container if more memory is
   * needed.  Otherwise, the memory can be reused.
   */
  virtual void PrepareOutputs();

  /** @brief The GenerateData method normally allocates the buffers for all of the
   * outputs of a filter.
   *
   * Some filters may want to override this default
   * behavior. For example, a filter may have multiple outputs with
   * varying resolution. Or a filter may want to process data in place by
   * grafting its input to its output.*/
  virtual void AllocateOutputs();

  /** @brief If an imaging filter needs to perform processing after the buffer
   * has been allocated but before threads are spawned, the filter can
   * can provide an implementation for BeforeThreadedGenerateData().
   *
   * The execution flow in the default GenerateData() method will be:
   *      1) Allocate the output buffer
   *      2) Call BeforeThreadedGenerateData()
   *      3) Spawn threads, calling ThreadedGenerateData() in each thread.
   *      4) Call AfterThreadedGenerateData()
   * Note that this flow of control is only available if a filter provides
   * a ThreadedGenerateData() method and NOT a GenerateData() method. */
  virtual void BeforeThreadedGenerateData() {}

  /** @brief If an imaging filter needs to perform processing after all
   * processing threads have completed, the filter can can provide an
   * implementation for AfterThreadedGenerateData().
   *
   * The execution
   * flow in the default GenerateData() method will be:
   *      1) Allocate the output buffer
   *      2) Call BeforeThreadedGenerateData()
   *      3) Spawn threads, calling ThreadedGenerateData() in each thread.
   *      4) Call AfterThreadedGenerateData()
   * Note that this flow of control is only available if a filter provides
   * a ThreadedGenerateData() method and NOT a GenerateData() method. */
  virtual void AfterThreadedGenerateData() {}

  /** @brief Split the output's RequestedRegion into "num" pieces, returning
   * region "i" as "splitRegion".
   *
   * This method is called "num" times. The
   * regions must not overlap. The method returns the number of pieces that
   * the routine is capable of splitting the output RequestedRegion,
   * i.e. return value is less than or equal to "num". */
  virtual
  unsigned int SplitRequestedRegion(unsigned int i, unsigned int num, OutputImageRegionType& splitRegion);

  /** @brief Static function used as a "callback" by the MultiThreader.
   *
   * The threading library will call this routine for each thread, which will delegate the
   * control to ThreadedGenerateData(). */
  static ITK_THREAD_RETURN_TYPE ThreaderCallback( void *arg );

  /** @brief Internal structure used for passing image data into the threading library */
  struct ThreadStruct
  {
   Pointer Filter;
  };

private:
  ImageSource(const Self &);   //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // namespace mitk

#endif /* IMAGESOURCE_H_HEADER_INCLUDED_C1E7D6EC */
