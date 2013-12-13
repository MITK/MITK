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
:itk::LightObject()
{
}

mitk::DICOMFileReader
::~DICOMFileReader()
{
}

mitk::DICOMFileReader
::DICOMFileReader(const DICOMFileReader& other )
:itk::LightObject()
,m_Outputs( other.m_Outputs ) // TODO copy instead of reference!
{
}

mitk::DICOMFileReader&
mitk::DICOMFileReader
::operator=(const DICOMFileReader& other)
{
  if (this != &other)
  {
    m_InputFilenames = other.m_InputFilenames;
    m_Outputs = other.m_Outputs; // TODO copy instead of reference!
  }
  return *this;
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
::PrintOutputs(std::ostream& os, bool filenameDetails)
{
  os << "---- Outputs of DICOMFilereader " << (void*)this << "----"<< std::endl;

  for (unsigned int o = 0; o < m_Outputs.size(); ++o)
  {
    os << "-- Output " << o << std::endl;
    const DICOMImageBlockDescriptor& block = m_Outputs[o];
    const DICOMImageFrameList& frames = block.GetImageFrameList();
    os << "  Number of frames: " << frames.size() << std::endl;
    os << "  Pixels interpolated: " << (block.GetPixelsInterpolated() ? "true" : "false") << std::endl;
    os << "  Pixel spacing interpretation: " << (int)block.GetPixelSpacingInterpretation() << std::endl;
    os << "  MITK image: " << (void*)block.GetMitkImage().GetPointer() << std::endl;
    if (filenameDetails)
    {
      for (DICOMImageFrameList::const_iterator frameIter = frames.begin();
          frameIter != frames.end();
          ++frameIter)
      {
        os << "    " << (*frameIter)->Filename;
        if ((*frameIter)->FrameNo > 0)
        {
          os << ", " << (*frameIter)->FrameNo;
        }
        os << std::endl;
      }
    }
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
