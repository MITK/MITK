/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __itkMultiOutputNaryFunctorImageFilter_h
#define __itkMultiOutputNaryFunctorImageFilter_h

#include "itkImageToImageFilter.h"
#include "itkImageIterator.h"
#include "itkArray.h"

namespace itk
{
/** \class MultiOutputNaryFunctorImageFilter
 * \brief Perform a generic pixel-wise operation on N images and produces m output images.
 *
 * This is an extension of the itk::NaryFunctorImageFilter. It assumes that the functor
 * returns not only one result pixel value but a vector of m values an therefore generates
 * m output images. In addition (and difference to itk::NaryFunctorImageFilter) it also
 * passes the index of the current value vector to the functor (for space correlated
 * evaluations).\n
 * Class is templated over the types of the input images
 * and the type of the output images.  It is also templated by the
 * operation to be applied.  A Functor style is used to represent the
 * function.\n
 *
 * All the input images must be of the same type.
 *
 * \ingroup IntensityImageFilters MultiThreaded
 * \ingroup ITKImageIntensity
 */

template< class TInputImage, class TOutputImage, class TFunction, class TMaskImage = ::itk::Image<unsigned char, TInputImage::ImageDimension> >
class ITK_EXPORT MultiOutputNaryFunctorImageFilter:
  public ImageToImageFilter< TInputImage, TOutputImage >

{
public:
  /** Standard class typedefs. */
  typedef MultiOutputNaryFunctorImageFilter                          Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                            Pointer;
  typedef SmartPointer< const Self >                      ConstPointer;
  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(MultiOutputNaryFunctorImageFilter, ImageToImageFilter);

  /** Some typedefs. */
  typedef TFunction                            FunctorType;
  typedef TInputImage                          InputImageType;
  typedef typename InputImageType::Pointer     InputImagePointer;
  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename InputImageType::PixelType   InputImagePixelType;
  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputImagePixelType;
  typedef typename FunctorType::InputPixelArrayType     NaryInputArrayType;
  typedef typename FunctorType::OutputPixelArrayType    NaryOutputArrayType;
  typedef TMaskImage MaskImageType;
  typedef typename MaskImageType::Pointer     MaskImagePointer;
  typedef typename MaskImageType::RegionType  MaskImageRegionType;

  /** Get the functor object.  The functor is returned by reference.
   * (Functors do not have to derive from itk::LightObject, so they do
   * not necessarily have a reference count. So we cannot return a
   * SmartPointer). */
  FunctorType & GetFunctor() { return m_Functor; }

  /** Set the functor object.  This replaces the current Functor with a
   * copy of the specified Functor. This allows the user to specify a
   * functor that has ivars set differently than the default functor.
   * This method requires an operator!=() be defined on the functor
   * (or the compiler's default implementation of operator!=() being
   * appropriate). */
  void SetFunctor(FunctorType & functor)
  {
    if ( m_Functor != functor )
      {
      m_Functor = functor;
      this->ActualizeOutputs();
      this->Modified();
      }
  }

  itkSetObjectMacro(Mask, MaskImageType);
  itkGetConstObjectMacro(Mask, MaskImageType);

  /** ImageDimension constants */
  itkStaticConstMacro(
    InputImageDimension, unsigned int, TInputImage::ImageDimension);
  itkStaticConstMacro(
    OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro( SameDimensionCheck,
                   ( Concept::SameDimension< InputImageDimension, OutputImageDimension > ) );
  itkConceptMacro( OutputHasZeroCheck,
                   ( Concept::HasZero< OutputImagePixelType > ) );
  /** End concept checking */
#endif
protected:
  MultiOutputNaryFunctorImageFilter();
  ~MultiOutputNaryFunctorImageFilter() override {}

  /** MultiOutputNaryFunctorImageFilter can be implemented as a multi threaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData() routine
   * which is called for each processing thread. The output image data is
   * allocated automatically by the superclass prior to calling
   * ThreadedGenerateData().  ThreadedGenerateData can only write to the
   * portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData()  */
  void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                            ThreadIdType threadId) override;

  /** Methods actualize the output settings of the filter according to the current functor*/
  void ActualizeOutputs();

private:
  MultiOutputNaryFunctorImageFilter(const Self &); //purposely not implemented
  void operator=(const Self &);         //purposely not implemented

  FunctorType m_Functor;
  MaskImagePointer m_Mask;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMultiOutputNaryFunctorImageFilter.tpp"
#endif

#endif
