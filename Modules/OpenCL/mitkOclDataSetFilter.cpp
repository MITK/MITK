/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkOclDataSetFilter.h"
#include "mitkOclFilter.h"
#include "mitkOclDataSet.h"
#include "mitkImageReadAccessor.h"

mitk::OclDataSetFilter::OclDataSetFilter()
{
}

mitk::OclDataSetFilter::~OclDataSetFilter()
{
}

void mitk::OclDataSetFilter::SetInput(mitk::OclDataSet::Pointer DataSet)
{
  m_Input = DataSet;
}

void mitk::OclDataSetFilter::SetInput(void* DataSet, unsigned int size, unsigned int BpE)
{
  m_Input = mitk::OclDataSet::New();
  m_Input->SetData(DataSet);
  m_Input->SetBufferSize(size);
  m_Input->SetBpE(BpE);
  m_CurrentSize = BpE;
}

void mitk::OclDataSetFilter::SetInput(mitk::Image::Pointer image)
{
  m_Input = mitk::OclDataSet::New();
  mitk::ImageReadAccessor reader(image);

  m_Input->SetData(const_cast<void*>(reader.GetData()));
  m_CurrentSize = (unsigned int)(image->GetPixelType().GetBitsPerComponent() / 8);
  unsigned int elements = image->GetDimension(0) * image->GetDimension(1) * image->GetDimension(2);

  m_Input->SetBufferSize(elements);
  m_Input->SetBpE(m_CurrentSize);
}
