/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-12-10 18:05:13 +0100 (Mi, 10 Dez 2008) $
Version:   $Revision: 15922 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkNrrdDiffusionImageWriter__cpp
#define __mitkNrrdDiffusionImageWriter__cpp

#include "mitkNrrdDiffusionImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

template<typename TPixelType>
mitk::NrrdDiffusionImageWriter<TPixelType>::NrrdDiffusionImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template<typename TPixelType>
mitk::NrrdDiffusionImageWriter<TPixelType>::~NrrdDiffusionImageWriter()
{}

template<typename TPixelType>
void mitk::NrrdDiffusionImageWriter<TPixelType>::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdDiffusionImageWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
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

  if(input->GetOriginalDirections()->Size())
  {
    sprintf( valbuffer, "%1f", input->GetB_Value() );
    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string("DWMRI_b-value"),std::string(valbuffer));
  }

  for(unsigned int i=0; i<input->GetOriginalDirections()->Size(); i++)
  {
    sprintf( keybuffer, "DWMRI_gradient_%04d", i );

    /*if(itk::ExposeMetaData<std::string>(input->GetMetaDataDictionary(),
      std::string(keybuffer),tmp))
      continue;*/

    sprintf( valbuffer, "%1f %1f %1f", input->GetOriginalDirections()->ElementAt(i).get(0),
             input->GetOriginalDirections()->ElementAt(i).get(1), input->GetOriginalDirections()->ElementAt(i).get(2));

    itk::EncapsulateMetaData<std::string>(input->GetVectorImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
  }

  typedef itk::VectorImage<TPixelType,3> ImageType;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext == ".hdwi" || ext == ".dwi")
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    //io->SetNrrdVectorType( nrrdKindList );
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();

    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();
    nrrdWriter->SetInput( input->GetVectorImage() );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName(m_FileName);
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
    typename ImageType::Pointer vecimg = input->GetVectorImage();

    typedef itk::Image<TPixelType,4> ImageType4D;
    typename ImageType4D::Pointer img4 = ImageType4D::New();

    typename ImageType::SpacingType spacing = vecimg->GetSpacing();
    typename ImageType4D::SpacingType spacing4;
    for(int i=0; i<3; i++)
      spacing4[i] = spacing[i];
    spacing4[3] = 1;
    img4->SetSpacing( spacing4 );   // Set the image spacing

    typename ImageType::PointType origin = vecimg->GetOrigin();
    typename ImageType4D::PointType origin4;
    for(int i=0; i<3; i++)
      origin4[i] = origin[i];
    origin4[3] = 0;
    img4->SetOrigin( origin4 );     // Set the image origin

    typename ImageType::DirectionType direction = vecimg->GetDirection();
    typename ImageType4D::DirectionType direction4;
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        direction4[i][j] = direction[i][j];
    for(int i=0; i<4; i++)
      direction4[i][3] = 0;
    for(int i=0; i<4; i++)
      direction4[3][i] = 0;
    direction4[3][3] = 1;
    img4->SetDirection( direction4 );  // Set the image direction

    typename ImageType::RegionType region = vecimg->GetLargestPossibleRegion();
    typename ImageType4D::RegionType region4;

    typename ImageType::RegionType::SizeType size = region.GetSize();
    typename ImageType4D::RegionType::SizeType size4;

    for(int i=0; i<3; i++)
      size4[i] = size[i];
    size4[3] = vecimg->GetVectorLength();

    typename ImageType::RegionType::IndexType index = region.GetIndex();
    typename ImageType4D::RegionType::IndexType index4;
    for(int i=0; i<3; i++)
      index4[i] = index[i];
    index4[3] = 0;

    region4.SetSize(size4);
    region4.SetIndex(index4);
    img4->SetRegions( region4 );

    img4->Allocate();

    itk::ImageRegionIterator<ImageType>   it (vecimg, vecimg->GetLargestPossibleRegion() );
    typedef typename ImageType::PixelType VecPixType;

    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      typename ImageType::IndexType currentIndex = it.GetIndex();
      for(unsigned int ind=0; ind<vec.Size(); ind++)
      {

        for(int i=0; i<3; i++)
          index4[i] = currentIndex[i];
        index4[3] = ind;
        img4->SetPixel(index4, vec[ind]);
      }
    }

    // create copy of file with correct ending for mitk
    std::string fname3 = m_FileName;
    std::string::iterator itend = fname3.end();
    if (ext == ".fsl")
      fname3.replace( itend-3, itend, "nii");
    else
      fname3.replace( itend-5, itend, "nii.gz");

    itk::NiftiImageIO::Pointer io4 = itk::NiftiImageIO::New();

    typedef itk::VectorImage<TPixelType,3> ImageType;
    typedef itk::ImageFileWriter<ImageType4D> WriterType4;
    typename WriterType4::Pointer nrrdWriter4 = WriterType4::New();
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

    itksys::SystemTools::CopyAFile(fname3.c_str(), m_FileName.c_str());

    if(input->GetDirections()->Size())
    {
      std::ofstream myfile;
      std::string fname = m_FileName;
      fname += ".bvals";
      myfile.open (fname.c_str());
      for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
      {
        double twonorm = input->GetDirections()->ElementAt(i).two_norm();
        myfile << input->GetB_Value()*twonorm*twonorm << " ";
      }
      myfile.close();

      std::ofstream myfile2;
      std::string fname2 = m_FileName;
      fname2 += ".bvecs";
      myfile2.open (fname2.c_str());
      for(int j=0; j<3; j++)
      {
        for(unsigned int i=0; i<input->GetDirections()->Size(); i++)
        {
          //need to modify the length
          typename mitk::DiffusionImage<TPixelType>::GradientDirectionContainerType::Pointer grads = input->GetDirections();
          typename mitk::DiffusionImage<TPixelType>::GradientDirectionType direction = grads->ElementAt(i);
          direction.normalize();
          myfile2 << direction.get(j) << " ";
          //myfile2 << input->GetDirections()->ElementAt(i).get(j) << " ";
        }
        myfile2 << std::endl;
      }

      std::ofstream myfile3;
      std::string fname4 = m_FileName;
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
  m_Success = true;
}

template<typename TPixelType>
void mitk::NrrdDiffusionImageWriter<TPixelType>::SetInput( InputType* diffVolumes )
{
  this->ProcessObject::SetNthInput( 0, diffVolumes );
}

template<typename TPixelType>
mitk::DiffusionImage<TPixelType>* mitk::NrrdDiffusionImageWriter<TPixelType>::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( 0 ) );
  }
}

template<typename TPixelType>
std::vector<std::string> mitk::NrrdDiffusionImageWriter<TPixelType>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".dwi");
  possibleFileExtensions.push_back(".hdwi");
  possibleFileExtensions.push_back(".fsl");
  possibleFileExtensions.push_back(".fslgz");
  return possibleFileExtensions;
}

#endif //__mitkNrrdDiffusionImageWriter__cpp
