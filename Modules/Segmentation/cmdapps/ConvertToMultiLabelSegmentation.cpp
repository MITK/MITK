/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// std includes
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>

// ITK includes
#include "itksys/SystemTools.hxx"

// MITK includes
#include "mitkCommandLineParser.h"
#include <mitkIOUtil.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkLabelSetImage.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateGeometry.h>
#include <mitkColorProperty.h>
#include <mitkException.h>
#include "ConvertToMultiLabelSegmentation.h"

// Global variables
std::vector<std::string> inputFilenames;
std::string outputFilename;
std::string referenceImageFilename;

enum class GroupingMode
{
  SingleGroup,    // All inputs into one group
  SeparateGroups  // Each input into separate group
};

GroupingMode groupingMode = GroupingMode::SingleGroup;

void setupParser(mitkCommandLineParser& parser)
{
  parser.setCategory("Segmentation Tools");
  parser.setTitle("Convert To Multi-Label Segmentation");
  parser.setDescription("Convert images, surfaces, or contours to multi-label segmentation format.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("Required I/O parameters");
  parser.addArgument("inputs", "i",
    mitkCommandLineParser::StringList,
    "Input files",
    "Paths to input files (images, surfaces, or contours) to convert",
    us::Any(), false, false, false, mitkCommandLineParser::Input);

  parser.addArgument("output", "o",
    mitkCommandLineParser::File,
    "Output segmentation",
    "Path for the output multi-label segmentation file",
    us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("Optional parameters");
  parser.addArgument("reference", "r",
    mitkCommandLineParser::File,
    "Reference image",
    "Reference image defining the geometry for conversion (only needed if no input images are provided)",
    us::Any(), true, false, false, mitkCommandLineParser::Input);

  parser.addArgument("groups", "g",
    mitkCommandLineParser::Bool,
    "Separate groups",
    "Create separate label groups for each input (default: merge all into single group)",
    us::Any(false), true);

  parser.addArgument("help", "h",
    mitkCommandLineParser::Bool,
    "Help",
    "Show this help text");
  parser.endGroup();
}

bool configureApplicationSettings(const std::map<std::string, us::Any>& parsedArgs)
{
  try
  {
    if (parsedArgs.empty())
      return false;

    inputFilenames = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs.at("inputs"));
    outputFilename = us::any_cast<std::string>(parsedArgs.at("output"));

    if (parsedArgs.count("reference"))
    {
      referenceImageFilename = us::any_cast<std::string>(parsedArgs.at("reference"));
    }

    if (parsedArgs.count("groups"))
    {
      auto separateGroups = us::any_cast<bool>(parsedArgs.at("groups"));
      groupingMode = separateGroups ? GroupingMode::SeparateGroups : GroupingMode::SingleGroup;
    }
  }
  catch (...)
  {
    return false;
  }

  return true;
}

struct InputData
{
  std::string filename;
  mitk::BaseData::Pointer data;
  mitk::Image::Pointer convertedImage;
  mitk::MultiLabelSegmentation::LabelValueVectorType foundLabels;

  bool isImage() const
  {
    return nullptr != dynamic_cast<const mitk::Image*>(this->data.GetPointer());
  };
};

bool ValidateGeometryCompatibility(const std::vector<InputData>& inputs, const mitk::Image* referenceImage)
{
  mitk::BaseGeometry::ConstPointer refGeometry = referenceImage->GetGeometry();

  for (const auto& input : inputs)
  {
    if (input.isImage())
    {
      auto image = dynamic_cast<const mitk::Image*>(input.data.GetPointer());
      if (image)
      {
        // Check if geometries are compatible (same or sub-geometry)
        if (!mitk::IsSubGeometry(*refGeometry, *(image->GetGeometry()),
            mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
            mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
        {
          if (!mitk::IsSubGeometry(*(image->GetGeometry()), *refGeometry,
              mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
              mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
          {
            MITK_ERROR << "Geometry mismatch for input: " << input.filename;
            return false;
          }
        }
      }
    }
  }
  return true;
}

mitk::Image::ConstPointer FindLargestImage(const std::vector<InputData>& inputs)
{
  mitk::BaseGeometry::ConstPointer largestGeometry;
  mitk::Image::ConstPointer largestImage;

  for (const auto& input : inputs)
  {
    if (input.isImage())
    {
      auto image = dynamic_cast<const mitk::Image*>(input.data.GetPointer());
      if (image)
      {
        if (largestGeometry.IsNull() ||
            mitk::IsSubGeometry(*largestGeometry, *(image->GetGeometry()),
                mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,
                mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION))
        {
          largestGeometry = image->GetGeometry();
          largestImage = image;
        }
      }
    }
  }
  return largestImage;
}

void CheckForLabelCollision(const std::vector<InputData>& inputs,
                           mitk::MultiLabelSegmentation::LabelValueVectorType& usedLabelValues,
                           std::map<const mitk::BaseData*, mitk::LabelValueMappingVector>& labelsMappingMap)
{
  for (auto& input : inputs)
  {
    const auto& foundLabels = input.foundLabels;
    mitk::MultiLabelSegmentation::LabelValueVectorType correctedLabelValues;
    mitk::CheckForLabelValueConflictsAndResolve(foundLabels, usedLabelValues, correctedLabelValues);

    mitk::LabelValueMappingVector mapping;
    std::transform(foundLabels.begin(), foundLabels.end(), correctedLabelValues.begin(),
                   std::back_inserter(mapping),
                   [](mitk::MultiLabelSegmentation::LabelValueType a,
                      mitk::MultiLabelSegmentation::LabelValueType b)
                   {
                     return std::make_pair(a, b);
                   });
    labelsMappingMap.emplace(input.data, mapping);
  }
}

bool IsUnsupportedDataType(InputData& inputData)
{
  return nullptr == dynamic_cast<mitk::Image*>(inputData.data.GetPointer()) &&
    nullptr == dynamic_cast<mitk::Surface*>(inputData.data.GetPointer()) &&
    nullptr == dynamic_cast<mitk::ContourModel*>(inputData.data.GetPointer()) &&
    nullptr == dynamic_cast<mitk::ContourModelSet*>(inputData.data.GetPointer());
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  setupParser(parser);

  const std::map<std::string, us::Any>& parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  if (!configureApplicationSettings(parsedArgs))
  {
    MITK_ERROR << "Invalid command line arguments. Use -h or --help for usage information.";
    return EXIT_FAILURE;
  }

  if (inputFilenames.empty())
  {
    MITK_ERROR << "No input files specified.";
    return EXIT_FAILURE;
  }

  try
  {
    std::cout << "Loading input files..." << std::endl;

    // Load all input data
    std::vector<InputData> inputs;
    mitk::PreferenceListReaderOptionsFunctor readerFilterFunctor =
      mitk::PreferenceListReaderOptionsFunctor({"MITK DICOM Reader v2 (autoselect)"}, {""});

    for (const auto& filename : inputFilenames)
    {
      std::cout << "Loading: " << filename << std::endl;

      InputData inputData;
      inputData.filename = filename;

      try
      {
        // Try to load as different data types
        auto loadedData = mitk::IOUtil::Load(filename, &readerFilterFunctor);
        if (loadedData.empty())
        {
          MITK_ERROR << "Failed to load: " << filename;
          return EXIT_FAILURE;
        }

        inputData.data = loadedData[0];

        // Check data type
        if (IsUnsupportedDataType(inputData))
        {
          MITK_ERROR << "Unsupported data type ("<< inputData.data->GetNameOfClass() << ")"
            "for file: " << filename;
          return EXIT_FAILURE;
        }

        inputs.push_back(inputData);
      }
      catch (const std::exception& e)
      {
        MITK_ERROR << "Error loading " << filename << ": " << e.what();
        return EXIT_FAILURE;
      }
    }

    // Determine reference geometry
    mitk::Image::ConstPointer referenceImage;

    if (!referenceImageFilename.empty())
    {
      std::cout << "Loading reference image: " << referenceImageFilename << std::endl;
      referenceImage = mitk::IOUtil::Load<mitk::Image>(referenceImageFilename, &readerFilterFunctor);
    }
    else
    {
      // First try to find the largest image from inputs
      referenceImage = FindLargestImage(inputs);
    }

    if (referenceImage.IsNull())
    {
      MITK_ERROR << "No reference geometry available. Either provide input images or specify a reference image.";
      return EXIT_FAILURE;
    }

    // Validate geometry compatibility
    if (!ValidateGeometryCompatibility(inputs, referenceImage))
    {
      MITK_ERROR << "Geometry validation failed. All images must have compatible geometries.";
      return EXIT_FAILURE;
    }

    std::cout << "Converting inputs to images..." << std::endl;

    // Convert all inputs to images and determine labels
    for (auto& input : inputs)
    {
      if (input.isImage())
      {
        auto image = dynamic_cast<mitk::Image*>(input.data.GetPointer());
        input.convertedImage = mitk::ConvertImageToGroupImage(image, input.foundLabels);
      }
      else
      {
        // Convert non-image data to binary images
        if (auto surface = dynamic_cast<mitk::Surface*>(input.data.GetPointer()); nullptr != surface)
        {
          input.convertedImage = mitk::ConvertSurfaceToLabelMask(referenceImage, surface);
        }
        else if (auto contourModelSet = dynamic_cast<mitk::ContourModelSet*>(input.data.GetPointer()); nullptr != contourModelSet)
        {
          input.convertedImage = mitk::ConvertContourModelSetToLabelMask(referenceImage, contourModelSet);
        }
        else if (auto contourModel = dynamic_cast<mitk::ContourModel*>(input.data.GetPointer()); nullptr != contourModel)
        {
          input.convertedImage = mitk::ConvertContourModelToLabelMask(referenceImage, contourModel);
        }

        if (input.convertedImage.IsNull())
        {
          MITK_ERROR << "Failed to convert input: " << input.filename;
          return EXIT_FAILURE;
        }

        // Non-image data creates binary maps with label value 1
        input.foundLabels = {1};
      }
    }

    std::cout << "Creating multi-label segmentation..." << std::endl;

    // Create output segmentation
    auto outputSegmentation = mitk::MultiLabelSegmentation::New();
    outputSegmentation->Initialize(referenceImage);

    // Check for label collisions and resolve them
    mitk::MultiLabelSegmentation::LabelValueVectorType usedLabelValues = outputSegmentation->GetAllLabelValues();
    std::map<const mitk::BaseData*, mitk::LabelValueMappingVector> labelsMappingMap;

    try
    {
      CheckForLabelCollision(inputs, usedLabelValues, labelsMappingMap);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Label collision resolution failed. Cannot convert input into segmentation due to unresolved "
        "label collisions. The inputs contain at least one equal label value that could not be resolved by remapping "
        "as not enough unused destination label values are available. One can often mitigate this problem by "
        "splitting the inputs into multiple segmentations conversions. Error detail:" << e.GetDescription();
      return EXIT_FAILURE;
    }

    // Process each input according to grouping mode
    mitk::MultiLabelSegmentation::GroupIndexType currentGroupIndex = 0;

    for (const auto& input : inputs)
    {
      // Create new group for separate groups mode (except for first input)
      if (groupingMode == GroupingMode::SeparateGroups)
      {
        if (input.data != inputs.front().data)
        {
          currentGroupIndex = outputSegmentation->AddGroup();
        }
        auto groupName = itksys::SystemTools::GetFilenameName(input.filename);
        outputSegmentation->SetGroupName(currentGroupIndex, groupName);
      }

      // Add labels for this input
      const auto& labelsMapping = labelsMappingMap.at(input.data);
      for (const auto& [oldValue, correctedValue] : labelsMapping)
      {
        std::string labelName;
        if (input.isImage())
        {
          if (groupingMode != GroupingMode::SeparateGroups)
          {
            std::string baseName = itksys::SystemTools::GetFilenameName(input.filename);
            labelName = baseName + " ";
          }
          labelName += "Value " + std::to_string(oldValue);
        }
        else
        {
          labelName = itksys::SystemTools::GetFilenameName(input.filename);
        }

        auto label = mitk::LabelSetImageHelper::CreateNewLabel(outputSegmentation, labelName, true);
        label->SetValue(correctedValue);

        outputSegmentation->AddLabel(label, currentGroupIndex, false, false);
      }

      // Transfer the converted image content
      mitk::TransferLabelContent(
        input.convertedImage,
        outputSegmentation->GetGroupImage(currentGroupIndex),
        outputSegmentation->GetConstLabelsByValue(outputSegmentation->GetLabelValuesByGroup(currentGroupIndex)),
        mitk::MultiLabelSegmentation::UNLABELED_VALUE,
        mitk::MultiLabelSegmentation::UNLABELED_VALUE,
        false,
        labelsMapping);
    }

    std::cout << "Saving output segmentation: " << outputFilename << std::endl;
    mitk::IOUtil::Save(outputSegmentation.GetPointer(), outputFilename);

    std::cout << "Conversion completed successfully." << std::endl;
    std::cout << "Created " << outputSegmentation->GetNumberOfGroups() << " group(s) with "
              << outputSegmentation->GetTotalNumberOfLabels() << " label(s) total." << std::endl;

    return EXIT_SUCCESS;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << "Error: " << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error occurred.";
    return EXIT_FAILURE;
  }
}
