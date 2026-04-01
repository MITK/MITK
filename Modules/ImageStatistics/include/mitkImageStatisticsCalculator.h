/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageStatisticsCalculator_h
#define mitkImageStatisticsCalculator_h

#include <MitkImageStatisticsExports.h>
#include <mitkImage.h>
#include <mitkMaskGenerator.h>
#include <mitkImageStatisticsContainer.h>

namespace mitk
{
    /**
     * \brief Computes image statistics, optionally restricted by one or two masks.
     *
     * ImageStatisticsCalculator is the central class for computing pixel statistics
     * on MITK images. It supports single images and time-series images, and can
     * restrict computation to regions defined by one or two MaskGenerator instances
     * (combined via pixel-wise AND).
     *
     * Computed statistics include: mean, standard deviation, variance, minimum,
     * maximum, RMS, skewness, kurtosis, MPP (mean of positive pixels), entropy,
     * uniformity, UPP, median, and a configurable histogram.
     *
     * Results are stored in an ImageStatisticsContainer that organizes statistics
     * by label value and time step.
     *
     * \sa ImageStatisticsContainer
     * \sa MaskGenerator
     * \sa HistogramStatisticsCalculator
     */
    class MITKIMAGESTATISTICS_EXPORT ImageStatisticsCalculator: public itk::Object
    {
    public:
        /** Standard Self typedef */
        typedef ImageStatisticsCalculator        Self;
        typedef itk::Object                         Superclass;
        typedef itk::SmartPointer< Self >           Pointer;
        typedef itk::SmartPointer< const Self >     ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self); /** Runtime information support. */
        itkTypeMacro(ImageStatisticsCalculator, itk::Object);

        /** \brief Type for individual statistics values. */
        typedef double statisticsValueType;
        /** \brief Map type associating statistic names with their values. */
        typedef std::map<std::string, statisticsValueType> statisticsMapType;
        /** \brief Histogram type used for pixel value distribution. */
        typedef itk::Statistics::Histogram<double> HistogramType;
        /** \brief Pixel type used for mask images. */
        typedef unsigned short MaskPixelType;
        /** \brief Type used for label indexing in statistics containers. */
        using LabelIndex = ImageStatisticsContainer::LabelValueType;

        /**
         * \brief Set the image for which statistics are to be computed.
         * \param[in] image Pointer to the input MITK image.
         * \pre image must not be nullptr.
         */
        void SetInputImage(const mitk::Image* image);

        /**
         * \brief Set the primary mask generator for restricting statistics computation.
         *
         * If no mask is desired, pass nullptr to clear the mask.
         *
         * \param[in] mask Pointer to a MaskGenerator, or nullptr.
         */
        void SetMask(mitk::MaskGenerator* mask);

        /**
         * \brief Set a secondary mask generator for additional region restriction.
         *
         * When set, the secondary mask is combined with the primary mask via
         * pixel-wise AND operation. The secondary mask does not need to be the
         * same size as the primary, but they must have some spatial overlap.
         *
         * \param[in] mask Pointer to a secondary MaskGenerator, or nullptr.
         */
        void SetSecondaryMask(mitk::MaskGenerator* mask);

        /**
         * \brief Set the number of histogram bins for histogram statistics.
         *
         * If SetBinSizeForHistogramStatistics() is called after this method,
         * bin size will take precedence over number of bins.
         *
         * \param[in] nBins Number of histogram bins. Default is 100.
         */
        void SetNBinsForHistogramStatistics(unsigned int nBins);

        /**
         * \brief Get the number of histogram bins.
         *
         * \return The currently configured number of bins. Note: this does not
         *         indicate whether NBins or BinSize is actually used; that depends
         *         on which was set last.
         */
        unsigned int GetNBinsForHistogramStatistics() const;

        /**
         * \brief Set the bin size for histogram statistics.
         *
         * If SetNBinsForHistogramStatistics() is called after this method,
         * number of bins will take precedence over bin size.
         *
         * \param[in] binSize The desired histogram bin width. Default is 10.0.
         */
        void SetBinSizeForHistogramStatistics(double binSize);

        /**
         * \brief Get the histogram bin size.
         *
         * \return The currently configured bin size. Note: this does not indicate
         *         whether NBins or BinSize is actually used; that depends on which
         *         was set last.
         */
        double GetBinSizeForHistogramStatistics() const;

        /**
         * \brief Get the computed statistics container.
         *
         * If statistics have not been computed yet or are out of date, this method
         * triggers recomputation before returning the results.
         *
         * \return Pointer to the ImageStatisticsContainer holding all results.
         */
        ImageStatisticsContainer* GetStatistics();

    protected:
        ImageStatisticsCalculator(){
            m_nBinsForHistogramStatistics = 100;
            m_binSizeForHistogramStatistics = 10;
            m_UseBinSizeOverNBins = false;
        };


    private:
        //Calculates statistics for each timestep for image
        template < typename TPixel, unsigned int VImageDimension >
        void InternalCalculateStatisticsUnmasked(const itk::Image< TPixel, VImageDimension >* image, TimeStepType timeStep);

        template < typename TPixel, unsigned int VImageDimension >
        void InternalCalculateStatisticsMasked(const itk::Image< TPixel, VImageDimension >* image, TimeStepType timeStep);

        template < typename TPixel, unsigned int VImageDimension >
        double GetVoxelVolume(const itk::Image<TPixel, VImageDimension>* image) const;

        bool IsUpdateRequired() const;

        mitk::Image::ConstPointer m_Image;
        mitk::Image::ConstPointer m_ImageTimeSlice;
        mitk::Image::ConstPointer m_InternalImageForStatistics;

        mitk::MaskGenerator::Pointer m_MaskGenerator;
        mitk::Image::ConstPointer m_InternalMask;

        mitk::MaskGenerator::Pointer m_SecondaryMaskGenerator;
        mitk::Image::ConstPointer m_SecondaryMask;

        unsigned int m_nBinsForHistogramStatistics;
        double m_binSizeForHistogramStatistics;
        bool m_UseBinSizeOverNBins;

        ImageStatisticsContainer::Pointer m_StatisticContainer;
    };

}
#endif
