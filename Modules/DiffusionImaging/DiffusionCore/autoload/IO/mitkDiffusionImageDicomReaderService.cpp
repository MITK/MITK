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

#ifndef __mitkDiffusionImageDicomReaderService_cpp
#define __mitkDiffusionImageDicomReaderService_cpp

#include "mitkDiffusionImageDicomReaderService.h"

#include <iostream>
#include <fstream>


// Diffusion properties
#include <mitkGradientDirectionsProperty.h>
#include <mitkBValueMapProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <mitkProperties.h>

#include <itkImageRegionIterator.h>
#include <itkMetaDataObject.h>
#include "itksys/SystemTools.hxx"
#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

#include "mitkCustomMimeType.h"
#include "mitkDiffusionCoreIOMimeTypes.h"
#include <mitkDiffusionDICOMFileReader.h>
#include <mitkDICOMTagBasedSorter.h>
#include <mitkSortByImagePositionPatient.h>
#include <mitkDICOMSortByTag.h>

#include <mitkITKImageImport.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageDataItem.h>
#include <mitkIOUtil.h>
#include <mitkLocaleSwitch.h>
#include <itksys/Directory.hxx>
#include <gdcmDirectory.h>
#include <mitkStringProperty.h>

namespace mitk
{

DiffusionImageDicomReaderService::
DiffusionImageDicomReaderService(const DiffusionImageDicomReaderService & other)
  : AbstractFileReader(other)
{
}


DiffusionImageDicomReaderService* DiffusionImageDicomReaderService::Clone() const
{
  return new DiffusionImageDicomReaderService(*this);
}


DiffusionImageDicomReaderService::
~DiffusionImageDicomReaderService()
{}

DiffusionImageDicomReaderService::
DiffusionImageDicomReaderService()
  : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DWI_DICOM_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DWI_DICOM_MIMETYPE_DESCRIPTION() )
{
  Options defaultOptions;
  defaultOptions["Load recoursive"] = false;
  defaultOptions["Split mosaic"] = true;
  this->SetDefaultOptions(defaultOptions);

  m_ServiceReg = this->RegisterService();
}

std::vector<itk::SmartPointer<mitk::BaseData> >
DiffusionImageDicomReaderService::Read()
{
  return InternalRead();
}


