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

#include "mitkOclDataSetFilter.h"
#include "mitkOclFilter.h"
#include "mitkOclDataSet.h"

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
  m_Input->CreateGPUBuffer(size, BpE);
  m_CurrentSize = size;
}

