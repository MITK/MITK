/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKHOTSPOTCALCULATOR_H
#define MITKHOTSPOTCALCULATOR_H

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
     * @brief The HotspotMaskGenerator class is used when a hotspot has to be found in an image. A hotspot is
     * the region of the image where the mean intensity is maximal (=brightest spot). It is usually used in PET scans.
     * The identification of the hotspot is done as follows: First a cubic (or circular, if image is 2d)
     * mask of predefined size is generated. This mask is then convolved with the input image (in fourier domain).
     * The maximum value of the convolved image then corresponds to the hotspot.
     * If a maskGenerator is set, only the pixels of the convolved image where the corresponding mask is == @a label
     * are searched for the maximum value.
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
        @brief Set the input image. Required for this class
         */
        void SetInputImage(mitk::Image::Pointer inputImage);

        /**
        @brief Set a mask (can be nullptr if no mask is desired)
         */
        void SetMask(MaskGenerator::Pointer mask);

        /**
        @brief Set the radius of the hotspot (in MM)
         */
        void SetHotspotRadiusInMM(double radiusInMillimeter);

        const double& GetHotspotRadiusinMM() const;

        /**
        @brief Define whether the hotspot must be completely inside the image. Default is true
         */
        void SetHotspotMustBeCompletelyInsideImage(bool hotspotCompletelyInsideImage);

        bool GetHotspotMustBeCompletelyInsideImage() const;

        /**
        @brief If a maskGenerator is set, this detemines which mask value is used
         */
        void SetLabel(unsigned short label);

        /**
        @brief Computes and returns the hotspot mask. The hotspot mask has the same size as the input image. The hopspot has value 1, the remaining pixels are set to 0
         */
        mitk::Image::Pointer GetMask() override;

        /**
        @brief Returns the image index where the hotspot is located
         */
        vnl_vector<int> GetHotspotIndex();

        /**
        @brief Returns the index where the convolution image is minimal (darkest spot in image)
         */
        vnl_vector<int> GetConvolutionImageMinIndex();

        /**
         * @brief SetTimeStep is used to set the time step for which the mask is to be generated
         * @param timeStep
         */
        void SetTimeStep(unsigned int timeStep) override;

    protected:
        HotspotMaskGenerator();

        ~HotspotMaskGenerator() override;

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
          CalculateHotspotMask(itk::Image<TPixel, VImageDimension>* inputImage,
                               typename itk::Image<unsigned short, VImageDimension>::Pointer maskImage,
                               unsigned int label);


        template <typename TPixel, unsigned int VImageDimension  >
        ImageExtrema CalculateExtremaWorld( const itk::Image<TPixel, VImageDimension>* inputImage,
                                                        typename itk::Image<unsigned short, VImageDimension>::Pointer maskImage,
                                                        double neccessaryDistanceToImageBorderInMM,
                                                        unsigned int label);

        bool IsUpdateRequired() const;

        HotspotMaskGenerator(const HotspotMaskGenerator &);
        HotspotMaskGenerator & operator=(const HotspotMaskGenerator &);

        MaskGenerator::Pointer m_Mask;
        mitk::Image::Pointer m_internalImage;
        itk::Image<unsigned short, 2>::Pointer m_internalMask2D;
        itk::Image<unsigned short, 3>::Pointer m_internalMask3D;
        double m_HotspotRadiusinMM;
        bool m_HotspotMustBeCompletelyInsideImage;
        unsigned short m_Label;
        vnl_vector<int> m_ConvolutionImageMinIndex, m_ConvolutionImageMaxIndex;
        unsigned long m_InternalMaskUpdateTime;
    };
}
#endif // MITKHOTSPOTCALCULATOR


