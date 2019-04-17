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
// MITK
#include <mitkIOUtil.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImageToItk.h>
#include <mitkPixelType.h>
#include <mitkPlaneGeometry.h>

// ITK
#include "itkImageDuplicator.h"
#include "itkPermuteAxesImageFilter.h"
#include "itkTileImageFilter.h"
#include <itkExtractImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkRGBPixel.h>
#include <itkResampleImageFilter.h>
#include <itkVTKImageImport.h>

#include <itkDirectory.h>
#include <itksys/SystemTools.hxx>
// VTK
#include <vtkCellArray.h>
#include <vtkFreeTypeStringToImage.h>
#include <vtkImageData.h>
#include <vtkTextProperty.h>
#include <vtkUnicodeString.h>

// MITK
#include <mitkCommandLineParser.h>

#include "itkRescaleIntensityImageFilter.h"

#include <typeExtension.h>

using namespace std;

typedef unsigned short PixelType;
typedef double InputPixelType;
typedef itk::Image<itk::RGBPixel<PixelType>, 2> RGB2DImage;
typedef itk::RGBPixel<PixelType> RGBPixelType;

typedef std::vector<std::string> FileList;
typedef std::vector<mitk::Image::Pointer> ImageList;

/// Create list of all files in provided folder ending with same postfix
static FileList CreateFileList(std::string folder, std::string postfix)
{
  itk::Directory::Pointer dir = itk::Directory::New();
  FileList fileList;

  if (dir->Load(folder.c_str()))
  {
    int n = dir->GetNumberOfFiles();
    for (int r = 0; r < n; r++)
    {
      std::string filename = dir->GetFile(r);
      if (filename == "." || filename == "..")
        continue;
      filename = folder + filename;
      if (!itksys::SystemTools::FileExists(filename.c_str()))
        continue;

      if (postfix.compare(filename.substr(filename.length() - postfix.length())) == 0)
        fileList.push_back(filename);
    }
  }
  std::sort(fileList.begin(), fileList.end());
  return fileList;
}

class mitkProgressionVisualization
{
public:
  mitkProgressionVisualization() {}
  /**
   * @brief ConvertToRGBImage converts a gray image to RGB by filling all three channels with the gray intensity
   * @param grayImage
   * @return
   */
  mitk::Image::Pointer ConvertToRGBImage(mitk::Image::Pointer grayImage)
  {
    mitk::Image::Pointer rgbImage = mitk::Image::New();
    unsigned int *dim = grayImage->GetDimensions();
    rgbImage->Initialize(mitk::MakePixelType<PixelType, RGBPixelType, 3>(), 3, dim);
    rgbImage->SetGeometry(grayImage->GetGeometry());

    itk::Image<InputPixelType, 3>::Pointer itkGrayImage = itk::Image<InputPixelType, 3>::New();
    mitk::CastToItkImage(grayImage, itkGrayImage);

    mitk::ImagePixelWriteAccessor<RGBPixelType, 3> writeAcc(rgbImage);

    typedef itk::RescaleIntensityImageFilter<itk::Image<InputPixelType, 3>, itk::Image<PixelType, 3>> RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(itkGrayImage);
    rescaleFilter->SetOutputMinimum(0);
    rescaleFilter->SetOutputMaximum(255 * 255);
    rescaleFilter->Update();

    itk::Index<3> idx;
    RGBPixelType value;

    // Fill rgb image with gray values
    for (idx[2] = 0; (unsigned int)idx[2] < dim[2]; idx[2]++)
    {
      for (idx[1] = 0; (unsigned int)idx[1] < dim[1]; idx[1]++)
      {
        for (idx[0] = 0; (unsigned int)idx[0] < dim[0]; idx[0]++)
        {
          value.Fill(rescaleFilter->GetOutput()->GetPixel(idx));
          writeAcc.SetPixelByIndex(idx, value);
        }
      }
    }
    return rgbImage;
  }

