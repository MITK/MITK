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

#include "mitkBaseDataIOFactory.h"
#include "mitkDiffusionImage.h"
#include "mitkBaseData.h"

#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include "mitkCommandLineParser.h"
#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

#include "mitkDiffusionDICOMFileReader.h"
#include "mitkDICOMTagBasedSorter.h"
#include "mitkDICOMSortByTag.h"

#include "itkMergeDiffusionImagesFilter.h"
#include <mitkIOUtil.h>

static mitk::StringList& GetInputFilenames()
{
  static mitk::StringList inputs;
  return inputs;
}

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
      inputlist.push_back(  itksys::SystemTools::ConvertToOutputPath( fullpath.c_str() ) );

      MITK_INFO("dicom.loader.inputdir.addfile") << input.GetFile(idx);
    }
  }

  /*mergedlist.reserve( GetInputFilenames().size() + inputlist.size() );

  mergedlist.insert( mergedlist.end(), GetInputFilenames().begin(), GetInputFilenames().end() );
  mergedlist.insert( mergedlist.end(), inputlist.begin(), inputlist.end() );*/

  GetInputFilenames() = inputlist;//mergedlist;

  MITK_INFO("dicom.loader.setinputfiles.end") << "[]";
}


mitk::DiffusionImage<short>::Pointer ReadInDICOMFiles( mitk::StringList& input_files, std::string output_file )
{
  // repeat test with some more realistic sorting
  mitk::DiffusionDICOMFileReader::Pointer gdcmReader = mitk::DiffusionDICOMFileReader::New(); // this also tests destruction
  mitk::DICOMTagBasedSorter::Pointer tagSorter = mitk::DICOMTagBasedSorter::New();

  // Use tags as in Qmitk
  // all the things that split by tag in DicomSeriesReader
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0010) ); // Number of Rows
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0011) ); // Number of Columns
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0030) ); // Pixel Spacing
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x1164) ); // Imager Pixel Spacing
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0037) ); // Image Orientation (Patient) // TODO add tolerance parameter (l. 1572 of original code)
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0018, 0x0050) ); // Slice Thickness
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0028, 0x0008) ); // Number of Frames
  tagSorter->AddDistinguishingTag( mitk::DICOMTag(0x0020, 0x0052) ); // Frame of Reference UID

  mitk::DICOMSortCriterion::ConstPointer sorting =
      mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0013), // instance number
                                 mitk::DICOMSortByTag::New( mitk::DICOMTag(0x0020, 0x0012) //acquisition number
                                                            ).GetPointer()
                                 ).GetPointer();
  tagSorter->SetSortCriterion( sorting );

  MITK_INFO("dicom.loader.read.init") << "[]" ;
  MITK_INFO("dicom.loader.read.inputs") << " " << input_files.size();

  gdcmReader->SetInputFiles( input_files );
  gdcmReader->AddSortingElement( tagSorter );
  gdcmReader->AnalyzeInputFiles();
  gdcmReader->LoadImages();

  mitk::Image::Pointer loaded_image = gdcmReader->GetOutput(0).GetMitkImage();

  mitk::DiffusionImage<short>::Pointer d_img = static_cast<mitk::DiffusionImage<short>*>( loaded_image.GetPointer() );

  return d_img;
}

typedef short DiffusionPixelType;
typedef itk::VectorImage<DiffusionPixelType,3>         DwiImageType;
typedef DwiImageType::PixelType                        DwiPixelType;
typedef DwiImageType::RegionType                       DwiRegionType;
typedef std::vector< DwiImageType::Pointer >  DwiImageContainerType;

typedef mitk::DiffusionImage<DiffusionPixelType>              DiffusionImageType;
typedef DiffusionImageType::GradientDirectionContainerType    GradientContainerType;
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

    std::string directory_path = std::string(rootdir.GetPath()) + std::string("/") + current_path;

    MITK_INFO("dicom.loader.inputrootdir.test") << "ProbePath:   " << current_path;
    MITK_INFO("dicom.loader.inputrootdir.test") << "IsDirectory: " << itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
                                                << " StartsWith: " << itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() );

    // test for prefix
    if(    itksys::SystemTools::FileIsDirectory( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ).c_str() )
           && itksys::SystemTools::StringStartsWith( current_path.c_str(), subdir_prefix.c_str() )
           )
    {

      MITK_INFO("dicom.loader.inputrootdir.searchin") << directory_path;
      SetInputFileNames( itksys::SystemTools::ConvertToOutputPath( directory_path.c_str() ) );

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
  parser.addArgument("inputdir", "i", mitkCommandLineParser::String, "Input Directory" ,"input directory containing dicom files", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::String, "Output File Name", "output file", us::Any(), false);
  parser.addArgument("dwprefix", "p", mitkCommandLineParser::String, "Recursive Scan Prefix", "prefix for subfolders search rootdir is specified by the 'inputdir' argument value", us::Any(), true);
  parser.addArgument("dryrun", "-s", mitkCommandLineParser::Bool, "Dry run","do not read, only look for input files ", us::Any(), true );

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }

  std::string inputDirectory = us::any_cast<std::string>( parsedArgs["inputdir"] );
  MITK_INFO << "Loading data from directory: " << inputDirectory;

  // retrieve the prefix flag (if set)
  bool search_for_subdirs = false;
  std::string subdir_prefix;
  if( parsedArgs.count("dwprefix"))
  {
    MITK_INFO << "Prefix specified, will search for subdirs in the input directory!";
    subdir_prefix = us::any_cast<std::string>( parsedArgs["dwprefix"] );
    search_for_subdirs = true;
  }

  // retrieve the output
  std::string outputFile = us::any_cast< std::string >( parsedArgs["output"] );

  // if the executable is called with a single directory, just parse the given folder for files and read them into a diffusion image
  if( !search_for_subdirs )
  {
    SetInputFileNames( inputDirectory );

    MITK_INFO << "Got " << GetInputFilenames().size() << " input files.";
    mitk::DiffusionImage<short>::Pointer d_img = ReadInDICOMFiles( GetInputFilenames(), outputFile );

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
    std::vector<mitk::DiffusionImage<DiffusionPixelType>::Pointer> output_container;

    SearchForInputInSubdirs( inputDirectory, subdir_prefix, output_container );

    // final output image
    mitk::DiffusionImage<DiffusionPixelType>::Pointer image = mitk::DiffusionImage<DiffusionPixelType>::New();
    if( output_container.size() > 1 )
    {
      DwiImageContainerType       imageContainer;
      GradientListContainerType   gradientListContainer;
      std::vector< double >       bValueContainer;

      for ( std::vector< mitk::DiffusionImage<DiffusionPixelType>::Pointer >::iterator dwi = output_container.begin();
            dwi != output_container.end(); ++dwi )
      {
        imageContainer.push_back((*dwi)->GetVectorImage());
        gradientListContainer.push_back((*dwi)->GetDirections());
        bValueContainer.push_back((*dwi)->GetReferenceBValue());
      }

      typedef itk::MergeDiffusionImagesFilter<short> FilterType;
      FilterType::Pointer filter = FilterType::New();
      filter->SetImageVolumes(imageContainer);
      filter->SetGradientLists(gradientListContainer);
      filter->SetBValues(bValueContainer);
      filter->Update();

      vnl_matrix_fixed< double, 3, 3 > mf; mf.set_identity();

      image->SetVectorImage( filter->GetOutput() );
      image->SetReferenceBValue(filter->GetB_Value());
      image->SetDirections(filter->GetOutputGradients());
      image->SetMeasurementFrame(mf);
      image->InitializeFromVectorImage();
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
