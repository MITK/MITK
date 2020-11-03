/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKHISTOGRAMSTATISTICSCALCULATOR
#define MITKHISTOGRAMSTATISTICSCALCULATOR

#include <MitkImageStatisticsExports.h>
#include <itkHistogram.h>
#include <mitkImageStatisticsCalculator.h>


namespace mitk
{
/**
     * @brief Computes basic histogram statistics such as Uniformity, UPP (Uniformity of positive entries), Entropy and Median (approximation)
     */
    class MITKIMAGESTATISTICS_EXPORT HistogramStatisticsCalculator
    {
    public:
        typedef double MeasurementType;
        typedef itk::Statistics::Histogram<MeasurementType> HistogramType;

        HistogramStatisticsCalculator();

        /**
         * @brief SetHistogram requires a itk::Statistics::Histogram<double>
         */
        void SetHistogram(HistogramType::Pointer histogram);

        MeasurementType GetUPP();

        MeasurementType GetUniformity();

        MeasurementType GetEntropy();

        MeasurementType GetMedian();

        /**
         * @brief calculate statistics
         */
        void CalculateStatistics();

    protected:

    private:
        HistogramType::Pointer m_Histogram;
        MeasurementType m_Uniformity, m_UPP, m_Entropy, m_Median;
        bool m_StatisticsCalculated;
    };
}

#endif