  RGB2DImage::Pointer TextAsImage(std::string text)
  {
    vtkSmartPointer<vtkImageData> textImage = vtkSmartPointer<vtkImageData>::New();
    vtkSmartPointer<vtkFreeTypeStringToImage> freetype = vtkSmartPointer<vtkFreeTypeStringToImage>::New();
    vtkSmartPointer<vtkTextProperty> prop = vtkSmartPointer<vtkTextProperty>::New();
    float color[3] = {1, 1, 1};
    float opacity = 1.0;
    prop->SetColor(color[0], color[1], color[2]);
    prop->SetFontSize(40);
    prop->SetOpacity(opacity);
    textImage->AllocateScalars(VTK_UNSIGNED_SHORT, 3);
    freetype->RenderString(prop, vtkUnicodeString::from_utf8(text.c_str()), 72, textImage);
    textImage->Modified();

    int *extent = textImage->GetExtent();

    RGB2DImage::Pointer itkImage = RGB2DImage::New();

    RGB2DImage::IndexType start;
    start.Fill(0);
    RGB2DImage::SizeType size;
    size[0] = extent[1];
    size[1] = extent[3];
    size[2] = 0;

    RGB2DImage::RegionType region(start, size);
    itkImage->SetRegions(region);
    itkImage->Allocate();
    RGB2DImage::IndexType idx;

    for (unsigned int y = 0; y < size[1]; y++)
    {
      for (unsigned int x = 0; x < size[0]; x++)
      {
        PixelType *pixel = static_cast<PixelType *>(textImage->GetScalarPointer(x, y, 0));
        RGBPixelType values;
        values.Fill(0);
        values[0] = pixel[1];
        values[1] = pixel[1];
        values[2] = pixel[1];
        idx.Fill(0);
        idx[0] = x;
        idx[1] = y;

        itkImage->SetPixel(idx, values);
      }
    }

    typedef itk::PermuteAxesImageFilter<RGB2DImage> PermuteAxesImageFilterType;

    itk::FixedArray<unsigned int, 2> order;
    order[0] = 1;
    order[1] = 0;

    PermuteAxesImageFilterType::Pointer permuteAxesFilter = PermuteAxesImageFilterType::New();
    permuteAxesFilter->SetInput(itkImage);
    permuteAxesFilter->SetOrder(order);
    permuteAxesFilter->Update();

    RGB2DImage::Pointer itkResultImage = RGB2DImage::New();
    itkResultImage->SetRegions(region);
    itkResultImage->Allocate();

    itkResultImage->Graft(permuteAxesFilter->GetOutput());

    return itkResultImage;
  }

  /**
   * @brief AddColouredOverlay - Overlays the rgbImage with an coloured overlay
   *
   * For all positions in overlayImage not zero, its value is multiplied the the colour value and added
   * to the rgbImage.
   *
   * @param rgbImage - input rgbImage to which the coloured overlay is added
   * @param overlayImage
   * @param color
   */
  void AddColouredOverlay(mitk::Image::Pointer rgbImage, mitk::Image::Pointer overlayImage, mitk::Color color)
  {
    unsigned int *dim = rgbImage->GetDimensions();
    itk::Image<PixelType, 3>::Pointer itkOverlayImage = itk::Image<PixelType, 3>::New();
    mitk::CastToItkImage(overlayImage.GetPointer(), itkOverlayImage);
    mitk::ImagePixelWriteAccessor<RGBPixelType, 3> writeAcc(rgbImage);

    itk::Index<3> idx;
    itk::RGBPixel<PixelType> value;
    unsigned short overlayVal = 0;
    // Fill rgb image with gray values
    for (idx[2] = 0; (unsigned int)idx[2] < dim[2]; idx[2]++)
    {
      for (idx[1] = 0; (unsigned int)idx[1] < dim[1]; idx[1]++)
      {
        for (idx[0] = 0; (unsigned int)idx[0] < dim[0]; idx[0]++)
        {
          overlayVal = 255 * itkOverlayImage->GetPixel(idx);
          value = writeAcc.GetPixelByIndex(idx);
          value[0] = std::min((int)(value[0] + overlayVal * color[0]), 254 * 255);
          value[1] = std::min((int)(value[1] + overlayVal * color[1]), 254 * 255);
          value[2] = std::min((int)(value[2] + overlayVal * color[2]), 254 * 255);
          writeAcc.SetPixelByIndex(idx, value);
        }
      }
    }
  }

