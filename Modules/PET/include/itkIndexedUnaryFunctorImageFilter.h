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

#ifndef __itkIndexedUnaryFunctorImageFilter_h
#define __itkIndexedUnaryFunctorImageFilter_h

#include "itkUnaryFunctorImageFilter.h"

namespace itk
{
/** \class IndexedUnaryFunctorImageFilter
 * \brief Perform a generic pixel-wise index specific operation on an input image and produces an output image.
 *
 * This is filter is simelar to itk::UnaryFunctorImageFilter, but it can used for operations where the index position
 * in the input image is relevant for the output result.\n
 * Class is templated over the type of the input image
 * and the type of the output image. It is also templated by the
 * operation to be applied.  A Functor style is used to represent the
 * function.\n
 *
 * \ingroup IntensityImageFilters MultiThreaded
 * \ingroup ITKImageIntensity
 */

  template< typename TInputImage, typename TOutputImage, typename TFunction >
  class IndexedUnaryFunctorImageFilter :public InPlaceImageFilter< TInputImage, TOutputImage >
  {
  public:
    /** Standard class typedefs. */
    typedef IndexedUnaryFunctorImageFilter                         Self;
    typedef InPlaceImageFilter< TInputImage, TOutputImage > Superclass;
    typedef SmartPointer< Self >                            Pointer;
    typedef SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(IndexedUnaryFunctorImageFilter, InPlaceImageFilter);

    /** Some typedefs. */
    typedef TFunction FunctorType;

    typedef TInputImage                              InputImageType;
    typedef typename    InputImageType::ConstPointer InputImagePointer;
    typedef typename    InputImageType::RegionType   InputImageRegionType;
    typedef typename    InputImageType::PixelType    InputImagePixelType;

    typedef TOutputImage                             OutputImageType;
    typedef typename     OutputImageType::Pointer    OutputImagePointer;
    typedef typename     OutputImageType::RegionType OutputImageRegionType;
    typedef typename     OutputImageType::PixelType  OutputImagePixelType;

    /** Get the functor object.  The functor is returned by reference.
    * (Functors do not have to derive from itk::LightObject, so they do
    * not necessarily have a reference count. So we cannot return a
    * SmartPointer.) */
    FunctorType &       GetFunctor() { return m_Functor; }
    const FunctorType & GetFunctor() const { return m_Functor; }

    /** Set the functor object.  This replaces the current Functor with a
    * copy of the specified Functor. This allows the user to specify a
    * functor that has ivars set differently than the default functor.
    * This method requires an operator!=() be defined on the functor
    * (or the compiler's default implementation of operator!=() being
    * appropriate). */
    void SetFunctor(const FunctorType & functor)
    {
      if (m_Functor != functor)
      {
        m_Functor = functor;
        this->Modified();
      }
    }

  protected:
    IndexedUnaryFunctorImageFilter();
    virtual ~IndexedUnaryFunctorImageFilter() {}

    /** IndexedUnaryFunctorImageFilter can produce an image which is a different
    * resolution than its input image.  As such, IndexedUnaryFunctorImageFilter
    * needs to provide an implementation for
    * GenerateOutputInformation() in order to inform the pipeline
    * execution model.  The original documentation of this method is
    * below.
    *
    * \sa ProcessObject::GenerateOutputInformaton()  */
    virtual void GenerateOutputInformation();

    /** IndexedUnaryFunctorImageFilter can be implemented as a multithreaded filter.
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
      ThreadIdType threadId);

  private:
    IndexedUnaryFunctorImageFilter(const Self &); //purposely not implemented
    void operator=(const Self &);          //purposely not implemented

    FunctorType m_Functor;
  };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkIndexedUnaryFunctorImageFilter.tpp"
#endif

#endif
