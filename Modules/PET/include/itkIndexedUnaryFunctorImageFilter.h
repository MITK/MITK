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

#ifndef itkIndexedUnaryFunctorImageFilter_h
#define itkIndexedUnaryFunctorImageFilter_h

#include <itkUnaryFunctorImageFilter.h>

namespace itk
{
/**
 * \class IndexedUnaryFunctorImageFilter
 * \brief Perform a generic pixel-wise, index-aware operation on an input image to produce an output image.
 *
 * This filter is similar to itk::UnaryFunctorImageFilter, but it passes both the pixel value
 * and its image index to the functor. This is useful for operations where the spatial position
 * of a pixel within the image is relevant for computing the output (e.g., spatially varying
 * decay correction in PET SUV calculations).
 *
 * \par Why a custom filter?
 *
 * No upstream ITK filter exposes "(pixel value, image index) -> output pixel" directly.
 * itk::UnaryFunctorImageFilter passes only the pixel value; itk::UnaryGeneratorImageFilter
 * is region-based (functor signature is (input, region, threadId, ProgressReporter)) and
 * not pixel-based. Hand-rolling around itk::ImageRegionConstIteratorWithIndex /
 * itk::ImageRegionIterator -- which is what this filter does -- is the standard way to
 * close that gap.
 *
 * \tparam TInputImage  Type of the input image.
 * \tparam TOutputImage Type of the output image.
 * \tparam TFunction    Functor type. Must define \c operator()(const InputPixelType&, const IndexType&)
 *                      and \c operator!=().
 *
 * The functor is called for each pixel as:
 * \code
 * output[index] = functor(input[index], index);
 * \endcode
 *
 * \sa itk::UnaryFunctorImageFilter, itk::UnaryGeneratorImageFilter, SUVbwFunctorPolicy
 */

  template< typename TInputImage, typename TOutputImage, typename TFunction >
  class IndexedUnaryFunctorImageFilter :public InPlaceImageFilter< TInputImage, TOutputImage >
  {
  public:
    /** \brief Standard class typedefs. */
    typedef IndexedUnaryFunctorImageFilter                         Self;
    typedef InPlaceImageFilter< TInputImage, TOutputImage > Superclass;
    typedef SmartPointer< Self >                            Pointer;
    typedef SmartPointer< const Self >                      ConstPointer;

    /** \brief Method for creation through the object factory. */
    itkNewMacro(Self);

    /** \brief Run-time type information (and related methods). */
    itkTypeMacro(IndexedUnaryFunctorImageFilter, InPlaceImageFilter);

    /** \brief Type of the functor used for the pixel-wise operation. */
    typedef TFunction FunctorType;

    typedef TInputImage                              InputImageType;     ///< \brief Input image type.
    typedef typename    InputImageType::ConstPointer InputImagePointer;  ///< \brief Input image const pointer type.
    typedef typename    InputImageType::RegionType   InputImageRegionType; ///< \brief Input image region type.
    typedef typename    InputImageType::PixelType    InputImagePixelType;  ///< \brief Input pixel type.

    typedef TOutputImage                             OutputImageType;     ///< \brief Output image type.
    typedef typename     OutputImageType::Pointer    OutputImagePointer;  ///< \brief Output image pointer type.
    typedef typename     OutputImageType::RegionType OutputImageRegionType; ///< \brief Output image region type.
    typedef typename     OutputImageType::PixelType  OutputImagePixelType;  ///< \brief Output pixel type.

    /**
     * \brief Get a mutable reference to the functor object.
     *
     * The functor is returned by reference because functors do not have to derive
     * from itk::LightObject and thus may not have a reference count.
     *
     * \return Mutable reference to the functor.
     */
    FunctorType &       GetFunctor() { return m_Functor; }

    /**
     * \brief Get a const reference to the functor object.
     *
     * \return Const reference to the functor.
     */
    const FunctorType & GetFunctor() const { return m_Functor; }

    /**
     * \brief Set the functor object.
     *
     * Replaces the current functor with a copy of the specified functor. This allows
     * configuring functor parameters (e.g., injected activity, body weight) before
     * running the filter. Requires \c operator!=() to be defined on the functor type.
     *
     * \param[in] functor The functor to copy into this filter.
     */
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
    void GenerateOutputInformation() override;

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
      ThreadIdType threadId) override;

  private:
    IndexedUnaryFunctorImageFilter(const Self &); //purposely not implemented
    void operator=(const Self &);          //purposely not implemented

    FunctorType m_Functor;
  };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkIndexedUnaryFunctorImageFilter.tpp>
#endif

#endif
