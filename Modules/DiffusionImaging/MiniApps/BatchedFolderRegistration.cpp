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

#include "MiniAppManager.h"

// CTK
#include "ctkCommandLineParser.h"

#include <mitkIOUtil.h>
#include <mitkRegistrationWrapper.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkDiffusionImage.h>
#include "mitkNrrdDiffusionImageWriter.h"
// ITK
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>


typedef std::vector<std::string> FileListType;


static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
  {
    elems.push_back(item);
  }
  return elems;
}

static std::vector<std::string> split(const std::string &s, char delim)
{
  std::vector < std::string > elems;
  return split(s, delim, elems);
}

/// Create list of all files in provided folder ending with same postfix
static FileListType CreateFileList(std::string folder , std::string postfix)
{
  itk::Directory::Pointer dir = itk::Directory::New();
  FileListType fileList;

  if( dir->Load(folder.c_str() ) )
  {
    int n = dir->GetNumberOfFiles();
    for(int r=0;r<n;r++)
    {
      std::string filename = dir->GetFile( r );
      if (filename == "." || filename == "..")
        continue;
      filename = folder + filename;
      if (!itksys::SystemTools::FileExists( filename.c_str()))
        continue;
      if (filename.substr(filename.length() -postfix.length() ) == postfix)
        fileList.push_back(filename);
    }
  }
  return fileList;
}

/// Add '_reg' tag to file name
static std::string GetSavePath(std::string outputFolder, std::string fileName)
{
  std::string fileType = itksys::SystemTools::GetFilenameExtension(fileName);
  std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(fileName);

  std::string savePathAndFileName = outputFolder +fileStem + "_reg" + fileType;

  return savePathAndFileName;
}

/// Build a derived file name from moving images e.g. xxx_T2.nrrd becomes xxx_GTV.nrrd
static FileListType CreateDerivedFileList(std::string baseFN, std::string baseSuffix, std::vector<std::string> derivedPatterns)
{
  FileListType files;
  for (unsigned int i=0; i < derivedPatterns.size(); i++)
  {
    std::string derResourceSuffix =  derivedPatterns.at(i);
    std::string derivedResourceFilename = baseFN.substr(0,baseFN.length() -baseSuffix.length()) + derResourceSuffix;
    MITK_INFO <<" Looking for file: " << derivedResourceFilename;

    if (!itksys::SystemTools::FileExists(derivedResourceFilename.c_str()))
    {
      MITK_INFO << "CreateDerivedFileList: File does not exit. Skipping entry.";
      continue;
    }
    files.push_back(derivedResourceFilename);
  }
  return files;
}

/// Save images according to file type
static void SaveImage(std::string fileName, mitk::Image* image, std::string fileType )
{
  MITK_INFO << "----Save to " << fileName;
  if (fileType == "dwi") // IOUtil does not handle dwi files properly Bug 15772
  {
    mitk::NrrdDiffusionImageWriter< short >::Pointer dwiwriter = mitk::NrrdDiffusionImageWriter< short >::New();
    dwiwriter->SetInput( dynamic_cast<mitk::DiffusionImage<short>* > (image));
    dwiwriter->SetFileName( fileName );
    try
    {
      dwiwriter->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      MITK_ERROR << "Caught exception: " << e.what();
      mitkThrow() << "Failed with exception from subprocess!";
    }
  }
  else
  {
    mitk::IOUtil::SaveImage(image, fileName);
  }
}

/// Copy derived resources from first time step. Append _reg tag, but leave data untouched.
static void CopyResources(FileListType fileList, std::string outputPath)
{
  for (unsigned int j=0; j < fileList.size(); j++)
  {
    std::string derivedResourceFilename = fileList.at(j);
    std::string fileType = itksys::SystemTools::GetFilenameExtension(derivedResourceFilename);
    std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(derivedResourceFilename);
    std::string savePathAndFileName = outputPath +fileStem + "_reg." + fileType;
    MITK_INFO << "Copy resource " << savePathAndFileName;
    mitk::Image::Pointer resImage = mitk::IOUtil::LoadImage(derivedResourceFilename);
    mitk::IOUtil::SaveImage(resImage, savePathAndFileName);
  }
}


