/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHotspotMaskGenerator_h
#define mitkHotspotMaskGenerator_h

#include <itkObject.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkTimeStamp.h>
#include <stdexcept>
#include <MitkImageStatisticsExports.h>
#include <mitkImageTimeSelector.h>
#include <mitkMaskGenerator.h>


namespace mitk
{
/**
     * \brief Generates a spherical mask centered on the hotspot (brightest region) of an image.
     *
     * \warning Until T30375 is clarified, this class should be considered
     * deprecated/erroneous and should not be used.
     *
     * A hotspot is the spherical region of the image where the mean intensity
     * is maximal (the brightest spot). This is commonly used in PET scans.
     *
     * The identification of the hotspot works as follows:
     * -# A cubic (or circular in 2D) convolution kernel of the specified radius
     *    is generated.
     * -# The kernel is convolved with the input image in the Fourier domain.
     * -# The maximum value of the convolved image corresponds to the hotspot center.
     * -# A binary spherical mask is generated around that center.
     *
     * If an additional MaskGenerator is set, only pixels where the mask equals
     * the specified label value are considered as candidate hotspot centers.
     *
     * \sa MaskGenerator
     * \sa ImageStatisticsCalculator
     */
    class MITKIMAGESTATISTICS_EXPORT HotspotMaskGenerator: public MaskGenerator
    {
    public:
        /** Standard Self typedef */
        typedef HotspotMaskGenerator                Self;
        typedef MaskGenerator                       Superclass;
        typedef itk::SmartPointer< Self >           Pointer;
        typedef itk::SmartPointer< const Self >     ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self); /** Runtime information support. */
        itkTypeMacro(HotspotMaskGenerator, MaskGenerator);

        /**
         * \brief Returns the number of masks this generator provides.
         * \return Always returns 1.
         */
        unsigned int GetNumberOfMasks() const override;

        /**
         * \brief Set an optional mask generator to restrict the hotspot search area.
         * \param[in] _arg Pointer to a MaskGenerator, or nullptr to disable masking.
         */
        itkSetObjectMacro(Mask, MaskGenerator);

        /**
         * \brief Set/Get the radius of the hotspot sphere in millimeters.
         */
        itkGetConstMacro(HotspotRadiusInMM, double);
        itkSetMacro(HotspotRadiusInMM, double);

        /**
         * \brief Set/Get whether the hotspot sphere must be completely inside the image.
         *
         * When true, candidate hotspot centers are rejected if the sphere at that
         * center extends beyond the image boundary. Default is true.
         */
        itkGetConstMacro(HotspotMustBeCompletelyInsideImage, bool);
        itkSetMacro(HotspotMustBeCompletelyInsideImage, bool);

        /**
         * \brief Set the mask label value used to restrict the hotspot search.
         *
         * Only pixels in the convolved image where the corresponding mask
         * pixel equals this label value are considered as candidate hotspot
         * centers.
         * \param[in] _arg The label value to match.
         */
        itkSetMacro(Label, unsigned short);

    protected:
        HotspotMaskGenerator();

        ~HotspotMaskGenerator() override;

        Image::ConstPointer DoGetMask(unsigned int) override;

        class ImageExtrema
        {
        public:
          bool Defined;
          double Max;
          double Min;
          vnl_vector<int> MaxIndex;
          vnl_vector<int> MinIndex;

          ImageExtrema()
            :Defined(false)
            ,Max(itk::NumericTraits<double>::min())
            ,Min(itk::NumericTraits<double>::max())
          {
          }
        };

    private:
        /** \brief Returns size of convolution kernel depending on spacing and radius. */
        template <unsigned int VImageDimension>
        itk::Size<VImageDimension>
          CalculateConvolutionKernelSize(double spacing[VImageDimension], double radiusInMM);

        /** \brief Generates image of kernel which is needed for convolution. */
        template <unsigned int VImageDimension>
        itk::SmartPointer< itk::Image<float, VImageDimension> >
          GenerateHotspotSearchConvolutionKernel(double spacing[VImageDimension], double radiusInMM);

        /** \brief Convolves image with spherical kernel image. Used for hotspot calculation.   */
        template <typename TPixel, unsigned int VImageDimension>
        itk::SmartPointer< itk::Image<TPixel, VImageDimension> >
          GenerateConvolutionImage( const itk::Image<TPixel, VImageDimension>* inputImage );


        /** \brief Fills pixels of the spherical hotspot mask. */
        template < typename TPixel, unsigned int VImageDimension>
        void
          FillHotspotMaskPixels( itk::Image<TPixel, VImageDimension>* maskImage,
          itk::Point<double, VImageDimension> sphereCenter,
          double sphereRadiusInMM);


        /** \brief */
        template <typename TPixel, unsigned int VImageDimension>
        void
          CalculateHotspotMask(const itk::Image<TPixel, VImageDimension>* inputImage,
                               const itk::Image<unsigned short, VImageDimension>* maskImage,
                               unsigned int label);


        template <typename TPixel, unsigned int VImageDimension  >
        ImageExtrema CalculateExtremaWorld( const itk::Image<TPixel, VImageDimension>* inputImage,
                                                        const itk::Image<unsigned short, VImageDimension>* maskImage,
                                                        double necessaryDistanceToImageBorderInMM,
                                                        unsigned int label);

        bool IsUpdateRequired() const;

        HotspotMaskGenerator(const HotspotMaskGenerator &);
        HotspotMaskGenerator & operator=(const HotspotMaskGenerator &);

        MaskGenerator::Pointer m_Mask;
        mitk::Image::Pointer m_InternalMask;
        itk::Image<unsigned short, 2>::ConstPointer m_internalMask2D;
        itk::Image<unsigned short, 3>::ConstPointer m_internalMask3D;
        double m_HotspotRadiusInMM;
        bool m_HotspotMustBeCompletelyInsideImage;
        unsigned short m_Label;
        vnl_vector<int> m_ConvolutionImageMinIndex, m_ConvolutionImageMaxIndex;
        unsigned long m_InternalMaskUpdateTime;
    };
}
#endif
