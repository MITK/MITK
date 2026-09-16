/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHistogramStatisticsCalculator_h
#define mitkHistogramStatisticsCalculator_h

#include <MitkImageStatisticsExports.h>
#include <itkHistogram.h>
#include <mitkImageStatisticsCalculator.h>


namespace mitk
{
/**
     * \brief Computes histogram-based statistics: Uniformity, UPP, and Entropy.
     *
     * This class takes an itk::Statistics::Histogram<double> as input and computes
     * the following statistics from it:
     * - Uniformity: sum of squared bin frequencies (normalized).
     * - UPP (Uniformity of Positive Pixels): uniformity computed only over bins
     *   with positive measurement values.
     * - Entropy: Shannon entropy of the histogram distribution.
     *
     * Call SetHistogram() followed by CalculateStatistics() before querying results.
     *
     * \sa ImageStatisticsCalculator
     * \sa ImageStatisticsContainer
     */
    class MITKIMAGESTATISTICS_EXPORT HistogramStatisticsCalculator
    {
    public:
        /** \brief Measurement type used for histogram bins and statistics values. */
        typedef double MeasurementType;
        /** \brief Histogram type used as input. */
        typedef itk::Statistics::Histogram<MeasurementType> HistogramType;

        /** \brief Default constructor. Initializes all statistics to zero. */
        HistogramStatisticsCalculator();

        /**
         * \brief Set the histogram from which to compute statistics.
         * \param[in] histogram Pointer to an itk::Statistics::Histogram<double>.
         * \pre histogram must not be nullptr.
         */
        void SetHistogram(HistogramType::Pointer histogram);

        /**
         * \brief Get the Uniformity of Positive Pixels (UPP).
         * \return The UPP value computed from the histogram.
         * \pre CalculateStatistics() must have been called.
         */
        MeasurementType GetUPP();

        /**
         * \brief Get the Uniformity of the histogram distribution.
         * \return The Uniformity value (sum of squared normalized bin frequencies).
         * \pre CalculateStatistics() must have been called.
         */
        MeasurementType GetUniformity();

        /**
         * \brief Get the Shannon entropy of the histogram distribution.
         * \return The Entropy value.
         * \pre CalculateStatistics() must have been called.
         */
        MeasurementType GetEntropy();

        /**
         * \brief Compute all histogram statistics (Uniformity, UPP, Entropy).
         * \pre A histogram must have been set via SetHistogram().
         */
        void CalculateStatistics();

    protected:

    private:
        HistogramType::Pointer m_Histogram;
        MeasurementType m_Uniformity, m_UPP, m_Entropy;
        bool m_StatisticsCalculated;
    };
}

#endif
