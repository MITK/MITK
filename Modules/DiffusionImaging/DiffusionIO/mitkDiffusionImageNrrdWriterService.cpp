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

#ifndef __mitkDiffusionImageNrrdWriterService__cpp
#define __mitkDiffusionImageNrrdWriterService__cpp

#include "mitkDiffusionImageNrrdWriterService.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"
#include "mitkDiffusionIOMimeTypes.h"
#include "mitkImageCast.h"

#include <iostream>
#include <fstream>


mitk::DiffusionImageNrrdWriterService::DiffusionImageNrrdWriterService()
  : AbstractFileWriter(mitk::Image::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::DWI_NRRD_MIMETYPE() ), mitk::DiffusionIOMimeTypes::DWI_NRRD_MIMETYPE_DESCRIPTION())
{
  RegisterService();
}

mitk::DiffusionImageNrrdWriterService::DiffusionImageNrrdWriterService(const mitk::DiffusionImageNrrdWriterService& other)
  : AbstractFileWriter(other)
{
}

mitk::DiffusionImageNrrdWriterService::~DiffusionImageNrrdWriterService()
{}

void mitk::DiffusionImageNrrdWriterService::Write()
{
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image *>(this->GetInput());

  VectorImageType::Pointer itkImg;
  mitk::CastToItkImage(input,itkImg);

  if (input.IsNull())
  {
    MITK_ERROR <<"Sorry, input to DiffusionImageNrrdWriterService is NULL!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return ;
  }
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

  char keybuffer[512];
  char valbuffer[512];

  //itk::MetaDataDictionary dic = input->GetImage()->GetMetaDataDictionary();

  vnl_matrix_fixed<double,3,3> measurementFrame = mitk::DiffusionPropertyHelper::GetMeasurementFrame(input);
  if (measurementFrame(0,0) || measurementFrame(0,1) || measurementFrame(0,2) ||
    measurementFrame(1,0) || measurementFrame(1,1) || measurementFrame(1,2) ||
    measurementFrame(2,0) || measurementFrame(2,1) || measurementFrame(2,2))
  {
    sprintf( valbuffer, " (%lf,%lf,%lf) (%lf,%lf,%lf) (%lf,%lf,%lf)", measurementFrame(0,0), measurementFrame(0,1), measurementFrame(0,2), measurementFrame(1,0), measurementFrame(1,1), measurementFrame(1,2), measurementFrame(2,0), measurementFrame(2,1), measurementFrame(2,2));
    itk::EncapsulateMetaData<std::string>(itkImg->GetMetaDataDictionary(),std::string("measurement frame"),std::string(valbuffer));
  }

  sprintf( valbuffer, "DWMRI");
  itk::EncapsulateMetaData<std::string>(itkImg->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

  if(mitk::DiffusionPropertyHelper::GetGradientContainer(input)->Size())
  {
    sprintf( valbuffer, "%1f", mitk::DiffusionPropertyHelper::GetReferenceBValue(input) );
    itk::EncapsulateMetaData<std::string>(itkImg->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
  }

  for(unsigned int i=0; i<mitk::DiffusionPropertyHelper::GetGradientContainer(input)->Size(); i++)
  {
    sprintf( keybuffer, "DWMRI_gradient_%04d", i );

    /*if(itk::ExposeMetaData<std::string>(input->GetMetaDataDictionary(),
    std::string(keybuffer),tmp))
    continue;*/

    sprintf( valbuffer, "%1f %1f %1f", mitk::DiffusionPropertyHelper::GetGradientContainer(input)->ElementAt(i).get(0),
      mitk::DiffusionPropertyHelper::GetGradientContainer(input)->ElementAt(i).get(1), mitk::DiffusionPropertyHelper::GetGradientContainer(input)->ElementAt(i).get(2));

    itk::EncapsulateMetaData<std::string>(itkImg->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
  }

  typedef itk::VectorImage<short,3> ImageType;

  std::string ext = this->GetMimeType()->GetExtension(this->GetOutputLocation());
  ext = itksys::SystemTools::LowerCase(ext);

  // default extension is .dwi
  if( ext == "")
  {
    ext = ".nrrd";
    this->SetOutputLocation(this->GetOutputLocation() + ext);
  }

  if (ext == ".hdwi" || ext == ".nrrd" || ext == ".dwi")
  {

    MITK_INFO << "Extension " << ext;
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    //io->SetNrrdVectorType( nrrdKindList );
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();

    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput( itkImg );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName(this->GetOutputLocation());
    nrrdWriter->UseCompressionOn();
    nrrdWriter->SetImageIO(io);
    try
    {
      nrrdWriter->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
      throw;
    }

  }

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    MITK_INFO << "Could not reset locale " << currLocale;
  }
}

mitk::DiffusionImageNrrdWriterService* mitk::DiffusionImageNrrdWriterService::Clone() const
{
  return new DiffusionImageNrrdWriterService(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::DiffusionImageNrrdWriterService::GetConfidenceLevel() const
{
  mitk::Image::ConstPointer input = dynamic_cast<const mitk::Image*>(this->GetInput());
  if (input.IsNull() || !mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage( input ) )
  {
    return Unsupported;
  }
  else
  {
    return Supported;
  }
}

#endif //__mitkDiffusionImageNrrdWriterService__cpp
