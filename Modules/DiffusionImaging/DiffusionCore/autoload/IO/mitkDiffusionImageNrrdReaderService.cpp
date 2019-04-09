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

#ifndef __mitkDiffusionImageNrrdReaderService_cpp
#define __mitkDiffusionImageNrrdReaderService_cpp

#include "mitkDiffusionImageNrrdReaderService.h"

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
#include "itkNrrdImageIO.h"

#include "mitkCustomMimeType.h"
#include "mitkDiffusionCoreIOMimeTypes.h"

#include <mitkITKImageImport.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageDataItem.h>
#include "mitkIOUtil.h"
#include <mitkLocaleSwitch.h>

namespace mitk
{

  DiffusionImageNrrdReaderService::
  DiffusionImageNrrdReaderService(const DiffusionImageNrrdReaderService & other)
    : AbstractFileReader(other)
  {
  }


  DiffusionImageNrrdReaderService* DiffusionImageNrrdReaderService::Clone() const
  {
    return new DiffusionImageNrrdReaderService(*this);
  }


  DiffusionImageNrrdReaderService::
  ~DiffusionImageNrrdReaderService()
  {}

  DiffusionImageNrrdReaderService::
  DiffusionImageNrrdReaderService()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE_DESCRIPTION() )
  {
    Options defaultOptions;
    defaultOptions["Apply image rotation to gradients"] = true;
    this->SetDefaultOptions(defaultOptions);
    m_ServiceReg = this->RegisterService();
  }

  std::vector<itk::SmartPointer<mitk::BaseData> >
  DiffusionImageNrrdReaderService::
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


  void DiffusionImageNrrdReaderService::InternalRead()
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
        mitk::LocaleSwitch localeSwitch("C");

        MITK_INFO << "DiffusionImageNrrdReaderService: reading image information";
        VectorImageType::Pointer itkVectorImage;

        std::string ext = this->GetMimeType()->GetExtension( this->GetInputLocation() );
        ext = itksys::SystemTools::LowerCase( ext );

        if (ext == ".hdwi" || ext == ".dwi" || ext == ".nrrd")
        {
          typedef itk::ImageFileReader<VectorImageType> FileReaderType;
          FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName(this->GetInputLocation());
          itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
          reader->SetImageIO(io);
          reader->Update();
          itkVectorImage = reader->GetOutput();
        }

        // Diffusion Image information START
        GradientDirectionContainerType::Pointer DiffusionVectors = GradientDirectionContainerType::New();
        GradientDirectionContainerType::Pointer OriginalDiffusionVectors = GradientDirectionContainerType::New();
        MeasurementFrameType MeasurementFrame;
        float BValue = -1;
        // Diffusion Image information END

        if (ext == ".hdwi" || ext == ".dwi" || ext == ".nrrd")
        {

          itk::MetaDataDictionary imgMetaDictionary =  itkVectorImage->GetMetaDataDictionary();
          std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
          std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
          std::string metaString;

          GradientDirectionType vect3d;

          int numberOfImages = 0;
          int numberOfGradientImages = 0;
          bool readb0 = false;
          double xx, xy, xz, yx, yy, yz, zx, zy, zz;

          for (; itKey != imgMetaKeys.end(); itKey ++)
          {
            double x,y,z;

            itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
            if (itKey->find("DWMRI_gradient") != std::string::npos)
            {
              sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
              vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
              DiffusionVectors->InsertElement( numberOfImages, vect3d );
              ++numberOfImages;
              // If the direction is 0.0, this is a reference image
              if (vect3d[0] == 0.0 &&
                  vect3d[1] == 0.0 &&
                  vect3d[2] == 0.0)
              {
                continue;
              }
              ++numberOfGradientImages;;
            }
            else if (itKey->find("DWMRI_b-value") != std::string::npos)
            {
              readb0 = true;
              BValue = atof(metaString.c_str());
            }
            else if (itKey->find("measurement frame") != std::string::npos)
            {
              sscanf(metaString.c_str(), " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);

              if (xx>10e-10 || xy>10e-10 || xz>10e-10 ||
                  yx>10e-10 || yy>10e-10 || yz>10e-10 ||
                  zx>10e-10 || zy>10e-10 || zz>10e-10 )
              {
                MeasurementFrame(0,0) = xx;
                MeasurementFrame(0,1) = xy;
                MeasurementFrame(0,2) = xz;
                MeasurementFrame(1,0) = yx;
                MeasurementFrame(1,1) = yy;
                MeasurementFrame(1,2) = yz;
                MeasurementFrame(2,0) = zx;
                MeasurementFrame(2,1) = zy;
                MeasurementFrame(2,2) = zz;
              }
              else
              {
                MeasurementFrame(0,0) = 1;
                MeasurementFrame(0,1) = 0;
                MeasurementFrame(0,2) = 0;
                MeasurementFrame(1,0) = 0;
                MeasurementFrame(1,1) = 1;
                MeasurementFrame(1,2) = 0;
                MeasurementFrame(2,0) = 0;
                MeasurementFrame(2,1) = 0;
                MeasurementFrame(2,2) = 1;
              }
            }
          }

          if(!readb0)
          {
            MITK_INFO << "BValue not specified in header file";
          }

        }

        outputForCache = mitk::GrabItkImageMemory( itkVectorImage);

        // create BValueMap
        mitk::BValueMapProperty::BValueMap BValueMap = mitk::BValueMapProperty::CreateBValueMap(DiffusionVectors,BValue);
        mitk::DiffusionPropertyHelper::SetOriginalGradientContainer(outputForCache, DiffusionVectors);
        mitk::DiffusionPropertyHelper::SetMeasurementFrame(outputForCache, MeasurementFrame);
        mitk::DiffusionPropertyHelper::SetBValueMap(outputForCache, BValueMap);
        mitk::DiffusionPropertyHelper::SetReferenceBValue(outputForCache, BValue);
        mitk::DiffusionPropertyHelper::SetApplyMatrixToGradients(outputForCache, us::any_cast<bool>(this->GetOptions()["Apply image rotation to gradients"]));
        mitk::DiffusionPropertyHelper::InitializeImage(outputForCache);

        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();

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
