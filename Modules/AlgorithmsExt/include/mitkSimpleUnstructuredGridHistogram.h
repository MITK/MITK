/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H
#define MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H

#include <MitkAlgorithmsExtExports.h>

#include "mitkSimpleHistogram.h"

#include <itkListSample.h>

namespace mitk
{
  class MITKALGORITHMSEXT_EXPORT SimpleUnstructuredGridHistogram : public SimpleHistogram
  {
  public:
    SimpleUnstructuredGridHistogram();

    double GetMin() const override;
    double GetMax() const override;

    void ComputeFromBaseData(BaseData *source) override;
    float GetRelativeBin(double start, double end) const override;

  private:
    typedef double MeasurementType;
    typedef itk::Vector<MeasurementType, 1> MeasurementVectorType;
    typedef itk::Statistics::ListSample<MeasurementVectorType> ListSampleType;

    typedef itk::Statistics::Histogram<MeasurementType> HistogramType;

    HistogramType::ConstPointer m_UGHistogram;

    // UnstructuredGridHistogram::Pointer m_UGHistogram;

    HistogramType::BinMinContainerType m_Mins;
    HistogramType::BinMaxContainerType m_Maxs;

    double m_InvMaxFrequency;
    double m_BinSize;
  };
}

#endif // MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H
