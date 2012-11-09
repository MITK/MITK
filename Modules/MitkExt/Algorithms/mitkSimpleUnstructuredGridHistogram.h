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


#ifndef MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H
#define MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H

#include <MitkExtExports.h>

#include "mitkSimpleHistogram.h"
//#include "mitkUnstructuredGridHistogram.h"

#include <itkListSample.h>

namespace mitk {

class MitkExt_EXPORT SimpleUnstructuredGridHistogram : public SimpleHistogram
{
public:
    SimpleUnstructuredGridHistogram();

    virtual double GetMin() const;
    virtual double GetMax() const;

    virtual void ComputeFromBaseData( BaseData* source );
    virtual float GetRelativeBin( double start, double end ) const;

private:

    typedef double MeasurementType;
    typedef itk::Vector<MeasurementType, 1> MeasurementVectorType;
    typedef itk::Statistics::ListSample<MeasurementVectorType> ListSampleType;

    typedef itk::Statistics::Histogram<MeasurementType, 1> HistogramType;

    HistogramType::Pointer m_UGHistogram;

    //UnstructuredGridHistogram::Pointer m_UGHistogram;

    HistogramType::BinMinContainerType m_Mins;
    HistogramType::BinMaxContainerType m_Maxs;

    double m_InvMaxFrequency;
    double m_BinSize;
};

}

#endif // MITKSIMPLEUNSTRUCTUREDGRIDHISTOGRAM_H
