/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkMITKScalarImageToHistogramGenerator_h
#define itkMITKScalarImageToHistogramGenerator_h

#include <itkImageToListSampleAdaptor.h>
#include <itkObject.h>
#include <itkSampleToHistogramFilter.h>

namespace itk
{
  namespace Statistics
  {
    /** \class MITKScalarImageToHistogramGenerator
     * \brief Generates a histogram from a scalar image.
     *
     * Adapts an ITK image into a list sample and uses SampleToHistogramFilter
     * to compute a histogram. This is a MITK-specific variant that works with
     * MITK image types.
     *
     * \tparam TImageType The input image type.
     * \tparam TMeasurementType The measurement type for histogram bins (defaults to pixel type).
     */
    template <class TImageType, class TMeasurementType = typename TImageType::PixelType>
    class MITKScalarImageToHistogramGenerator : public Object
    {
    public:
      /** \brief Standard typedefs. */
      typedef MITKScalarImageToHistogramGenerator Self;
      typedef Object Superclass;
      typedef SmartPointer<Self> Pointer;
      typedef SmartPointer<const Self> ConstPointer;

      /** Run-time type information (and related methods). */
      itkTypeMacro(MITKScalarImageToHistogramGenerator, Object);

      /** standard New() method support */
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

        typedef TImageType ImageType;
      typedef itk::Statistics::ImageToListSampleAdaptor<ImageType> AdaptorType;
      typedef typename AdaptorType::Pointer AdaptorPointer;
      typedef typename ImageType::PixelType PixelType;

      typedef itk::Statistics::Histogram<TMeasurementType, itk::Statistics::DenseFrequencyContainer2> HistogramType;
      typedef itk::Statistics::SampleToHistogramFilter<AdaptorType, HistogramType> GeneratorType;

      typedef typename GeneratorType::Pointer GeneratorPointer;

      typedef typename HistogramType::Pointer HistogramPointer;
      typedef typename HistogramType::ConstPointer HistogramConstPointer;

    public:
      /** \brief Trigger the computation of the histogram. */
      void Compute(void);

      /** \brief Connect the input image for which the histogram is going to be computed.
       * \param image The input scalar image.
       */
      void SetInput(const ImageType *image);

      /** \brief Return the computed histogram.
       *
       * \warning This output is only valid after the Compute() method has been invoked.
       * \return Pointer to the computed histogram.
       * \sa Compute
       */
      const HistogramType *GetOutput() const;

      /** \brief Set number of histogram bins.
       * \param numberOfBins The number of bins for the histogram.
       */
      void SetNumberOfBins(unsigned int numberOfBins);

      /** \brief Set the marginal scale value to be passed to the histogram generator.
       * \param marginalScale The marginal scale value.
       */
      void SetMarginalScale(double marginalScale);

    protected:
      /** \brief Default constructor. Creates the adaptor and histogram generator. */
      MITKScalarImageToHistogramGenerator();
      /** \brief Destructor. */
      ~MITKScalarImageToHistogramGenerator() override{};

      /** \brief Print the state of this object to an output stream. */
      void PrintSelf(std::ostream &os, Indent indent) const override;

    private:
      AdaptorPointer m_ImageToListAdaptor;

      GeneratorPointer m_HistogramGenerator;
    };

  } // end of namespace Statistics
} // end of namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkMITKScalarImageToHistogramGenerator.tpp>
#endif

#endif
