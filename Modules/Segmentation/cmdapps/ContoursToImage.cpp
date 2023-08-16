/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>
#include <mitkContourModelSet.h>
#include <mitkContourModelSetToImageFilter.h>
#include <mitkDataStorage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <filesystem>

enum class OutputFormat
{
  Binary,
  Label,
  Multilabel
};

void InitializeCommandLineParser(mitkCommandLineParser& parser)
{
  parser.setTitle("Contour to Image Converter");
  parser.setCategory("Segmentation");
  parser.setDescription("Converts contours (i. e. RTSTRUCT or MITK Contour Model Set) to binary image masks or (multi-)label segmentations.");
  parser.setContributor("German Cancer Research Center (DKFZ)");
  parser.setArgumentPrefix("--", "-");

  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input contour(s)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("reference", "r", mitkCommandLineParser::Image, "Reference image:", "Input reference image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::Image, "Output file:", "Output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("format", "f", mitkCommandLineParser::String, "Output format:", "Output format (binary, label, or multilabel)", std::string("binary"));
}

std::string GetSafeName(const mitk::IPropertyProvider* propertyProvider)
{
  std::string name;

  if (propertyProvider != nullptr)
  {
    name = propertyProvider->GetConstProperty("name")->GetValueAsString();

    if (!name.empty())
    {
      boost::trim(name);
      boost::replace_all(name, "/", "_");
      boost::replace_all(name, "\\", "_");

      // If you read this, feel free to handle invalid filename characters here. :)
    }
  }

  return name;
}

void CreateParentDirectories(const std::filesystem::path& path)
{
  if (path.has_parent_path())
  {
    auto parentPath = path.parent_path();

    if (!std::filesystem::exists(parentPath))
      std::filesystem::create_directories(parentPath);
  }
}

bool SetLabelName(const mitk::IPropertyProvider* propertyProvider, mitk::Label* label)
{
  if (propertyProvider != nullptr)
  {
    if (auto property = propertyProvider->GetConstProperty("name"); property.IsNotNull())
    {
      if (auto nameProperty = dynamic_cast<const mitk::StringProperty*>(property.GetPointer()); nameProperty != nullptr)
      {
        if (auto name = nameProperty->GetValueAsString(); !name.empty())
        {
          label->SetName(name);
          return true;
        }
      }
    }
  }

  return false;
}

bool SetLabelColor(const mitk::IPropertyProvider* propertyProvider, mitk::Label* label)
{
  if (propertyProvider != nullptr)
  {
    if (auto property = propertyProvider->GetConstProperty("color"); property.IsNotNull())
    {
      if (auto colorProperty = dynamic_cast<const mitk::ColorProperty*>(property.GetPointer()); colorProperty != nullptr)
      {
        label->SetColor(colorProperty->GetColor());
        return true;
      }
    }
  }

  return false;
}

void CopyImageToActiveLayerImage(const mitk::Image* image, mitk::LabelSetImage* labelSetImage)
{
  mitk::ImageReadAccessor readAccessor(image);
  mitk::ImageWriteAccessor writeAccessor(labelSetImage);

  auto size = sizeof(mitk::Label::PixelType);

  for (size_t dim = 0; dim < image->GetDimension(); ++dim)
    size *= image->GetDimension(dim);

  memcpy(writeAccessor.GetData(), readAccessor.GetData(), size);
}

OutputFormat ParseOutputFormat(const mitk::IFileIO::Options& args)
{
  auto it = args.find("format");

  if (it != args.end())
  {
    auto format = us::any_cast<std::string>(it->second);

    if (format == "multilabel")
      return OutputFormat::Multilabel;

    if (format == "label")
      return OutputFormat::Label;

    if (format != "binary")
      mitkThrow() << "Unknown output format \"" << format << "\" (must be \"binary\", \"label\" or \"multilabel\").";
  }

  return OutputFormat::Binary;
}

std::vector<mitk::ContourModelSet::Pointer> FilterValidInputs(const std::vector<mitk::BaseData::Pointer>& inputs)
{
  std::vector<mitk::ContourModelSet::Pointer> validInputs;

  for (auto input : inputs)
  {
    if (input.IsNull())
    {
      MITK_WARN << "Skipping null input.";
      continue;
    }

    auto* validInput = dynamic_cast<mitk::ContourModelSet*>(input.GetPointer());

    if (validInput == nullptr)
    {
      MITK_WARN << "Skipping input of type \"" << input->GetNameOfClass() << "\".";
      continue;
    }

    validInputs.push_back(validInput);
  }

  return validInputs;
}

int main(int argc, char* argv[])
{
  int returnValue = EXIT_SUCCESS;

  mitkCommandLineParser parser;
  InitializeCommandLineParser(parser);

  auto args = parser.parseArguments(argc, argv);

  if (args.empty())
    return EXIT_FAILURE;

  try
  {
    auto inputFilename = us::any_cast<std::string>(args["input"]);
    auto referenceFilename = us::any_cast<std::string>(args["reference"]);
    auto outputFilename = us::any_cast<std::string>(args["output"]);
    auto format = ParseOutputFormat(args);

    auto referenceImage = mitk::IOUtil::Load<mitk::Image>(referenceFilename);
    auto inputs = FilterValidInputs(mitk::IOUtil::Load(inputFilename));

    MITK_INFO << "Found " << inputs.size() << " input contour set(s)";

    std::filesystem::path outputPath(outputFilename);
    CreateParentDirectories(outputPath);

    mitk::LabelSetImage::Pointer labelSetImage; // Only used for "multilabel" output
    unsigned int nonameCounter = 0; // Helper variable to generate placeholder names for nameless contour sets

    for (auto input : inputs)
    {
      // If the input file contains multiple contour sets but the output format is not set to "multilabel",
      // we create separate output files for each contour set. In this case the specified output filename
      // is used only as a base filename and the names of the individual contour sets are appended accordingly.

      if (inputs.size() > 1 && format != OutputFormat::Multilabel)
      {
        outputPath = outputFilename;
        auto name = GetSafeName(input);

        if (name.empty())
          name = "nameless_" + std::to_string(nonameCounter++);

        outputPath.replace_filename(outputPath.stem().string() + '_' + name + outputPath.extension().string());
      }

      // Do the actual conversion from a contour set to an image with a background pixel value of 0.
      // - For "binary" output, use pixel value 1 and unsigned char as pixel type.
      // - For "label" output, use pixel value 1 and our label pixel type.
      // - For "multilabel" output, use the next available label value instead.

      const auto labelValue = labelSetImage.IsNotNull()
        ? static_cast<mitk::Label::PixelType>(labelSetImage->GetTotalNumberOfLabels() + 1)
        : 1;

      auto filter = mitk::ContourModelSetToImageFilter::New();
      filter->SetMakeOutputLabelPixelType(format != OutputFormat::Binary);
      filter->SetPaintingPixelValue(labelValue);
      filter->SetImage(referenceImage);
      filter->SetInput(input);

      filter->Update();

      mitk::Image::Pointer image = filter->GetOutput();
      filter = nullptr;

      if (image.IsNull())
      {
        MITK_ERROR << "Contour set to image conversion failed without exception. Continue with next contour set... ";
        returnValue = EXIT_FAILURE;
        continue;
      }

      if (format == OutputFormat::Binary)
      {
        mitk::IOUtil::Save(image, outputPath.string());
      }
      else
      {
        if (labelSetImage.IsNull())
        {
          labelSetImage = mitk::LabelSetImage::New();
          labelSetImage->Initialize(image);

          CopyImageToActiveLayerImage(image, labelSetImage);
        }
        else
        {
          labelSetImage->AddLayer(image);
        }

        auto label = mitk::LabelSetImageHelper::CreateNewLabel(labelSetImage);
        label->SetValue(labelValue);

        SetLabelName(input, label);
        SetLabelColor(input, label);

        if (format == OutputFormat::Multilabel)
          MITK_INFO << "Creating label: " << label->GetName() << " [" << labelValue << ']';

        labelSetImage->GetActiveLabelSet()->AddLabel(label, false);

        if (format == OutputFormat::Label)
        {
          mitk::IOUtil::Save(labelSetImage, outputPath.string());
          labelSetImage = nullptr;
        }
      }
    }

    // In case of the "multilabel" output format, eventually save the single output file.
    // For all other output formats, the output file(s) have been saved already while iterating
    // over the inputs.

    if (labelSetImage.IsNotNull())
      mitk::IOUtil::Save(labelSetImage, outputPath.string());
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return EXIT_FAILURE;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    return EXIT_FAILURE;
  }

  return returnValue;
}
