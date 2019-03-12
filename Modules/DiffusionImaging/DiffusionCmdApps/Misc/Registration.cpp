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


// CTK
#include "mitkCommandLineParser.h"

#include <mitkIOUtil.h>
#include <mitkRegistrationWrapper.h>
#include <mitkImage.h>
#include <mitkImageCast.h>
#include <mitkImageReadAccessor.h>
#include <mitkITKImageImport.h>
#include <mitkImageTimeSelector.h>
// ITK
#include <itksys/SystemTools.hxx>
#include <itkDirectory.h>
#include "itkLinearInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkIdentityTransform.h"
#include "itkResampleImageFilter.h"
#include "itkNrrdImageIO.h"

typedef std::vector<std::string> FileListType;
typedef itk::Image<double, 3> InputImageType;

static mitk::Image::Pointer ExtractFirstTS(mitk::Image* image, std::string fileType)
{
  if (fileType == ".dwi")
    return image;
  mitk::ImageTimeSelector::Pointer selector = mitk::ImageTimeSelector::New();
  selector->SetInput(image);
  selector->SetTimeNr(0);
  selector->UpdateLargestPossibleRegion();
  mitk::Image::Pointer img =selector->GetOutput()->Clone();
  return img;
}


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
      if (filename.length() <= postfix.length() )
        continue;
      if (filename.substr(filename.length() -postfix.length() ) == postfix)
        fileList.push_back(filename);
    }
  }
  return fileList;
}

static std::string GetSavePath(std::string outputFolder, std::string fileName)
{
  std::string fileType = itksys::SystemTools::GetFilenameExtension(fileName);
  std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(fileName);

  std::string savePathAndFileName = outputFolder +fileStem + fileType;

  return savePathAndFileName;
}


