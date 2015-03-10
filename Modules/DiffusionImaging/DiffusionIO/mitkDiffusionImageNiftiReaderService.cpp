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

#ifndef __mitkDiffusionImageNiftiReaderService_cpp
#define __mitkDiffusionImageNiftiReaderService_cpp

#include "mitkDiffusionImageNiftiReaderService.h"

#include <iostream>
#include <fstream>


// Diffusion properties
#include <mitkGradientDirectionsProperty.h>
#include <mitkBValueMapProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <mitkProperties.h>

// ITK includes
#include <itkImageRegionIterator.h>
#include <itkMetaDataObject.h>
#include "itksys/SystemTools.hxx"
#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNiftiImageIO.h"

#include "mitkCustomMimeType.h"
#include "mitkDiffusionIOMimeTypes.h"

#include <mitkITKImageImport.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageDataItem.h>
#include "mitkIOUtil.h"


namespace mitk
{

  DiffusionImageNiftiReaderService::
  DiffusionImageNiftiReaderService(const DiffusionImageNiftiReaderService & other)
    : AbstractFileReader(other)
  {
  }


  DiffusionImageNiftiReaderService* DiffusionImageNiftiReaderService::Clone() const
  {
    return new DiffusionImageNiftiReaderService(*this);
  }


  DiffusionImageNiftiReaderService::
  ~DiffusionImageNiftiReaderService()
  {}

