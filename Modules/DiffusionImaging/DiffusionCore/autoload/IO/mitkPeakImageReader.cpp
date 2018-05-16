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

#include "mitkPeakImageReader.h"
#include <mitkCustomMimeType.h>
#include <mitkDiffusionCoreIOMimeTypes.h>
#include <mitkPeakImage.h>
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itkImageFileReader.h>
#include <itksys/SystemTools.hxx>
#include <itkNiftiImageIO.h>
#include <itkNrrdImageIO.h>
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>

namespace mitk
{

  PeakImageReader::PeakImageReader(const PeakImageReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  PeakImageReader::PeakImageReader()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::PEAK_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::PEAK_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  PeakImageReader::~PeakImageReader()
  {
  }

  std::vector<itk::SmartPointer<BaseData> > PeakImageReader::Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    std::string location = GetInputLocation();
    std::string ext = itksys::SystemTools::GetFilenameExtension(location);

    typedef itk::ImageFileReader<PeakImage::ItkPeakImageType> FileReaderType;
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(GetInputLocation());
    if (ext==".peak")
    {
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
      reader->SetImageIO(io);
    }
    reader->Update();

    Image::Pointer resultImage = dynamic_cast<Image*>(PeakImage::New().GetPointer());
    mitk::CastToMitkImage(reader->GetOutput(), resultImage);
    resultImage->SetVolume(reader->GetOutput()->GetBufferPointer());

    StringProperty::Pointer nameProp;
    nameProp = StringProperty::New(itksys::SystemTools::GetFilenameWithoutExtension(GetInputLocation()));
    resultImage->SetProperty("name", nameProp);
    dynamic_cast<PeakImage*>(resultImage.GetPointer())->ConstructPolydata();

    result.push_back( resultImage.GetPointer() );

    return result;
  }

} //namespace MITK

mitk::PeakImageReader* mitk::PeakImageReader::Clone() const
{
  return new PeakImageReader(*this);
}
