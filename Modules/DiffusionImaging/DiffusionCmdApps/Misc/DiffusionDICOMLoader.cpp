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

#include "mitkImage.h"
#include "mitkBaseData.h"
#include <mitkDiffusionPropertyHelper.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include "mitkCommandLineParser.h"
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

#include "mitkDiffusionDICOMFileReader.h"
#include "mitkSortByImagePositionPatient.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"

#include "itkMergeDiffusionImagesFilter.h"
#include <mitkIOUtil.h>

static mitk::StringList& GetInputFilenames()
{
  static mitk::StringList inputs;
  return inputs;
}

// FIXME slash at the end
void SetInputFileNames( std::string input_directory )
{
  // I. Get all files in directory
  itksys::Directory input;
  input.Load( input_directory.c_str() );

  // II. Push back files
  mitk::StringList inputlist;//, mergedlist;
  for( unsigned long idx=0; idx<input.GetNumberOfFiles(); idx++)
  {
    if( ! itksys::SystemTools::FileIsDirectory( input.GetFile(idx)) )
    {
      std::string fullpath = input_directory + "/" + std::string( input.GetFile(idx) );
      //inputlist.push_back(  itksys::SystemTools::ConvertToOutputPath( fullpath.c_str() ) );
      inputlist.push_back(  fullpath.c_str() );

      MITK_INFO("dicom.loader.inputdir.addfile") << input.GetFile(idx);
    }
  }

  /*mergedlist.reserve( GetInputFilenames().size() + inputlist.size() );

  mergedlist.insert( mergedlist.end(), GetInputFilenames().begin(), GetInputFilenames().end() );
  mergedlist.insert( mergedlist.end(), inputlist.begin(), inputlist.end() );*/

  GetInputFilenames() = inputlist;//mergedlist;

  MITK_INFO("dicom.loader.setinputfiles.end") << "[]";
}


static mitk::Image::Pointer ReadInDICOMFiles( mitk::StringList& input_files, const std::string &)
{
  mitk::DiffusionDICOMFileReader::Pointer gdcmReader = mitk::DiffusionDICOMFileReader::New();
  //mitk::ClassicDICOMSeriesReader::Pointer gdcmReader = mitk::ClassicDICOMSeriesReader::New();
 /* mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

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
  //gdcmReader->SetResolveMosaic( this->m_Controls->m_SplitMosaicCheckBox->isChecked() );
  gdcmReader->AddSortingElement( tagSorter );*/
  gdcmReader->SetInputFiles( input_files );

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

  mitk::Image::Pointer loaded_image = gdcmReader->GetOutput(0).GetMitkImage();

  return loaded_image;
}

typedef short DiffusionPixelType;
typedef itk::VectorImage<DiffusionPixelType,3>         DwiImageType;
typedef DwiImageType::PixelType                        DwiPixelType;
typedef DwiImageType::RegionType                       DwiRegionType;
typedef std::vector< DwiImageType::Pointer >  DwiImageContainerType;

typedef mitk::Image          DiffusionImageType;
typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType    GradientContainerType;
typedef std::vector< GradientContainerType::Pointer >  GradientListContainerType;

void SearchForInputInSubdirs( std::string root_directory, std::string subdir_prefix , std::vector<DiffusionImageType::Pointer>& output_container)
{
  // I. Get all dirs in directory
  itksys::Directory rootdir;
  rootdir.Load( root_directory.c_str() );

  MITK_INFO("dicom.loader.setinputdirs.start") << "Prefix = " << subdir_prefix;

  for( unsigned int idx=0; idx<rootdir.GetNumberOfFiles(); idx++)
  {
    std::string current_path = rootdir.GetFile(idx);

    std::string directory_path = std::string(rootdir.GetPath()) + current_path;

    MITK_INFO("dicom.loader.inputrootdir.test") << "ProbePath:   " << current_path << "\n"
                                                << "escaped to " <<  itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() );

    MITK_INFO("dicom.loader.inputrootdir.test") << " IsDirectory: " << itksys::SystemTools::FileIsDirectory( directory_path.c_str() )
                                                << " StartsWith: " << itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() );

    // test for prefix
    if(    itksys::SystemTools::FileIsDirectory( directory_path.c_str() )
           && itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() )
           )
    {

      MITK_INFO("dicom.loader.inputrootdir.searchin") << directory_path;
      SetInputFileNames( directory_path.c_str() );

      MITK_INFO("dicom.loader.inputrootdir.preload") << "[]" ;
      DiffusionImageType::Pointer dwi = ReadInDICOMFiles( GetInputFilenames(), "" );

      output_container.push_back( dwi );


    }
  }

  MITK_INFO("dicom.loader.setinputdirs.end") << "[]";
}



