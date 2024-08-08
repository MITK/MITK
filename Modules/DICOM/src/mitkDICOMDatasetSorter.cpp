/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMDatasetSorter.h"

mitk::DICOMDatasetSorter
::DICOMDatasetSorter()
:itk::LightObject()
{
}

mitk::DICOMDatasetSorter
::~DICOMDatasetSorter()
{
}

mitk::DICOMDatasetSorter
::DICOMDatasetSorter(const DICOMDatasetSorter& other )
:itk::LightObject()
,m_Outputs( other.m_Outputs )
{
}

mitk::DICOMDatasetSorter&
mitk::DICOMDatasetSorter
::operator=(const DICOMDatasetSorter& other)
{
  if (this != &other)
  {
    m_Input = other.m_Input;
    m_Outputs = other.m_Outputs;
  }
  return *this;
}

void
mitk::DICOMDatasetSorter
::SetInput(DICOMDatasetList datasets)
{
  m_Input = datasets;
}

const mitk::DICOMDatasetList&
mitk::DICOMDatasetSorter
::GetInput() const
{
  return m_Input;
}

unsigned int
mitk::DICOMDatasetSorter
::GetNumberOfOutputs() const
{
  return m_Outputs.size();
}

void
mitk::DICOMDatasetSorter
::ClearOutputs()
{
  m_Outputs.clear();
  m_SplitReasons.clear();
}

void
mitk::DICOMDatasetSorter
::SetNumberOfOutputs(unsigned int numberOfOutputs)
{
  m_Outputs.resize(numberOfOutputs);
  m_SplitReasons.resize(numberOfOutputs);
}

void
mitk::DICOMDatasetSorter
::SetOutput(unsigned int index, const DICOMDatasetList& output, IOVolumeSplitReason::ConstPointer splitReason)
{
  if (index < m_Outputs.size())
  {
    m_Outputs[index] = output;
    m_SplitReasons[index] = (nullptr == splitReason) ? IOVolumeSplitReason::New() : splitReason->Clone();
  }
  else
  {
    std::stringstream ss;
    ss << "Cannot get output. Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

const mitk::DICOMDatasetList&
mitk::DICOMDatasetSorter
::GetOutput(unsigned int index) const
{
  return const_cast<DICOMDatasetSorter*>(this)->GetOutput(index);
}

mitk::DICOMDatasetList&
mitk::DICOMDatasetSorter
::GetOutput(unsigned int index)
{
  if (index < m_Outputs.size())
  {
    return m_Outputs[index];
  }
  else
  {
    std::stringstream ss;
    ss << "Cannot get output. Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

mitk::IOVolumeSplitReason::ConstPointer
mitk::DICOMDatasetSorter
::GetSplitReason(unsigned int index) const
{
  if (index >= m_Outputs.size())
  {
    std::stringstream ss;
    ss << "Cannot get split reason. Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument(ss.str());
  }

  return m_SplitReasons[index];
}
