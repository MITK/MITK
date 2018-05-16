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

#include "mitkPeakImageSerializer.h"
#include "mitkPeakImage.h"
#include "itkImageFileWriter.h"
#include <mitkImageToItk.h>
#include <itksys/SystemTools.hxx>
#include <itkNrrdImageIO.h>

MITK_REGISTER_SERIALIZER(PeakImageSerializer)


mitk::PeakImageSerializer::PeakImageSerializer()
{
}


mitk::PeakImageSerializer::~PeakImageSerializer()
{
}


std::string mitk::PeakImageSerializer::Serialize()
{
  const PeakImage* image = dynamic_cast<const PeakImage*>( m_Data.GetPointer() );
  if (image == nullptr)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::PeakImage. Cannot serialize as Nrrd.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".peak";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    typedef mitk::ImageToItk< PeakImage::ItkPeakImageType > CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput(image);
    caster->Update();
    mitk::PeakImage::ItkPeakImageType::Pointer itk_image = caster->GetOutput();

    itk::ImageFileWriter< PeakImage::ItkPeakImageType >::Pointer writer = itk::ImageFileWriter< PeakImage::ItkPeakImageType >::New();
    writer->SetInput(itk_image);
    writer->SetFileName(fullname);
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();
    writer->SetImageIO(io);
    writer->Update();
  }
  catch (std::exception& e)
  {
    MITK_ERROR << " Error serializing object at " << (const void*) this->m_Data
              << " to "
              << fullname
              << ": "
              << e.what();
    return "";
  }
  return filename;
}