using namespace mitk;
/**
 * Read DICOM Files through the new (refactored) Diffusion DICOM Loader. It serves also as a first test of the new functionality, it will replace the
 * current loading mechanism after the necessary parts are merged into the master branch.
 */
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  parser.setTitle("Diffusion Dicom Loader");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Loads Diffusion Dicom files.");
  parser.setContributor("MIC");

  parser.addArgument("", "i", mitkCommandLineParser::String, "Input Directory" ,"input directory containing dicom files", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output File Name", "output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("dwprefix", "p", mitkCommandLineParser::String, "Recursive Scan Prefix", "prefix for subfolders search rootdir is specified by the 'inputdir' argument value", us::Any(), true);
  parser.addArgument("dryrun", "s", mitkCommandLineParser::Bool, "Dry run","do not read, only look for input files ", us::Any(), true );

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }

  std::string inputDirectory = us::any_cast<std::string>( parsedArgs["i"] );
  MITK_INFO << "Loading data from directory: " << inputDirectory;

  // retrieve the prefix flag (if set)
  bool search_for_subdirs = false;
  std::string subdir_prefix;
  if( parsedArgs.count("dwprefix"))
  {
    subdir_prefix = us::any_cast<std::string>( parsedArgs["dwprefix"] );
    if (subdir_prefix != "")
    {
      MITK_INFO << "Prefix specified, will search for subdirs in the input directory!";
      search_for_subdirs = true;
    }
  }

  // retrieve the output
  std::string outputFile = us::any_cast< std::string >( parsedArgs["o"] );

  // if the executable is called with a single directory, just parse the given folder for files and read them into a diffusion image
  if( !search_for_subdirs )
  {
    SetInputFileNames( inputDirectory );

    MITK_INFO << "Got " << GetInputFilenames().size() << " input files.";
    mitk::Image::Pointer d_img = ReadInDICOMFiles( GetInputFilenames(), outputFile );

    try
    {
      mitk::IOUtil::Save(d_img, outputFile.c_str());
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Failed to write out the output file. \n\t Reason : ITK Exception " << e.what();
    }

  }
  // if the --dwprefix flag is set, then we have to look for the directories, load each of them separately and afterwards merge the images
  else
  {
    std::vector<mitk::Image::Pointer> output_container;

    SearchForInputInSubdirs( inputDirectory, subdir_prefix, output_container );

    // final output image
    mitk::Image::Pointer image = mitk::Image::New();
    if( output_container.size() > 1 )
    {
      DwiImageContainerType       imageContainer;
      GradientListContainerType   gradientListContainer;
      std::vector< double >       bValueContainer;

      for ( std::vector< mitk::Image::Pointer >::iterator dwi = output_container.begin();
        dwi != output_container.end(); ++dwi )
      {
        mitk::DiffusionPropertyHelper::ImageType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::ImageType::New();
        mitk::CastToItkImage(*dwi, itkVectorImagePointer);

        imageContainer.push_back(itkVectorImagePointer);
        gradientListContainer.push_back( mitk::DiffusionPropertyHelper::GetGradientContainer(*dwi));
        bValueContainer.push_back( mitk::DiffusionPropertyHelper::GetReferenceBValue(*dwi));
      }

      typedef itk::MergeDiffusionImagesFilter<short> FilterType;
      FilterType::Pointer filter = FilterType::New();
      filter->SetImageVolumes(imageContainer);
      filter->SetGradientLists(gradientListContainer);
      filter->SetBValues(bValueContainer);
      filter->Update();

      vnl_matrix_fixed< double, 3, 3 > mf; mf.set_identity();

      image = mitk::GrabItkImageMemory( filter->GetOutput() );

      mitk::DiffusionPropertyHelper::SetGradientContainer(image, filter->GetOutputGradients());
      mitk::DiffusionPropertyHelper::SetReferenceBValue(image, filter->GetB_Value());
      mitk::DiffusionPropertyHelper::SetMeasurementFrame(image, mf);
      mitk::DiffusionPropertyHelper::InitializeImage( image );
    }
    // just output the image if there was only one folder found
    else
    {
      image = output_container.at(0);
    }

    MITK_INFO("dicom.import.writeout") << " [OutputFile] " << outputFile.c_str();

    try
    {
      mitk::IOUtil::Save(image, outputFile.c_str());
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Failed to write out the output file. \n\t Reason : ITK Exception " << e.what();
    }

  }

  return 1;
}

