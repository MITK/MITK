#include "mitkPAIOUtil.h"

#include "mitkIOUtil.h"
#include "mitkImageReadAccessor.h"

#include <string>
#include <sstream>
#include <vector>

#include "mitkPAComposedVolume.h"
#include "mitkPASlicedVolumeGenerator.h"
#include "mitkPANoiseGenerator.h"
#include "mitkPAVolumeManipulator.h"
#include <mitkProperties.h>
#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>

static std::vector<int> splitString(const std::string &s, const char* delim) {
  std::vector<int> elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, *delim))
  {
    int numb;
    std::stringstream(item) >> numb;
    elems.push_back(numb);
  }
  return elems;
}

bool mitk::pa::IOUtil::DoesFileHaveEnding(std::string const &fullString, std::string const &ending) {
  if (fullString.length() == 0 || ending.length() == 0 || fullString.length() < ending.length())
    return false;

  return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
}

mitk::pa::IOUtil::IOUtil() {}

mitk::pa::IOUtil::~IOUtil() {}

mitk::pa::Volume::Pointer mitk::pa::IOUtil::LoadNrrd(std::string filename, double blur)
{
  if (filename.empty() || filename == "")
    return nullptr;

  auto inputImage = mitk::IOUtil::Load<mitk::Image>(filename);

  if (inputImage.IsNull())
    return nullptr;

  int xDim = inputImage->GetDimensions()[1];
  int yDim = inputImage->GetDimensions()[0];
  int zDim = inputImage->GetDimensions()[2];

  mitk::ImageReadAccessor readAccess(inputImage, inputImage->GetVolumeData(0));

  auto* dataArray = new double[xDim*yDim*zDim];
  const auto* srcData = (const double*)readAccess.GetData();
  memcpy(dataArray, srcData, xDim*yDim*zDim * sizeof(double));

  auto returnImage = mitk::pa::Volume::New(dataArray, xDim, yDim, zDim);

  mitk::pa::VolumeManipulator::GaussianBlur3D(returnImage, blur);

  return returnImage;
}

std::map<mitk::pa::IOUtil::Position, mitk::pa::Volume::Pointer>
mitk::pa::IOUtil::LoadFluenceContributionMaps(std::string foldername, double blur, int* progress, bool doLog10)
{
  std::map<IOUtil::Position, Volume::Pointer> resultMap;

  itk::Directory::Pointer directoryHandler = itk::Directory::New();
  directoryHandler->Load(foldername.c_str());
  for (unsigned int fileIndex = 0, numFiles = directoryHandler->GetNumberOfFiles(); fileIndex < numFiles; ++fileIndex)
  {
    std::string filename = std::string(directoryHandler->GetFile(fileIndex));
    if (itksys::SystemTools::FileIsDirectory(filename))
      continue;

    if (!DoesFileHaveEnding(filename, ".nrrd"))
      continue;

    size_t s = filename.find("_p");
    size_t e = filename.find("Fluence", s);
    std::string sub = filename.substr(s + 2, e - s - 2);
    std::vector<int> coords = splitString(sub, ",");

    if (coords.size() != 3)
    {
      MITK_ERROR << "Some of the data to read was corrupted or did not match the " <<
        "naming pattern *_pN,N,NFluence*.nrrd";
      mitkThrow() << "Some of the data to read was corrupted or did not match the" <<
        " naming pattern *_pN,N,NFluence*.nrrd";
    }
    else
    {
      MITK_DEBUG << "Extracted coords: " << coords[0] << "|" << coords[1] << "|" << coords[2] << " from string " << sub;
      Volume::Pointer nrrdFile = LoadNrrd(foldername + filename, blur);
      if (doLog10)
        VolumeManipulator::Log10Image(nrrdFile);
      resultMap[Position{ coords[0], coords[2] }] = nrrdFile;
      *progress = *progress + 1;
    }
  }

  return resultMap;
}

int mitk::pa::IOUtil::GetNumberOfNrrdFilesInDirectory(std::string directory)
{
  return GetListOfAllNrrdFilesInDirectory(directory).size();
}

std::vector<std::string> mitk::pa::IOUtil::GetListOfAllNrrdFilesInDirectory(std::string directory, bool keepFileFormat)
{
  std::vector<std::string> filenames;
  itk::Directory::Pointer directoryHandler = itk::Directory::New();
  directoryHandler->Load(directory.c_str());
  for (unsigned int fileIndex = 0, numFiles = directoryHandler->GetNumberOfFiles(); fileIndex < numFiles; ++fileIndex)
  {
    std::string filename = std::string(directoryHandler->GetFile(fileIndex));
    if (itksys::SystemTools::FileIsDirectory(filename))
      continue;

    if (!DoesFileHaveEnding(filename, ".nrrd"))
      continue;

    if (keepFileFormat)
    {
      filenames.push_back(filename);
    }
    else
    {
      filenames.push_back(filename.substr(0, filename.size() - 5));
    }
  }

  return filenames;
}