  DiffusionImageNiftiReaderService::
  DiffusionImageNiftiReaderService()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionIOMimeTypes::DWI_NIFTI_MIMETYPE() ), mitk::DiffusionIOMimeTypes::DWI_NIFTI_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  std::vector<itk::SmartPointer<mitk::BaseData> >
  DiffusionImageNiftiReaderService::
  Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;

    // Since everything is completely read in GenerateOutputInformation() it is stored
    // in a cache variable. A timestamp is associated.
    // If the timestamp of the cache variable is newer than the MTime, we only need to
    // assign the cache variable to the DataObject.
    // Otherwise, the tree must be read again from the file and OuputInformation must
    // be updated!

    if(m_OutputCache.IsNull()) InternalRead();

    result.push_back(m_OutputCache.GetPointer());
    return result;
  }


  void DiffusionImageNiftiReaderService::InternalRead()
  {
    OutputType::Pointer outputForCache = OutputType::New();
    if ( this->GetInputLocation() == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
    }
    else
    {
      try
      {
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


        MITK_INFO << "DiffusionImageNiftiReaderService: reading image information";
        VectorImageType::Pointer itkVectorImage;

        std::string ext = this->GetMimeType()->GetExtension( this->GetInputLocation() );
        ext = itksys::SystemTools::LowerCase( ext );

        if(ext == ".fsl" || ext == ".fslgz")
        {
          // create temporary file with correct ending for nifti-io
          std::string fname3 = "temp_dwi";
          fname3 += ext == ".fsl" ? ".nii" : ".nii.gz";
          itksys::SystemTools::CopyAFile(this->GetInputLocation().c_str(), fname3.c_str());

          // create reader and read file
          typedef itk::Image<DiffusionPixelType,4> ImageType4D;
          itk::NiftiImageIO::Pointer io2 = itk::NiftiImageIO::New();
          typedef itk::ImageFileReader<ImageType4D> FileReaderType;
          FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName(fname3);
          reader->SetImageIO(io2);
          reader->Update();
          ImageType4D::Pointer img4 = reader->GetOutput();

          // delete temporary file
          itksys::SystemTools::RemoveFile(fname3.c_str());

          // convert 4D file to vector image
          itkVectorImage = VectorImageType::New();

          VectorImageType::SpacingType spacing;
          ImageType4D::SpacingType spacing4 = img4->GetSpacing();
          for(int i=0; i<3; i++)
            spacing[i] = spacing4[i];
          itkVectorImage->SetSpacing( spacing );   // Set the image spacing

          VectorImageType::PointType origin;
          ImageType4D::PointType origin4 = img4->GetOrigin();
          for(int i=0; i<3; i++)
            origin[i] = origin4[i];
          itkVectorImage->SetOrigin( origin );     // Set the image origin

          VectorImageType::DirectionType direction;
          ImageType4D::DirectionType direction4 = img4->GetDirection();
          for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
              direction[i][j] = direction4[i][j];
          itkVectorImage->SetDirection( direction );  // Set the image direction

          VectorImageType::RegionType region;
          ImageType4D::RegionType region4 = img4->GetLargestPossibleRegion();

          VectorImageType::RegionType::SizeType size;
          ImageType4D::RegionType::SizeType size4 = region4.GetSize();

          for(int i=0; i<3; i++)
            size[i] = size4[i];

          VectorImageType::RegionType::IndexType index;
          ImageType4D::RegionType::IndexType index4 = region4.GetIndex();
          for(int i=0; i<3; i++)
            index[i] = index4[i];

          region.SetSize(size);
          region.SetIndex(index);
          itkVectorImage->SetRegions( region );

          itkVectorImage->SetVectorLength(size4[3]);
          itkVectorImage->Allocate();

          itk::ImageRegionIterator<VectorImageType>   it ( itkVectorImage,  itkVectorImage->GetLargestPossibleRegion() );
          typedef VectorImageType::PixelType VecPixType;
          for (it.GoToBegin(); !it.IsAtEnd(); ++it)
          {
            VecPixType vec = it.Get();
            VectorImageType::IndexType currentIndex = it.GetIndex();
            for(int i=0; i<3; i++)
              index4[i] = currentIndex[i];
            for(unsigned int ind=0; ind<vec.Size(); ind++)
            {
              index4[3] = ind;
              vec[ind] = img4->GetPixel(index4);
            }
            it.Set(vec);
          }
        }
        else if(ext == ".nii" || ext == ".nii.gz")
        {
          // create reader and read file
          typedef itk::Image<DiffusionPixelType,4> ImageType4D;
          itk::NiftiImageIO::Pointer io2 = itk::NiftiImageIO::New();
          typedef itk::ImageFileReader<ImageType4D> FileReaderType;
          FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName( this->GetInputLocation() );
          reader->SetImageIO(io2);
          reader->Update();
          ImageType4D::Pointer img4 = reader->GetOutput();


          // convert 4D file to vector image
          itkVectorImage = VectorImageType::New();

          VectorImageType::SpacingType spacing;
          ImageType4D::SpacingType spacing4 = img4->GetSpacing();
          for(int i=0; i<3; i++)
            spacing[i] = spacing4[i];
          itkVectorImage->SetSpacing( spacing );   // Set the image spacing

          VectorImageType::PointType origin;
          ImageType4D::PointType origin4 = img4->GetOrigin();
          for(int i=0; i<3; i++)
            origin[i] = origin4[i];
          itkVectorImage->SetOrigin( origin );     // Set the image origin

          VectorImageType::DirectionType direction;
          ImageType4D::DirectionType direction4 = img4->GetDirection();
          for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
              direction[i][j] = direction4[i][j];
          itkVectorImage->SetDirection( direction );  // Set the image direction

          VectorImageType::RegionType region;
          ImageType4D::RegionType region4 = img4->GetLargestPossibleRegion();

          VectorImageType::RegionType::SizeType size;
          ImageType4D::RegionType::SizeType size4 = region4.GetSize();

          for(int i=0; i<3; i++)
            size[i] = size4[i];

          VectorImageType::RegionType::IndexType index;
          ImageType4D::RegionType::IndexType index4 = region4.GetIndex();
          for(int i=0; i<3; i++)
            index[i] = index4[i];

          region.SetSize(size);
          region.SetIndex(index);
          itkVectorImage->SetRegions( region );

          itkVectorImage->SetVectorLength(size4[3]);
          itkVectorImage->Allocate();

          itk::ImageRegionIterator<VectorImageType>   it ( itkVectorImage,  itkVectorImage->GetLargestPossibleRegion() );
          typedef VectorImageType::PixelType VecPixType;
          for (it.GoToBegin(); !it.IsAtEnd(); ++it)
          {
            VecPixType vec = it.Get();
            VectorImageType::IndexType currentIndex = it.GetIndex();
            for(int i=0; i<3; i++)
              index4[i] = currentIndex[i];
            for(unsigned int ind=0; ind<vec.Size(); ind++)
            {
              index4[3] = ind;
              vec[ind] = img4->GetPixel(index4);
            }
            it.Set(vec);
          }
        }

        // Diffusion Image information START
        GradientDirectionContainerType::Pointer DiffusionVectors = GradientDirectionContainerType::New();
        GradientDirectionContainerType::Pointer OriginalDiffusionVectors = GradientDirectionContainerType::New();
        MeasurementFrameType MeasurementFrame;
        float BValue = -1;
        // Diffusion Image information END

        if(ext == ".fsl" || ext == ".fslgz" || ext == ".nii" || ext == ".nii.gz")
        {
          // some parsing depending on the extension
          bool useFSLstyle( true );
          std::string bvecsExtension("");
          std::string bvalsExtension("");

          std::string base = itksys::SystemTools::GetFilenamePath( this->GetInputLocation() ) + "/"
            + this->GetMimeType()->GetFilenameWithoutExtension( this->GetInputLocation() );

          // check for possible file names
          {
            if( useFSLstyle && itksys::SystemTools::FileExists( std::string( base + ".bvec").c_str() )
              && itksys::SystemTools::FileExists( std::string( base + ".bval").c_str() )
              )
            {
              useFSLstyle = false;
              bvecsExtension = ".bvec";
              bvalsExtension = ".bval";
            }

            if( useFSLstyle && itksys::SystemTools::FileExists( std::string( base + ".bvecs").c_str() )
              && itksys::SystemTools::FileExists( std::string( base + ".bvals").c_str() )
              )
            {
              useFSLstyle = false;
              bvecsExtension = ".bvecs";
              bvalsExtension = ".bvals";
            }
          }

          std::string line;
          std::vector<float> bvec_entries;
          std::string fname = this->GetInputLocation();
          if( useFSLstyle )
          {
            fname += ".bvecs";
          }
          else
          {
            fname = std::string( base + bvecsExtension);
          }
          std::ifstream myfile (fname.c_str());
          if (myfile.is_open())
          {
            while ( myfile.good() )
            {
              getline (myfile,line);
              char* pch = strtok (const_cast<char*>(line.c_str())," ");
              while (pch != NULL)
              {
                bvec_entries.push_back(atof(pch));
                pch = strtok (NULL, " ");
              }
            }
            myfile.close();
          }
          else
          {
            MITK_INFO << "Unable to open bvecs file";
          }

          std::vector<float> bval_entries;
          std::string fname2 = this->GetInputLocation();
          if( useFSLstyle )
          {
            fname2 += ".bvals";
          }
          else
          {
            fname2 = std::string( base + bvalsExtension);
          }
          std::ifstream myfile2 (fname2.c_str());
          if (myfile2.is_open())
          {
            while ( myfile2.good() )
            {
              getline (myfile2,line);
              char* pch = strtok (const_cast<char*>(line.c_str())," ");
              while (pch != NULL)
              {
                bval_entries.push_back(atof(pch));
                pch = strtok (NULL, " ");
              }
            }
            myfile2.close();
          }
          else
          {
            MITK_INFO << "Unable to open bvals file";
          }


          BValue = -1;
          unsigned int numb = bval_entries.size();
          for(unsigned int i=0; i<numb; i++)
          {

            // Take the first entry in bvals as the reference b-value
            if(BValue == -1 && bval_entries.at(i) != 0)
            {
              BValue = bval_entries.at(i);
            }

            float b_val = bval_entries.at(i);


            vnl_vector_fixed< double, 3 > vec;
            vec[0] = bvec_entries.at(i);
            vec[1] = bvec_entries.at(i+numb);
            vec[2] = bvec_entries.at(i+2*numb);

            // Adjust the vector length to encode gradient strength
            float factor = b_val/BValue;
            if(vec.magnitude() > 0)
            {
              vec[0] = sqrt(factor)*vec[0];
              vec[1] = sqrt(factor)*vec[1];
              vec[2] = sqrt(factor)*vec[2];
            }

            DiffusionVectors->InsertElement(i,vec);
          }

          for(int i=0; i<3; i++)
            for(int j=0; j<3; j++)
              MeasurementFrame[i][j] = i==j ? 1 : 0;
        }

        outputForCache = mitk::GrabItkImageMemory( itkVectorImage);

        // create BValueMap
        mitk::BValueMapProperty::BValueMap BValueMap = mitk::BValueMapProperty::CreateBValueMap(DiffusionVectors,BValue);
        outputForCache->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( DiffusionVectors ) );
        outputForCache->SetProperty( mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( OriginalDiffusionVectors ) );
        outputForCache->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( MeasurementFrame ) );
        outputForCache->SetProperty( mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New( BValueMap ) );
        outputForCache->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( BValue ) );


        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();

        try
        {
          setlocale(LC_ALL, currLocale.c_str());
        }
        catch(...)
        {
          MITK_INFO << "Could not reset locale " << currLocale;
        }
      }
      catch(std::exception& e)
      {
        MITK_INFO << "Std::Exception while reading file!!";
        MITK_INFO << e.what();
        throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
      }
      catch(...)
      {
        MITK_INFO << "Exception while reading file!!";
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
      }
    }
  }

} //namespace MITK

#endif