static mitk::Image::Pointer ResampleBySpacing(mitk::Image *input, float *spacing)
{
  InputImageType::Pointer itkImage = InputImageType::New();
  CastToItkImage(input,itkImage);

  /**
   * 1) Resampling
   *
   */
  // Identity transform.
  // We don't want any transform on our image except rescaling which is not
  // specified by a transform but by the input/output spacing as we will see
  // later.
  // So no transform will be specified.
  typedef itk::IdentityTransform<double, 3> T_Transform;

  // The resampler type itself.
  typedef itk::ResampleImageFilter<InputImageType, InputImageType>  T_ResampleFilter;

  // Prepare the resampler.
  // Instantiate the transform and specify it should be the id transform.
  T_Transform::Pointer _pTransform = T_Transform::New();
  _pTransform->SetIdentity();

  // Instantiate the resampler. Wire in the transform and the interpolator.
  T_ResampleFilter::Pointer _pResizeFilter = T_ResampleFilter::New();
  _pResizeFilter->SetTransform(_pTransform);

  // Set the output origin.
  _pResizeFilter->SetOutputOrigin(itkImage->GetOrigin());

  // Compute the size of the output.
  // The size (# of pixels) in the output is recomputed using
  // the ratio of the input and output sizes.
  InputImageType::SpacingType inputSpacing = itkImage->GetSpacing();
  InputImageType::SpacingType outputSpacing;
  const InputImageType::RegionType& inputSize = itkImage->GetLargestPossibleRegion();

  InputImageType::SizeType outputSize;
  typedef InputImageType::SizeType::SizeValueType SizeValueType;

  // Set the output spacing.
  outputSpacing[0] = spacing[0];
  outputSpacing[1] = spacing[1];
  outputSpacing[2] = spacing[2];

  outputSize[0] = static_cast<SizeValueType>(inputSize.GetSize()[0] * inputSpacing[0] / outputSpacing[0] + .5);
  outputSize[1] = static_cast<SizeValueType>(inputSize.GetSize()[1] * inputSpacing[1] / outputSpacing[1] + .5);
  outputSize[2] = static_cast<SizeValueType>(inputSize.GetSize()[2] * inputSpacing[2] / outputSpacing[2] + .5);

  _pResizeFilter->SetOutputSpacing(outputSpacing);
  _pResizeFilter->SetSize(outputSize);

  typedef itk::Function::WelchWindowFunction<4> WelchWindowFunction;
  typedef itk::WindowedSincInterpolateImageFunction< InputImageType, 4,WelchWindowFunction> WindowedSincInterpolatorType;
  WindowedSincInterpolatorType::Pointer sinc_interpolator = WindowedSincInterpolatorType::New();

  _pResizeFilter->SetInterpolator(sinc_interpolator);

  // Specify the input.
  _pResizeFilter->SetInput(itkImage);
  _pResizeFilter->Update();

  mitk::Image::Pointer image = mitk::Image::New();
  image->InitializeByItk(_pResizeFilter->GetOutput());
  mitk::GrabItkImageMemory( _pResizeFilter->GetOutput(), image);

  return image;
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

/// Copy derived resources from first time step. Append _reg tag, but leave data untouched.
static void CopyResources(FileListType fileList, std::string outputPath)
{
  for (unsigned int j=0; j < fileList.size(); j++)
  {
    std::string derivedResourceFilename = fileList.at(j);
    std::string fileType = itksys::SystemTools::GetFilenameExtension(derivedResourceFilename);
    std::string fileStem = itksys::SystemTools::GetFilenameWithoutExtension(derivedResourceFilename);
    std::string savePathAndFileName = outputPath +fileStem + "." + fileType;
    MITK_INFO << "Copy resource " << savePathAndFileName;
    mitk::Image::Pointer resImage = ExtractFirstTS(mitk::IOUtil::Load<mitk::Image>(derivedResourceFilename), fileType);
    mitk::IOUtil::Save(resImage, savePathAndFileName);
  }
}

int main( int argc, char* argv[] )
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--","-");

  parser.setTitle("Folder Registration");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("For detail description see http://docs.mitk.org/nightly/DiffusionMiniApps.html");
  parser.setContributor("MIC");

  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help", "Show this help text");
  //parser.addArgument("usemask", "u", QVariant::Bool, "Use segmentations (derived resources) to exclude areas from registration metrics");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "Input folder",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "Output folder (ending with /)",us::Any(),false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("fixed", "f", mitkCommandLineParser::String, "Fixed images:", "Suffix for fixed image (if none is supplied first file matching moving pattern is chosen)",us::Any(),true);
  parser.addArgument("moving", "m", mitkCommandLineParser::String, "Moving images:", "Suffix for moving images",us::Any(),false);
  parser.addArgument("derived", "d", mitkCommandLineParser::String, "Derived resources:", "Derived resources suffixes (replaces suffix for moving images); comma separated",us::Any(),true);
  parser.addArgument("silent", "s", mitkCommandLineParser::Bool, "Silent:", "No xml progress output.");
  parser.addArgument("resample", "r", mitkCommandLineParser::String, "Resample (x,y,z)mm:", "Resample provide x,y,z spacing in mm (e.g. -r 1,1,3), is not applied to tensor data",us::Any());
  parser.addArgument("binary", "b", mitkCommandLineParser::Bool, "Binary:", "Speficies that derived resource are binary (interpolation using nearest neighbor)",us::Any());
  parser.addArgument("correct-origin", "c", mitkCommandLineParser::Bool, "Origin correction:", "Correct for large origin displacement. Use switch when you reveive:  Joint PDF summed to zero ",us::Any());
  parser.addArgument("sinc-int", "s", mitkCommandLineParser::Bool, "Windowed-sinc interpolation:", "Use windowed-sinc interpolation (3) instead of linear interpolation ",us::Any());


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Handle special arguments
  bool silent = false;
  bool isBinary = false;
  bool alignOrigin = false;

  {
    if (parsedArgs.size() == 0)
    {
      return EXIT_FAILURE;
    }

    if (parsedArgs.count("silent"))
      silent = true;

    if (parsedArgs.count("binary"))
      isBinary = true;

    if (parsedArgs.count("correct-origin"))
      alignOrigin = true;

    // Show a help message
    if ( parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }
  }
  std::string refPattern = "";
  bool useFirstMoving = false;
  std::string movingImgPattern = us::any_cast<std::string>(parsedArgs["moving"]);

  if (parsedArgs.count("fixed"))
  {
    refPattern = us::any_cast<std::string>(parsedArgs["fixed"]);
  }
  else
  {
    useFirstMoving = true;
    refPattern = movingImgPattern;
  }

  std::string outputPath = us::any_cast<std::string>(parsedArgs["o"]);

  std::string inputPath = us::any_cast<std::string>(parsedArgs["i"]);
  //QString resampleReference = parsedArgs["resample"].toString();
  //bool maskTumor = parsedArgs["usemask"].toBool();

  // if derived sources pattern is provided, populate QStringList with possible filename postfixes
  std::vector<std::string> derPatterns;

  if (parsedArgs.count("derived") || parsedArgs.count("d") )
  {
    std::string arg =  us::any_cast<std::string>(parsedArgs["derived"]);
    derPatterns = split(arg ,',');
  }


  std::vector<std::string> spacings;
  float spacing[] = { 0.0f, 0.0f, 0.0f };
  bool doResampling = false;
  if (parsedArgs.count("resample") || parsedArgs.count("d") )
  {
    std::string arg =  us::any_cast<std::string>(parsedArgs["resample"]);
    spacings = split(arg ,',');
    spacing[0] = atoi(spacings.at(0).c_str());
    spacing[1] = atoi(spacings.at(1).c_str());
    spacing[2] = atoi(spacings.at(2).c_str());
    doResampling = true;
  }

  MITK_INFO << "Input Folder : " << inputPath;
  MITK_INFO << "Looking for reference image ...";
  FileListType referenceFileList = CreateFileList(inputPath,refPattern);

  if ((!useFirstMoving && referenceFileList.size() != 1) || (useFirstMoving && referenceFileList.size() == 0))
  {
    MITK_ERROR << "None or more than one possible reference images (" << refPattern <<") found. Exiting." << referenceFileList.size();
    MITK_INFO  << "Choose a fixed arguement that is unique in the given folder!";
    return EXIT_FAILURE;
  }

  std::string referenceFileName = referenceFileList.at(0);

  MITK_INFO << "Loading Reference (fixed) image: " << referenceFileName;
  std::string fileType = itksys::SystemTools::GetFilenameExtension(referenceFileName);
  mitk::Image::Pointer refImage = ExtractFirstTS(mitk::IOUtil::Load<mitk::Image>(referenceFileName), fileType);
  mitk::Image::Pointer resampleReference = nullptr;
  if (doResampling)
  {
    refImage = ResampleBySpacing(refImage,spacing);
    resampleReference = refImage;
  }

  if (refImage.IsNull())
    MITK_ERROR << "Loaded fixed image is nullptr";

  // Copy reference image to destination
  std::string savePathAndFileName = GetSavePath(outputPath, referenceFileName);

  mitk::IOUtil::Save(refImage, savePathAndFileName);

  // Copy all derived resources also to output folder, adding _reg suffix
  referenceFileList = CreateDerivedFileList(referenceFileName, movingImgPattern,derPatterns);
  CopyResources(referenceFileList, outputPath);

  std::string derivedResourceFilename;
  mitk::Image::Pointer referenceMask = nullptr; // union of all segmentations

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
      std::string fileType = itksys::SystemTools::GetFilenameExtension(fileMorphName);
      mitk::Image::Pointer movingImage = ExtractFirstTS(mitk::IOUtil::Load<mitk::Image>(fileMorphName), fileType);

      if (movingImage.IsNull())
        MITK_ERROR << "Loaded moving image is nullptr";

      // Store transformation,  apply it to morph file
      MITK_INFO << "----------Registering moving image to reference----------";

      mitk::RegistrationWrapper::GetTransformation(refImage, movingImage, transf, offset, alignOrigin, referenceMask);
      mitk::RegistrationWrapper::ApplyTransformationToImage(movingImage, transf,offset, resampleReference); // , resampleImage

      savePathAndFileName = GetSavePath(outputPath, fileMorphName);
      if (fileType == ".dwi")
        fileType = "dwi";

      {
        mitk::ImageReadAccessor readAccess(movingImage);

        if (readAccess.GetData() == nullptr)
          MITK_INFO <<"POST DATA is null";
      }

      mitk::IOUtil::Save(movingImage, savePathAndFileName);
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
      MITK_INFO << "----Processing derived resorce " << derivedResourceFilename << " ...";
      std::string fileType = itksys::SystemTools::GetFilenameExtension(derivedResourceFilename);
      mitk::Image::Pointer derivedMovingResource = ExtractFirstTS(mitk::IOUtil::Load<mitk::Image>(derivedResourceFilename), fileType);
      // Apply transformation to derived resource, treat derived resource as binary
      mitk::RegistrationWrapper::ApplyTransformationToImage(derivedMovingResource, transf,offset, resampleReference,isBinary);

      savePathAndFileName = GetSavePath(outputPath, derivedResourceFilename);
      mitk::IOUtil::Save(derivedMovingResource, savePathAndFileName);
    }
  }

  if (!silent)
    std::cout << "<filter-end/>";
  return EXIT_SUCCESS;
}
