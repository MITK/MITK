/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkHistogramStatisticsCalculator.h>
#include <stdexcept>

namespace mitk {

HistogramStatisticsCalculator::HistogramStatisticsCalculator():
    m_Uniformity(0), m_UPP(0), m_Entropy(0), m_StatisticsCalculated(false)
{

}

void HistogramStatisticsCalculator::SetHistogram(HistogramType::Pointer histogram)
{
    if (m_Histogram != histogram)
    {
        m_Histogram = histogram;
        m_StatisticsCalculated = false;
    }
}

HistogramStatisticsCalculator::MeasurementType HistogramStatisticsCalculator::GetEntropy()
{
    if (!m_StatisticsCalculated)
    {
        MITK_WARN << "Statistics have not yet been calculated, running calculation now...";
        CalculateStatistics();
    }
    return m_Entropy;
}

HistogramStatisticsCalculator::MeasurementType HistogramStatisticsCalculator::GetUniformity()
{
    if (!m_StatisticsCalculated)
    {
        MITK_WARN << "Statistics have not yet been calculated, running calculation now...";
        CalculateStatistics();
    }
    return m_Uniformity;
}

HistogramStatisticsCalculator::MeasurementType HistogramStatisticsCalculator::GetUPP()
{
    if (!m_StatisticsCalculated)
    {
        MITK_WARN << "Statistics have not yet been calculated, running calculation now...";
        CalculateStatistics();
    }
    return m_UPP;
}

void HistogramStatisticsCalculator::CalculateStatistics()
{
    if (m_Histogram.IsNull())
    {
        throw std::runtime_error("Histogram not set in HistogramStatisticsCalculator::CalculateStatistics()");
    }

    unsigned int nBins = m_Histogram->GetSize()[0];
    m_Uniformity = 0;
    m_Entropy = 0;
    m_UPP = 0;

    for (unsigned int i = 0; i < nBins; i++)
    {
      MeasurementType partialProbability = m_Histogram->GetFrequency(i, 0) / double( m_Histogram->GetTotalFrequency() );

        if (partialProbability != 0)
        {
            m_Entropy -= partialProbability * (std::log2( partialProbability ));
            m_Uniformity += std::pow(partialProbability, 2);

            if (m_Histogram->GetMeasurement(i, 0) > 0)
            {
                m_UPP += std::pow(partialProbability, 2);
            }

        }
    }
    m_StatisticsCalculated = true;
}

}