  itk::Image<itk::RGBPixel<PixelType>, 2>::Pointer ExtractSlice(
    itk::Image<itk::RGBPixel<PixelType>, 3>::Pointer itkImage, unsigned int sliceNo)
  {
    typedef itk::Image<RGBPixelType, 3> InputImageType;
    typedef itk::Image<RGBPixelType, 2> OutputImageType;

    int dim[3];
    dim[0] = itkImage->GetLargestPossibleRegion().GetSize()[0];
    dim[1] = itkImage->GetLargestPossibleRegion().GetSize()[1];
    dim[2] = itkImage->GetLargestPossibleRegion().GetSize()[2];

    itk::Index<3> desiredStart;
    itk::Size<3> desiredSize;

    // case AXIAL: // 3rd dimension fixed
    desiredStart.Fill(0);
    desiredStart[2] = sliceNo;
    desiredSize.Fill(0);
    desiredSize[0] = dim[0];
    desiredSize[1] = dim[1];
    desiredSize[2] = 0;

    itk::ImageRegion<3> desiredRegion(desiredStart, desiredSize);

    // Extract slice
    itk::ExtractImageFilter<InputImageType, OutputImageType>::Pointer extractSlice =
      itk::ExtractImageFilter<InputImageType, OutputImageType>::New();
    extractSlice->SetInput(itkImage);
    extractSlice->SetExtractionRegion(desiredRegion);
    extractSlice->SetDirectionCollapseToIdentity();
    extractSlice->Update();

    return extractSlice->GetOutput();
  }
};

static std::string GetName(std::string fileName, std::string suffix = "_T2.nrrd")
{
  fileName = itksys::SystemTools::GetFilenameName(fileName);
  return fileName.substr(0, fileName.length() - suffix.length() - 11); // 10 = date length
}

static std::string GetDate(std::string fileName, std::string suffix = "_T2.nrrd")
{
  fileName = itksys::SystemTools::GetFilenameName(fileName);
  fileName = fileName.substr(fileName.length() - suffix.length() - 10, 10); // 10 = date length
  return fileName;
}

static ImageList LoadPreprocessedFiles(FileList files)
{
  ImageList images;
  for (unsigned int i = 0; i < files.size(); ++i)
  {
    images.push_back(mitk::IOUtil::Load<mitk::Image>(files.at(i)));
  }
  return images;
}

