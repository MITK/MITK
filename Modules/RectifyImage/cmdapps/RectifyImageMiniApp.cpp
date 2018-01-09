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

#include <mitkCommandLineParser.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkIOUtil.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Rectify Image");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Resample image based on standard world to index transform");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to input image", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output Image", "Path to output image", us::Any(), false);

  auto parsedArgs = parser.parseArguments(argc, argv);

  if (2 != parsedArgs.size() || 0 == parsedArgs.count("input") || 0 == parsedArgs.count("output"))
    return EXIT_FAILURE;

  auto inputImagePath = us::any_value_to_string(parsedArgs["input"]);
  mitk::Image::Pointer inputImage;

  try
  {
    inputImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(inputImagePath)[0].GetPointer());
  }
  catch (const mitk::Exception&)
  {
    return EXIT_FAILURE;
  }

  auto inputGeometry = inputImage->GetGeometry();

  mitk::Point3D minInputIndex;
  mitk::FillVector3D(minInputIndex, 0.0, 0.0, 0.0);

  mitk::Point3D minInputIndexInWorld;
  inputGeometry->IndexToWorld(minInputIndex, minInputIndexInWorld);

  mitk::Point3D maxInputIndex;
  for (int i = 0; i < 3; ++i)
    maxInputIndex[i] = inputGeometry->GetExtent(i) - 1;

  mitk::Point3D maxInputIndexInWorld;
  inputGeometry->IndexToWorld(maxInputIndex, maxInputIndexInWorld);

  mitk::Point3D minOutputIndexInWorld;
  for (int i = 0; i < 3; ++i)
    minOutputIndexInWorld[i] = std::min(minInputIndexInWorld[i], maxInputIndexInWorld[i]);

  mitk::Point3D maxOutputIndexInWorld;
  for (int i = 0; i < 3; ++i)
    maxOutputIndexInWorld[i] = std::max(minInputIndexInWorld[i], maxInputIndexInWorld[i]);

  mitk::Vector3D outputExtent = maxOutputIndexInWorld - minOutputIndexInWorld + 1;

  mitk::Point3D origin = minOutputIndexInWorld;

  mitk::Vector3D right;
  mitk::FillVector3D(right, outputExtent[0], 0.0, 0.0);

  mitk::Vector3D down;
  mitk::FillVector3D(down, 0.0, outputExtent[1], 0.0);

  mitk::Vector3D spacing;
  mitk::FillVector3D(spacing, 1.0, 1.0, 1.0);

  auto planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(right, down, &spacing);
  planeGeometry->SetOrigin(origin);
  planeGeometry->SetImageGeometry(true);

  auto slicedGeometry = mitk::SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, static_cast<unsigned int>(outputExtent[2]));

  auto outputGeometry = mitk::ProportionalTimeGeometry::New();
  outputGeometry->SetTimeStepGeometry(slicedGeometry, 0);

  auto pixelType = inputImage->GetPixelType();
  const std::size_t PIXEL_TYPE_SIZE = pixelType.GetSize();

  auto outputImage = mitk::Image::New();
  outputImage->Initialize(pixelType, *outputGeometry);

  std::size_t outputImageBufferSize = PIXEL_TYPE_SIZE;
  for (int i = 0; i < 3; ++i)
    outputImageBufferSize *= outputImage->GetDimension(i);

  try
  {
    mitk::ImagePixelReadAccessor<unsigned short, 3> pixelReadAccess(inputImage);
    mitk::ImagePixelWriteAccessor<unsigned short, 3> pixelWriteAccess(outputImage);

    const auto OUTPUT_DEPTH = static_cast<itk::IndexValueType>(outputImage->GetDimension(2));
    const auto OUTPUT_HEIGHT = static_cast<itk::IndexValueType>(outputImage->GetDimension(1));
    const auto OUTPUT_WIDTH = static_cast<itk::IndexValueType>(outputImage->GetDimension(0));

    itk::Index<3> outputIndex;
    mitk::Point3D worldCoords;

    for (outputIndex[2] = 0; outputIndex[2] < OUTPUT_DEPTH; ++outputIndex[2])
    {
      for (outputIndex[1] = 0; outputIndex[1] < OUTPUT_HEIGHT; ++outputIndex[1])
      {
        for (outputIndex[0] = 0; outputIndex[0] < OUTPUT_WIDTH; ++outputIndex[0])
        {
          slicedGeometry->IndexToWorld(outputIndex, worldCoords);
          pixelWriteAccess.SetPixelByIndex(outputIndex, pixelReadAccess.GetPixelByWorldCoordinates(worldCoords));
        }
      }
    }
  }
  catch (const mitk::Exception &e)
  {
    MITK_ERROR << e.GetDescription();
    return EXIT_FAILURE;
  }

  auto outputImagePath = us::any_value_to_string(parsedArgs["output"]);

  try
  {
    mitk::IOUtil::Save(outputImage, outputImagePath);
  }
  catch (const mitk::Exception &)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