std::vector<std::string> mitk::pa::IOUtil::GetAllChildfoldersFromFolder(std::string folderPath)
{
  std::vector<std::string> returnVector;

  itksys::Directory directoryHandler;
  directoryHandler.Load(folderPath.c_str());
  for (unsigned int fileIndex = 0, numFiles = directoryHandler.GetNumberOfFiles(); fileIndex < numFiles; ++fileIndex)
  {
    std::string foldername = std::string(directoryHandler.GetFile(fileIndex));
    std::string filename = folderPath + "/" + foldername;
    if (itksys::SystemTools::FileIsDirectory(filename))
    {
      if (foldername != std::string(".") && foldername != std::string(".."))
      {
        MITK_INFO << filename;
        returnVector.push_back(filename);
      }
      continue;
    }

    //If there is a nrrd file in the directory we assume that a bottom level directory was chosen.
    if (DoesFileHaveEnding(filename, ".nrrd"))
    {
      returnVector.clear();
      returnVector.push_back(folderPath);
      return returnVector;
    }
  }

  return returnVector;
}

mitk::pa::InSilicoTissueVolume::Pointer mitk::pa::IOUtil::LoadInSilicoTissueVolumeFromNrrdFile(std::string nrrdFile)
{
  MITK_INFO << "Initializing ComposedVolume by nrrd...";
  auto inputImage = mitk::IOUtil::Load<mitk::Image>(nrrdFile);

  auto tissueParameters = TissueGeneratorParameters::New();

  unsigned int xDim = inputImage->GetDimensions()[1];
  unsigned int yDim = inputImage->GetDimensions()[0];
  unsigned int zDim = inputImage->GetDimensions()[2];
  tissueParameters->SetXDim(xDim);
  tissueParameters->SetYDim(yDim);
  tissueParameters->SetZDim(zDim);

  double xSpacing = inputImage->GetGeometry(0)->GetSpacing()[1];
  double ySpacing = inputImage->GetGeometry(0)->GetSpacing()[0];
  double zSpacing = inputImage->GetGeometry(0)->GetSpacing()[2];

  if ((xSpacing - ySpacing) > mitk::eps || (xSpacing - zSpacing) > mitk::eps || (ySpacing - zSpacing) > mitk::eps)
  {
    throw mitk::Exception("Cannot handle unequal spacing.");
  }

  tissueParameters->SetVoxelSpacingInCentimeters(xSpacing);

  mitk::PropertyList::Pointer propertyList = inputImage->GetPropertyList();

  mitk::ImageReadAccessor readAccess0(inputImage, inputImage->GetVolumeData(0));
  auto* m_AbsorptionArray = new double[xDim*yDim*zDim];
  memcpy(m_AbsorptionArray, readAccess0.GetData(), xDim*yDim*zDim * sizeof(double));
  auto absorptionVolume = Volume::New(m_AbsorptionArray, xDim, yDim, zDim);

  mitk::ImageReadAccessor readAccess1(inputImage, inputImage->GetVolumeData(1));
  auto*  m_ScatteringArray = new double[xDim*yDim*zDim];
  memcpy(m_ScatteringArray, readAccess1.GetData(), xDim*yDim*zDim * sizeof(double));
  auto scatteringVolume = Volume::New(m_ScatteringArray, xDim, yDim, zDim);

  mitk::ImageReadAccessor readAccess2(inputImage, inputImage->GetVolumeData(2));
  auto*  m_AnisotropyArray = new double[xDim*yDim*zDim];
  memcpy(m_AnisotropyArray, readAccess2.GetData(), xDim*yDim*zDim * sizeof(double));
  auto anisotropyVolume = Volume::New(m_AnisotropyArray, xDim, yDim, zDim);

  Volume::Pointer segmentationVolume;

  if (inputImage->GetDimension() == 4)
  {
    mitk::ImageReadAccessor readAccess3(inputImage, inputImage->GetVolumeData(3));
    auto*  m_SegmentationArray = new double[xDim*yDim*zDim];
    memcpy(m_SegmentationArray, readAccess3.GetData(), xDim*yDim*zDim * sizeof(double));
    segmentationVolume = Volume::New(m_SegmentationArray, xDim, yDim, zDim);
  }

  return mitk::pa::InSilicoTissueVolume::New(absorptionVolume, scatteringVolume,
    anisotropyVolume, segmentationVolume, tissueParameters, propertyList);
}

mitk::pa::FluenceYOffsetPair::Pointer mitk::pa::IOUtil::LoadFluenceSimulation(std::string fluenceSimulation)
{
  MITK_INFO << "Adding slice...";

  auto inputImage = mitk::IOUtil::Load<mitk::Image>(fluenceSimulation);
  mitk::ImageReadAccessor readAccess0(inputImage, inputImage->GetVolumeData(0));

  unsigned int xDim = inputImage->GetDimensions()[1];
  unsigned int yDim = inputImage->GetDimensions()[0];
  unsigned int zDim = inputImage->GetDimensions()[2];
  int size = xDim*yDim*zDim;
  auto* fluenceArray = new double[size];
  memcpy(fluenceArray, readAccess0.GetData(), size * sizeof(double));

  auto yOffsetProperty = inputImage->GetProperty("y-offset");

  if (yOffsetProperty.IsNull())
    mitkThrow() << "No \"y-offset\" property found in fluence file!";

  std::string yOff = yOffsetProperty->GetValueAsString();
  MITK_INFO << "Reading y Offset: " << yOff;
#ifdef __linux__
  std::replace(yOff.begin(), yOff.end(), '.', ',');
#endif // __linux__
  double yOffset = std::stod(yOff);
  MITK_INFO << "Converted offset " << yOffset;
  return mitk::pa::FluenceYOffsetPair::New(mitk::pa::Volume::New(fluenceArray, xDim, yDim, zDim), yOffset);
}