std::vector<itk::SmartPointer<mitk::BaseData> > DiffusionImageDicomReaderService::InternalRead()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result_images;
  OutputType::Pointer outputForCache = OutputType::New();
  if ( this->GetInputLocation() == "")
  {
    throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
  }
  else
  {
    const std::string& locale = "C";
    const std::string& currLocale = setlocale( LC_ALL, nullptr );

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

    try
    {
      Options options = this->GetOptions();
      bool load_recoursive = us::any_cast<bool>(options["Load recoursive"]);
      bool split_mosaic = us::any_cast<bool>(options["Split mosaic"]);

      gdcm::Directory::FilenamesType complete_list;
      std::string folderName = itksys::SystemTools::GetFilenamePath( this->GetInputLocation() );

      if( load_recoursive )
      {
        std::string subdir_prefix = "";

        itksys::Directory rootdir;
        rootdir.Load( folderName.c_str() );

        for( unsigned int idx=0; idx<rootdir.GetNumberOfFiles(); idx++)
        {
          std::string current_path = rootdir.GetFile(idx);
          std::string directory_path = std::string(rootdir.GetPath()) + std::string("/") + current_path;

          MITK_INFO("dicom.loader.inputrootdir.test") << "ProbePath:   " << current_path;
          MITK_INFO("dicom.loader.inputrootdir.test") << "IsDirectory: " << itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
                                                      << " StartsWith: " << itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() );

          // test for prefix
          if( itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
              && itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() )
              )
          {
            gdcm::Directory d;
            d.Load( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ) , false);

            MITK_INFO("dicom.load.subdir.attempt") << "In directory " << itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() );

            const gdcm::Directory::FilenamesType &l1 = d.GetFilenames();
            const unsigned int ntotalfiles = l1.size();

            for( unsigned int i=0; i< ntotalfiles; i++)
              complete_list.push_back( l1.at(i) );
          }
        }
      }
      else
      {
        gdcm::Directory d;
        d.Load( folderName.c_str(), load_recoursive ); // recursive !
        const gdcm::Directory::FilenamesType &l1 = d.GetFilenames();
        const unsigned int ntotalfiles = l1.size();

        for( unsigned int i=0; i< ntotalfiles; i++)
          complete_list.push_back( l1.at(i) );
      }

      mitk::DiffusionDICOMFileReader::Pointer gdcmReader = mitk::DiffusionDICOMFileReader::New();
      mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

      // Use tags as in Qmitk
      // all the things that split by tag in DicomSeriesReader
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0010) ); // Number of Rows
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0011) ); // Number of Columns
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0037) ); // Image Orientation (Patient) // TODO add tolerance parameter (l. 1572 of original code)
      // TODO handle as real vectors! cluster with configurable errors!
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x000e) ); // Series Instance UID
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x0050) ); // Slice Thickness
      tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0008) ); // Number of Frames
      //tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

      // gdcmReader->AddSortingElement( tagSorter );
      //mitk::DICOMFileReaderTestHelper::TestOutputsContainInputs( gdcmReader );

      mitk::DICOMSortCriterion::ConstPointer sorting =
          mitk::SortByImagePositionPatient::New(  // Image Position (Patient)
                                                  //mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0013), // instance number
                                                  mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0012), // aqcuisition number
                                                                             mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0008, 0x0032), // aqcuisition time
                                                                                                        mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0018, 0x1060), // trigger time
                                                                                                                                   mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0008, 0x0018) // SOP instance UID (last resort, not really meaningful but decides clearly)
                                                                                                                                                              ).GetPointer()
                                                                                                                                   ).GetPointer()
                                                                                                        ).GetPointer()
                                                                             ).GetPointer()
                                                  // ).GetPointer()
                                                  ).GetPointer();
      tagSorter->SetSortCriterion( sorting );

      // mosaic
      gdcmReader->SetResolveMosaic( split_mosaic );
      gdcmReader->AddSortingElement( tagSorter );
      gdcmReader->SetInputFiles( complete_list );
      try
      {
        gdcmReader->AnalyzeInputFiles();
      }
      catch( const itk::ExceptionObject &e)
      {
        MITK_ERROR << "Failed to analyze data. " << e.what();
      }
      catch( const std::exception &se)
      {
        MITK_ERROR << "Std Exception " << se.what();
      }

      gdcmReader->LoadImages();

      for( unsigned int o = 0; o < gdcmReader->GetNumberOfOutputs(); o++ )
      {
        mitk::Image::Pointer loaded_image = gdcmReader->GetOutput(o).GetMitkImage();

        std::stringstream ss;
        ss << "ImportedData_" << o;

        StringProperty::Pointer nameProp;
        if (gdcmReader->GetSeriesName(o)!="-")
          nameProp = StringProperty::New(gdcmReader->GetSeriesName(o));
        else if (gdcmReader->GetStudyName(o)!="-")
          nameProp = StringProperty::New(gdcmReader->GetStudyName(o));
        else
          nameProp = StringProperty::New(folderName);

        loaded_image->SetProperty("name", nameProp);

        result_images.push_back(loaded_image.GetPointer());
      }


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
      try
      {
        setlocale(LC_ALL, currLocale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not reset locale " << currLocale;
      }

      MITK_INFO << "Std::Exception while reading file!!";
      MITK_INFO << e.what();
      throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
    }
    catch(...)
    {
      try
      {
        setlocale(LC_ALL, currLocale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not reset locale " << currLocale;
      }

      MITK_INFO << "Exception while reading file!!";
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
    }
  }

  return result_images;
}

} //namespace MITK

#endif
