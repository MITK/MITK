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


#include "mitkSimpleUnstructuredGridHistogram.h"

#include <mitkUnstructuredGrid.h>

#include <itkListSampleToHistogramFilter.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

namespace mitk {

SimpleUnstructuredGridHistogram::SimpleUnstructuredGridHistogram()
  : m_UGHistogram(HistogramType::New()), m_InvMaxFrequency(1)
{
  //MITK_INFO << "#######  Created a SimpleUnstructuredGridHistogram";
}

double SimpleUnstructuredGridHistogram::GetMin() const
{
  return m_UGHistogram->GetBinMin(0, 0);
}

double SimpleUnstructuredGridHistogram::GetMax() const
{
  return m_UGHistogram->GetBinMax(0, m_UGHistogram->GetSize(0)-1);
}

void SimpleUnstructuredGridHistogram::ComputeFromBaseData( BaseData* source )
{
  UnstructuredGrid* grid = dynamic_cast<UnstructuredGrid*>(source);
  //m_UGHistogram->Initialize(grid);

  vtkUnstructuredGrid* vtkUGrid = grid->GetVtkUnstructuredGrid();

  ListSampleType::Pointer listSample = ListSampleType::New();
  listSample->SetMeasurementVectorSize(1);

  MeasurementVectorType v;

  MeasurementVectorType lowerBound;
  MeasurementVectorType upperBound;

  int numberOfBins = 1;
  HistogramType::SizeType size;

  vtkDataArray* data;
  /*if (m_UsePointData)*/ data = vtkUGrid->GetPointData()->GetScalars();
  //else data = vtkUGrid->GetCellData()->GetScalars();

  if (data == 0)
  {
    listSample->Resize(1);
    v[0] = 0;
    listSample->PushBack(v);
    lowerBound[0] = 0;
    upperBound[0] = 0;
    size.Fill(numberOfBins);
  }
  else
  {
    listSample->Resize(data->GetNumberOfTuples());
    for (vtkIdType i = 0; i < data->GetNumberOfTuples(); ++i)
    {
      v[0] = data->GetComponent(i, 0);
      //if (v[0] != 0) MITK_INFO << "ug scalar: " << v[0];
      listSample->PushBack(v);
    }

    vtkIdType numberOfTuples = data->GetNumberOfTuples();

    if (numberOfTuples < 1000)
      numberOfBins = 250;
    else if (numberOfTuples < 30000)
      numberOfBins = 100;
    else if (numberOfTuples < 100000)
      numberOfBins = 50;
    else
      numberOfBins = 20;

    size.Fill(numberOfBins);

    double range[2];
    data->GetRange(range);
    lowerBound[0] = range[0];
    upperBound[0] = range[1];
  }

  m_UGHistogram->Initialize(size, lowerBound, upperBound);

  typedef itk::Statistics::ListSampleToHistogramFilter<ListSampleType, HistogramType> FilterType;
  FilterType::Pointer histoFilter = FilterType::New();

  histoFilter->SetListSample(listSample);
  histoFilter->SetHistogram(m_UGHistogram);
  histoFilter->Update();

  m_BinSize = (GetMax() - GetMin())/(double)numberOfBins;

  m_Mins = m_UGHistogram->GetMins();
  m_Maxs = m_UGHistogram->GetMaxs();

  HistogramType::FrequencyType maxFrequency = 0;
  HistogramType::SizeValueType histoSize = m_UGHistogram->GetSize(0);
  for (HistogramType::SizeValueType i = 0; i < histoSize; ++i)
  {
    HistogramType::FrequencyType f = m_UGHistogram->GetFrequency(i);
    if (f > maxFrequency)
    {
      maxFrequency = f;
    }
    //MITK_INFO << "Bin #" << i << ": " << m_UGHistogram->GetMeasurement(i,0);
  }

  if (maxFrequency)
  {
    m_InvMaxFrequency = 1.0/log((double)maxFrequency);
  }

  //MITK_INFO << "UGHistogramm size: " << m_UGHistogram->GetSize(0) << ", maxF: " << maxFrequency
  //    << " min count: " << m_Mins.size() << " max count: " << m_Maxs.size();
}

float SimpleUnstructuredGridHistogram::GetRelativeBin( double start, double end ) const
{


  //MITK_INFO << "GetRelativeBin start: " << start << ", end: " << end;
  HistogramType::FrequencyType maxf = 0;
  MeasurementVectorType v;
  for(double pos = start; pos < end; pos += m_BinSize)
  {
    v[0] = pos;
    HistogramType::FrequencyType f = m_UGHistogram->GetFrequency(m_UGHistogram->GetIndex(v));
    if (f > maxf) maxf = f;
  }

  return log(maxf)*m_InvMaxFrequency;
}

}
