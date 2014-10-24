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

#ifndef __mitkNrrdDiffusionImageWriter__cpp
#define __mitkNrrdDiffusionImageWriter__cpp

#include "mitkNrrdDiffusionImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"
#include "mitkDiffusionIOMimeTypes.h"

#include <iostream>
#include <fstream>


mitk::NrrdDiffusionImageWriter::NrrdDiffusionImageWriter()
  : AbstractFileWriter(mitk::DiffusionImage<short>::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::DWI_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::DWI_MIMETYPE_DESCRIPTION())
{
  RegisterService();
}

mitk::NrrdDiffusionImageWriter::NrrdDiffusionImageWriter(const mitk::NrrdDiffusionImageWriter& other)
  : AbstractFileWriter(other)
{
}

mitk::NrrdDiffusionImageWriter::~NrrdDiffusionImageWriter()
{}

void mitk::NrrdDiffusionImageWriter::Write()
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull())
  {
    MITK_ERROR <<"Sorry, input to NrrdDiffusionImageWriter is NULL!";
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

  vnl_matrix_fixed<double,3,3> measurementFrame = input->GetMeasurementFrame();
  if (measurementFrame(0,0) || measurementFrame(0,1) || measurementFrame(0,2) ||
    measurementFrame(1,0) || measurementFrame(1,1) || measurementFrame(1,2) ||
    measurementFrame(2,0) || measurementFrame(2,1) || measurementFrame(2,2))
  {
    sprintf( valbuffer, " (%lf,%lf,%lf) (%lf,%lf,%lf) (%lf,%lf,%lf)", measurementFrame(0,0), measurementFrame(0,1), measurementFrame(0,2), measurementFrame(1,0), measurementFrame(1,1), measurementFrame(1,2), measurementFrame(2,0), measurementFrame(2,1), measurementFrame(2,2));
    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("measurement frame"),std::string(valbuffer));
  }

  sprintf( valbuffer, "DWMRI");
  itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

  if(input->GetDirections()->Size())
  {
    sprintf( valbuffer, "%1f", input->GetReferenceBValue() );
    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
  }

  for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
  {
    sprintf( keybuffer, "DWMRI_gradient_%04d", i );

    /*if(itk::ExposeMetaData<std::string>(input->GetMetaDataDictionary(),
    std::string(keybuffer),tmp))
    continue;*/

    sprintf( valbuffer, "%1f %1f %1f", input->GetDirections()->ElementAt(i).get(0),
      input->GetDirections()->ElementAt(i).get(1), input->GetDirections()->ElementAt(i).get(2));

    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
  }

  typedef itk::VectorImage<short,3> ImageType;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetOutputLocation());
  ext = itksys::SystemTools::LowerCase(ext);

  // default extension is .dwi
  if( ext == "")
  {
    ext = ".dwi";
    this->SetOutputLocation(this->GetOutputLocation() + ext);
  }

  if (ext == ".hdwi" || ext == ".dwi")
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    //io->SetNrrdVectorType( nrrdKindList );
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();

    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput( input->GetVectorImage() );
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
  else if (ext == ".fsl" || ext == ".fslgz")
  {
    MITK_INFO << "Writing Nifti-Image for FSL";
    ImageType::Pointer vecimg = input->GetVectorImage();

    typedef itk::Image<short,4> ImageType4D;
    ImageType4D::Pointer img4 = ImageType4D::New();

    ImageType::SpacingType spacing = vecimg->GetSpacing();
    ImageType4D::SpacingType spacing4;
    for(int i=0; i<3; i++)
      spacing4[i] = spacing[i];
    spacing4[3] = 1;
    img4->SetSpacing( spacing4 );   // Set the image spacing

    ImageType::PointType origin = vecimg->GetOrigin();
    ImageType4D::PointType origin4;
    for(int i=0; i<3; i++)
      origin4[i] = origin[i];
    origin4[3] = 0;
    img4->SetOrigin( origin4 );     // Set the image origin

    ImageType::DirectionType direction = vecimg->GetDirection();
    ImageType4D::DirectionType direction4;
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        direction4[i][j] = direction[i][j];
    for(int i=0; i<4; i++)
      direction4[i][3] = 0;
    for(int i=0; i<4; i++)
      direction4[3][i] = 0;
    direction4[3][3] = 1;
    img4->SetDirection( direction4 );  // Set the image direction

    ImageType::RegionType region = vecimg->GetLargestPossibleRegion();
    ImageType4D::RegionType region4;

    ImageType::RegionType::SizeType size = region.GetSize();
    ImageType4D::RegionType::SizeType size4;

    for(int i=0; i<3; i++)
      size4[i] = size[i];
    size4[3] = vecimg->GetVectorLength();

    ImageType::RegionType::IndexType index = region.GetIndex();
    ImageType4D::RegionType::IndexType index4;
    for(int i=0; i<3; i++)
      index4[i] = index[i];
    index4[3] = 0;

    region4.SetSize(size4);
    region4.SetIndex(index4);
    img4->SetRegions( region4 );

    img4->Allocate();

    itk::ImageRegionIterator<ImageType>   it (vecimg, vecimg->GetLargestPossibleRegion() );
    typedef ImageType::PixelType VecPixType;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      ImageType::IndexType currentIndex = it.GetIndex();
      for(unsigned int ind=0; ind<vec.Size(); ind++)
      {

        for(int i=0; i<3; i++)
          index4[i] = currentIndex[i];
        index4[3] = ind;
        img4->SetPixel(index4, vec[ind]);
      }
    }

    // create copy of file with correct ending for mitk
    std::string fname3 = this->GetOutputLocation();
    std::string::iterator itend = fname3.end();
    if (ext == ".fsl")
      fname3.replace( itend-3, itend, "nii");
    else
      fname3.replace( itend-5, itend, "nii.gz");

    itk::NiftiImageIO::Pointer io4 = itk::NiftiImageIO::New();

    typedef itk::VectorImage<short,3> ImageType;
    typedef itk::ImageFileWriter<ImageType4D> WriterType4;
    WriterType4::Pointer nrrdWriter4 = WriterType4::New();
    nrrdWriter4->UseInputMetaDataDictionaryOn();
    nrrdWriter4->SetInput( img4 );
    nrrdWriter4->SetFileName(fname3);
    nrrdWriter4->UseCompressionOn();
    nrrdWriter4->SetImageIO(io4);
    try
    {
      nrrdWriter4->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
      throw;
    }

    itksys::SystemTools::CopyAFile(fname3.c_str(), this->GetOutputLocation().c_str());

    if(input->GetDirections()->Size())
    {
      std::ofstream myfile;
      std::string fname = this->GetOutputLocation();
      fname += ".bvals";
      myfile.open (fname.c_str());
      for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
      {
        double twonorm = input->GetDirections()->ElementAt(i).two_norm();
        myfile << input->GetReferenceBValue()*twonorm*twonorm << " ";
      }
      myfile.close();

      std::ofstream myfile2;
      std::string fname2 = this->GetOutputLocation();
      fname2 += ".bvecs";
      myfile2.open (fname2.c_str());
      for(int j=0; j<3; j++)
      {
        for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
        {
          //need to modify the length
          mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer grads = input->GetDirections();
          mitk::DiffusionImage<short>::GradientDirectionType direction = grads->ElementAt(i);
          direction.normalize();
          myfile2 << direction.get(j) << " ";
          //myfile2 << input->GetDirections()->ElementAt(i).get(j) << " ";
        }
        myfile2 << std::endl;
      }

      std::ofstream myfile3;
      std::string fname4 = this->GetOutputLocation();
      fname4 += ".ttk";
      myfile3.open (fname4.c_str());
      for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
      {
        for(int j=0; j<3; j++)
        {
          myfile3 << input->GetDirections()->ElementAt(i).get(j) << " ";
        }
        myfile3 << std::endl;
      }
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

mitk::NrrdDiffusionImageWriter* mitk::NrrdDiffusionImageWriter::Clone() const
{
  return new NrrdDiffusionImageWriter(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::NrrdDiffusionImageWriter::GetConfidenceLevel() const
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull() )
  {
    return Unsupported;
  }
  else
  {
    return Supported;
  }
}

#endif //__mitkNrrdDiffusionImageWriter__cpp