int main(int argc, char *argv[])
{
  // Parse Command-Line Arguments
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  parser.setTitle("Tumor Progression Mapping");
  parser.setCategory("Preprocessing Tools");
  parser.setContributor("MBI");
  parser.setDescription("Convert a set of co-registered and resampled follow-up images into a 2D png overview (and "
                        "optionally in a 4D NRRD Volume).\nNaming convecntion of files is "
                        "IDENTIFIER_YYYY-MM-DD_MODALITY.nrrd");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::Directory, "Input folder containing all follow ups", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file (PNG)", "", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("blanked", "b", mitkCommandLineParser::Bool, "Only Display Morphology");
  parser.addArgument("morphology", "m", mitkCommandLineParser::String, "Morphology postfix.", "_T2.nrrd");
  parser.addArgument(
    "segmentation", "s", mitkCommandLineParser::String, "Segmentation postfix. Default: _GTV.nrrd", "_GTV.nrrd");
  parser.addArgument("4dVolume", "v", mitkCommandLineParser::File, "Filepath to merged 4d NRRD Volume.", "", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument(
    "skip", "k", mitkCommandLineParser::Int, "Number of slices to be skipped from top and from button (Default 0)");
  parser.addArgument(
    "interval", "n", mitkCommandLineParser::Int, "1 - for all slices, 2 - every second, 3 - every third ...");
  parser.addArgument("opacity", "c", mitkCommandLineParser::Float, "Opacity of overlay [0,1] invisible -> visible");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_SUCCESS;

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string outputFile;
  std::string inputFolder;

  if (parsedArgs.count("input") || parsedArgs.count("i"))
  {
    inputFolder = us::any_cast<string>(parsedArgs["input"]) + "/";
  }

  if (parsedArgs.count("output") || parsedArgs.count("o"))
  {
    outputFile = us::any_cast<string>(parsedArgs["output"]);
  }

  int skip = 0;
  int interval = 1;
  float opacity = .3;

  if (parsedArgs.count("skip") || parsedArgs.count("k"))
  {
    skip = us::any_cast<int>(parsedArgs["skip"]);
  }

  if (parsedArgs.count("interval") || parsedArgs.count("n"))
  {
    interval = us::any_cast<int>(parsedArgs["interval"]);
  }

  if (parsedArgs.count("opacity") || parsedArgs.count("c"))
  {
    opacity = us::any_cast<float>(parsedArgs["opacity"]);
  }

  FileList morphFiles;
  FileList segFiles;

  std::string refPattern;
  std::string segPattern;

  if (parsedArgs.count("morphology") || parsedArgs.count("m"))
  {
    refPattern = us::any_cast<std::string>(parsedArgs["morphology"]);
  }
  else
    return EXIT_FAILURE;

  if (parsedArgs.count("segmentation") || parsedArgs.count("s"))
  {
    segPattern = us::any_cast<std::string>(parsedArgs["segmentation"]);
  }

  bool blank = false;
  if (parsedArgs.count("blanked") || parsedArgs.count("b"))
  {
    blank = true;
  }
  /// END Parsing CL Options
  typedef itk::Image<RGBPixelType, 2> OutputImageType;
  typedef itk::Image<RGBPixelType, 3> InputImageType;

  mitkProgressionVisualization progressVis;

  morphFiles = CreateFileList(inputFolder, refPattern);
  segFiles = CreateFileList(inputFolder, segPattern);

  ImageList morphImages = LoadPreprocessedFiles(morphFiles);
  ImageList segImages;
  if (segFiles.size() > 0 && blank == false)
    segImages = LoadPreprocessedFiles(segFiles);

  mitk::Image::Pointer rgbImage;

  // define color for overlay image
  mitk::Color color;
  color.Fill(0);
  color[0] = 200 * opacity;
  color[1] = 0;

  //  Set up itk time image filter to contain 0..N-1 images
  itk::TileImageFilter<OutputImageType, OutputImageType>::Pointer tileFilter =
    itk::TileImageFilter<OutputImageType, OutputImageType>::New();
  itk::FixedArray<unsigned int, 2> layout;
  unsigned int noOfTimeSteps = morphImages.size();
  layout[0] = noOfTimeSteps;
  layout[1] = 0; // automatic number of neccessary rows
  tileFilter->SetLayout(layout);

  // Get Reference image (all images are expected to have exact same dimensions!)

  std::string fileName = morphFiles.at(0);
  mitk::Image *referenceImg = morphImages.at(0);

  mitk::Image::Pointer merged4D;
  std::string volumeFile;

  if (parsedArgs.count("4dVolume") || parsedArgs.count("v"))
  {
    volumeFile = us::any_cast<string>(parsedArgs["4dVolume"]);
    if (volumeFile != "")
    {
      unsigned int *dims = new unsigned int[4];
      dims[0] = referenceImg->GetDimensions()[0];
      dims[1] = referenceImg->GetDimensions()[1];
      dims[2] = referenceImg->GetDimensions()[2];
      dims[3] = morphImages.size();
      merged4D = mitk::Image::New();
      merged4D->Initialize(referenceImg->GetPixelType(), 4, dims);
      merged4D->GetTimeGeometry()->Expand(noOfTimeSteps);
    }
  }

  unsigned int *dim = referenceImg->GetDimensions();
  unsigned int sliceMaxAxial = dim[2];

  // Now iterate over all data sets, extract overlay and add it to reference image
  mitk::Image *morphImage;
  mitk::Image *segmentationImage = nullptr;

  for (unsigned int i = 0; i < noOfTimeSteps; i++)
  {
    MITK_INFO << "File : " << i << " of /" << noOfTimeSteps;
    int currentSliceNo = 0;

    // Retrieve images of current time step
    fileName = morphFiles.at(i);
    morphImage = morphImages.at(i);

    // Create 4D Volume image
    if (volumeFile != "")
    {
      mitk::ImagePixelReadAccessor<InputPixelType, 3> readAc(morphImage);

      merged4D->GetGeometry(i)->SetSpacing(referenceImg->GetGeometry()->GetSpacing());
      merged4D->GetGeometry(i)->SetOrigin(referenceImg->GetGeometry()->GetOrigin());
      merged4D->GetGeometry(i)->SetIndexToWorldTransform(referenceImg->GetGeometry()->GetIndexToWorldTransform());
      merged4D->SetVolume(readAc.GetData(), i);
    }

    MITK_INFO << "-- Convert to RGB";
    rgbImage = progressVis.ConvertToRGBImage(morphImage);

    // Add current seg in red
    color.Fill(0);
    color[0] = 200 * opacity;

    if (!blank)
    {
      segmentationImage = segImages.at(i);
      if (segmentationImage != nullptr)
      {
        MITK_INFO << "-- Add Overlay";
        progressVis.AddColouredOverlay(rgbImage, segmentationImage, color);
      }
    }
    // Add Segmentation of next time step in red
    if (i == 0)
    {
      MITK_INFO << "Skipping retro view in first time step";
    }
    else
    {
      color.Fill(0);
      // Add previous in green
      color[1] = 200 * opacity;
      if (!blank)
      {
        mitk::Image *nextSeg = segImages.at(i - 1);
        MITK_INFO << "-- Add Overlay of next Step";
        progressVis.AddColouredOverlay(rgbImage, nextSeg, color);
      }
    }

    // Now save all slices from overlay in output folder
    MITK_INFO << "-- Extract Slices";
    for (int slice = sliceMaxAxial - skip - 1; slice > skip; slice -= interval) // sliceMaxAxial/40.0f))
    {
      InputImageType::Pointer itkImage = InputImageType::New();
      InputImageType::Pointer itkImage2;

      mitk::CastToItkImage(rgbImage, itkImage);
      typedef itk::ImageDuplicator<InputImageType> DuplicatorType;
      DuplicatorType::Pointer duplicator = DuplicatorType::New();
      duplicator->SetInputImage(itkImage);
      duplicator->Update();
      itkImage2 = duplicator->GetOutput();

      itk::Image<RGBPixelType, 2>::Pointer extractedSlice = itk::Image<RGBPixelType, 2>::New();
      extractedSlice->Graft(progressVis.ExtractSlice(itkImage2, slice));
      tileFilter->SetInput(((currentSliceNo + 1) * (noOfTimeSteps) + i), extractedSlice);

      tileFilter->SetInput(i, progressVis.TextAsImage(GetDate(fileName)));

      currentSliceNo++;
    }
  }

  MITK_INFO << "Tile Filter Update";
  tileFilter->Update();

  // Write the output image
  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(tileFilter->GetOutput());
  std::string patientName;

  patientName = GetName(fileName);

  if (blank)
    writer->SetFileName(outputFile);
  else
    writer->SetFileName(outputFile);

  writer->Update();
  if (volumeFile != "")
    mitk::IOUtil::Save(merged4D, volumeFile);

  return EXIT_SUCCESS;
}
