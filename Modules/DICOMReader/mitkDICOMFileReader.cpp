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

#include "mitkDICOMFileReader.h"

#include <itkGDCMImageIO.h>

mitk::DICOMFileReader
::DICOMFileReader()
:itk::Object()
{
}

mitk::DICOMFileReader
::~DICOMFileReader()
{
}

mitk::DICOMFileReader
::DICOMFileReader(const DICOMFileReader& other )
:itk::Object()
,m_InputFilenames( other.m_InputFilenames )
,m_Outputs( other.m_Outputs )
,m_ConfigLabel( other.m_ConfigLabel )
,m_ConfigDescription( other.m_ConfigDescription )
{
}

mitk::DICOMFileReader&
mitk::DICOMFileReader
::operator=(const DICOMFileReader& other)
{
  if (this != &other)
  {
    m_InputFilenames = other.m_InputFilenames;
    m_Outputs = other.m_Outputs;
    m_ConfigLabel = other.m_ConfigLabel;
    m_ConfigDescription = other.m_ConfigDescription;
  }
  return *this;
}

void
mitk::DICOMFileReader
::SetConfigurationLabel(const std::string& label)
{
  m_ConfigLabel = label;
}

std::string
mitk::DICOMFileReader
::GetConfigurationLabel() const
{
  return m_ConfigLabel;
}

void
mitk::DICOMFileReader
::SetConfigurationDescription(const std::string& desc)
{
  m_ConfigDescription = desc;
}

std::string
mitk::DICOMFileReader
::GetConfigurationDescription() const
{
  return m_ConfigDescription;
}

void
mitk::DICOMFileReader
::SetInputFiles(StringList filenames)
{
  m_InputFilenames = filenames;
}

const mitk::StringList&
mitk::DICOMFileReader
::GetInputFiles() const
{
  return m_InputFilenames;
}

unsigned int
mitk::DICOMFileReader
::GetNumberOfOutputs() const
{
  return m_Outputs.size();
}

void
mitk::DICOMFileReader
::ClearOutputs()
{
  m_Outputs.clear();
}

void
mitk::DICOMFileReader
::SetNumberOfOutputs(unsigned int numberOfOutputs)
{
  m_Outputs.resize(numberOfOutputs);
}

void
mitk::DICOMFileReader
::SetOutput(unsigned int index, const mitk::DICOMImageBlockDescriptor& output)
{
  if (index < m_Outputs.size())
  {
    m_Outputs[index] = output;
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

void
mitk::DICOMFileReader
::PrintConfiguration(std::ostream& os) const
{
  os << "---- Configuration of " << this->GetNameOfClass() <<" " << (void*)this << " ----"<< std::endl;
  this->InternalPrintConfiguration(os);
  os << "---- End of configuration ----" << std::endl;
}


void
mitk::DICOMFileReader
::PrintOutputs(std::ostream& os, bool filenameDetails) const
{
  os << "---- Outputs of DICOMFilereader " << (void*)this << " ----"<< std::endl;

  for (unsigned int o = 0; o < m_Outputs.size(); ++o)
  {
    os << "-- Output " << o << std::endl;
    const DICOMImageBlockDescriptor& block = m_Outputs[o];
    block.Print(os, filenameDetails);
  }
  os << "---- End of output list ----" << std::endl;
}


const mitk::DICOMImageBlockDescriptor&
mitk::DICOMFileReader
::GetOutput(unsigned int index) const
{
  if (index < m_Outputs.size())
  {
    return m_Outputs[index];
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}
mitk::DICOMImageBlockDescriptor&
mitk::DICOMFileReader
::InternalGetOutput(unsigned int index)
{
  if (index < m_Outputs.size())
  {
    return m_Outputs[index];
  }
  else
  {
    std::stringstream ss;
    ss << "Index " << index << " out of range (" << m_Outputs.size() << " indices reserved)";
    throw std::invalid_argument( ss.str() );
  }
}

bool
mitk::DICOMFileReader
::IsDICOM(const std::string& filename)
{
  itk::GDCMImageIO::Pointer io = itk::GDCMImageIO::New();
  return io->CanReadFile( filename.c_str() );
}