int BatchedFolderRegistration( int argc, char* argv[] )
{
  ctkCommandLineParser parser;
  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",ctkCommandLineParser::Bool, "Show this help text");
  parser.addArgument("xml", "x",ctkCommandLineParser::Bool, "Print a XML description of this modules command line interface");
  //parser.addArgument("usemask", "u", QVariant::Bool, "Use segmentations (derived resources) to exclude areas from registration metrics");
  parser.addArgument("input", "i", ctkCommandLineParser::String, "Input folder",us::Any(),false);
  parser.addArgument("output", "o", ctkCommandLineParser::String, "Output folder (ending with /)",us::Any(),false);
  parser.addArgument("fixed", "f", ctkCommandLineParser::String, "Suffix for fixed image",us::Any(),false);
  parser.addArgument("moving", "m", ctkCommandLineParser::String, "Suffix for moving images",us::Any(),false);
  parser.addArgument("derived", "d", ctkCommandLineParser::String, "Derived resources suffixes (replaces suffix for moving images); comma separated",us::Any(),true);
  parser.addArgument("silent", "s", ctkCommandLineParser::Bool, "No xml progress output.");
  // Feature currently disabled
  //parser.addArgument("resample", "r", QVariant::String, "Reference Image for resampling (optional), is not applied to tensor data");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Handle special arguments
  bool silent = false;
  {
    if (parsedArgs.size() == 0)
    {
      MITK_ERROR << "Missig arguements" ;
      return EXIT_FAILURE;
    }

    if (parsedArgs.count("xml"))
    {
      MITK_ERROR << "This is to be handled by shell script";
      return EXIT_SUCCESS;
    }

    if (parsedArgs.count("silent"))
      silent = true;

    // Show a help message
    if ( parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }
  }
  std::string outputPath = us::any_cast<string>(parsedArgs["output"]);
  std::string refPattern = us::any_cast<string>(parsedArgs["fixed"]);
  std::string inputPath = us::any_cast<string>(parsedArgs["input"]);
  std::string movingImgPattern = us::any_cast<string>(parsedArgs["moving"]);
  //QString resampleReference = parsedArgs["resample"].toString();
  //bool maskTumor = parsedArgs["usemask"].toBool();

  // if derived sources pattern is provided, populate QStringList with possible filename postfixes
  std::vector<std::string> derPatterns;

  if (parsedArgs.count("derived") || parsedArgs.count("d") )
  {
    std::string arg =  us::any_cast<string>(parsedArgs["derived"]);
    derPatterns = split(arg ,',');
  }

  MITK_INFO << "Input Folder : " << inputPath;
  MITK_INFO << "Looking for reference image ...";
  FileListType referenceFileList = CreateFileList(inputPath,refPattern);

  if (referenceFileList.size() != 1)
  {
    MITK_ERROR << "None or more than one possible reference images (" << refPattern <<") found. Exiting." << referenceFileList.size();
    MITK_INFO  << "Choose a fixed arguement that is unique in the given folder!";
    return EXIT_FAILURE;
  }

  std::string referenceFileName = referenceFileList.at(0);

  MITK_INFO << "Loading Reference (fixed) image: " << referenceFileName;
  mitk::Image::Pointer refImage = mitk::IOUtil::LoadImage(referenceFileName);

  if (refImage.IsNull())
    MITK_ERROR << "Loaded fixed image is NULL";

  // Copy reference image to destination
  std::string savePathAndFileName = GetSavePath(outputPath, referenceFileName);
  mitk::IOUtil::SaveImage(refImage, savePathAndFileName);

  // Copy all derived resources also to output folder, adding _reg suffix
  referenceFileList = CreateDerivedFileList(referenceFileName, movingImgPattern,derPatterns);
  CopyResources(referenceFileList, outputPath);

  std::string derivedResourceFilename;
  mitk::Image::Pointer referenceMask = NULL; // union of all segmentations

  if (!silent)
  {
    // XML Output to report progress
    std::cout << "<filter-start>";
    std::cout << "<filter-name>Batched Registration</filter-name>";
    std::cout << "<filter-comment>Starting registration ... </filter-comment>";
    std::cout << "</filter-start>";
  }

  // Now iterate over all files and register them to the reference image,
  // also register derived resources based on file patterns
  // ------------------------------------------------------------------------------

  // Create File list

  FileListType movingImagesList = CreateFileList(inputPath, movingImgPattern);


  // TODO Reactivate Resampling Feature
  //  mitk::Image::Pointer resampleImage = NULL;
  //  if (QFileInfo(resampleReference).isFile())
  //  {
  //    resampleImage = mitk::IOUtil::LoadImage(resampleReference.toStdString());
  //  }
  for (unsigned int i =0; i < movingImagesList.size(); i++)
  {
    std::string fileMorphName = movingImagesList.at(i);
    if (fileMorphName == referenceFileName)
    {

      // do not process reference image again
      continue;
    }
    MITK_INFO << "Processing image " << fileMorphName;

    // 1 Register morphological file to reference image

    if (!itksys::SystemTools::FileExists(fileMorphName.c_str()))
    {
      MITK_WARN << "File does not exit. Skipping entry.";
      continue;
    }
    // Origin of images is cancelled
    // TODO make this optional!!
    double transf[6];
    double offset[3];
    {
      mitk::Image::Pointer movingImage = mitk::IOUtil::LoadImage(fileMorphName);

      MITK_ERROR << "REF COUNT MOVING 0 " << movingImage->GetReferenceCount();
      if (movingImage.IsNull())
        MITK_ERROR << "Loaded moving image is NULL";

      // Store transformation,  apply it to morph file
      MITK_INFO << "----------Registering moving image to reference----------";


      MITK_ERROR << "REF COUNT MOVING 1 " << movingImage->GetReferenceCount();
      //mitk::RegistrationWrapper::GetTransformation(refImage, movingImage, transf, offset, referenceMask);
      MITK_ERROR << "REF COUNT MOVING 2 " << movingImage->GetReferenceCount();

      mitk::Image::Pointer regImage = mitk::RegistrationWrapper::ApplyTransformationToImage(movingImage, transf,offset, NULL); // , resampleImage
      MITK_ERROR << "REF COUNT MOVING 3 " << movingImage->GetReferenceCount();
      savePathAndFileName = GetSavePath(outputPath, fileMorphName);
      std::string fileType = itksys::SystemTools::GetFilenameExtension(fileMorphName);
      SaveImage(savePathAndFileName,regImage,fileType );
    }

    if (!silent)
    {
      std::cout << "<filter-progress-text progress=\"" <<
                   (float)i / (float)movingImagesList.size()
                << "\" >.</filter-progress-text>";
    }

    // Now parse all derived resource and apply the above calculated transformation to them
    // ------------------------------------------------------------------------------------

    FileListType fList = CreateDerivedFileList(fileMorphName, movingImgPattern,derPatterns);
    if (fList.size() > 0)
      MITK_INFO << "----------DERIVED RESOURCES ---------";
    for (unsigned int j=0; j < fList.size(); j++)
    {
      derivedResourceFilename = fList.at(j);
      MITK_INFO << "----Processing derived resource " << derivedResourceFilename << " ...";
      mitk::Image::Pointer derivedMovingResource = mitk::IOUtil::LoadImage(derivedResourceFilename);
      // Apply transformation to derived resource, treat derived resource as binary
      mitk::Image::Pointer regDerImage = mitk::RegistrationWrapper::ApplyTransformationToImage(derivedMovingResource->Clone(), transf,offset, NULL, true);

      savePathAndFileName = GetSavePath(outputPath, derivedResourceFilename);
      std::string fileType = itksys::SystemTools::GetFilenameExtension(derivedResourceFilename);

      SaveImage(savePathAndFileName,regDerImage,fileType );
    }
  }

  if (!silent)
    std::cout << "<filter-end/>";
  return EXIT_SUCCESS;
}

RegisterDiffusionMiniApp(BatchedFolderRegistration);
