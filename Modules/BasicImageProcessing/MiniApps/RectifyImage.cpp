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

template <typename TPixelType>
void RectifyImage(mitk::Image::Pointer inputImage, mitk::Image::Pointer outputImage)
{
  mitk::ImagePixelReadAccessor<TPixelType, 3> pixelReadAccess(inputImage);
  mitk::ImagePixelWriteAccessor<TPixelType, 3> pixelWriteAccess(outputImage);

  const auto DEPTH = static_cast<itk::IndexValueType>(outputImage->GetDimension(2));
  const auto HEIGHT = static_cast<itk::IndexValueType>(outputImage->GetDimension(1));
  const auto WIDTH = static_cast<itk::IndexValueType>(outputImage->GetDimension(0));

  auto geometry = outputImage->GetGeometry();
  itk::Index<3> index;
  mitk::Point3D worldCoords;

  for (index[2] = 0; index[2] < DEPTH; ++index[2])
  {
    for (index[1] = 0; index[1] < HEIGHT; ++index[1])
    {
      for (index[0] = 0; index[0] < WIDTH; ++index[0])
      {
        geometry->IndexToWorld(index, worldCoords);
        pixelWriteAccess.SetPixelByIndex(index, pixelReadAccess.GetPixelByWorldCoordinates(worldCoords));
      }
    }
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Rectify Image");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("Resample image based on standard world to index transform");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::Image, "Input Image", "Path to input image", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::Image, "Output Image", "Path to output image", us::Any(), false);

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

  if (3 != inputImage->GetDimension())
  {
    MITK_ERROR << "Only 3-d images are supported.";
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

  mitk::Vector3D spacing = inputGeometry->GetSpacing();
  auto transform = inputGeometry->GetIndexToWorldTransform()->Clone();
  auto matrix = transform->GetMatrix();

  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      matrix[i][j] = std::abs(matrix[i][j]) / spacing[j];
    }
  }

  transform->SetMatrix(matrix);
  spacing = transform->TransformVector(spacing);

  mitk::Vector3D outputExtent = (maxOutputIndexInWorld - minOutputIndexInWorld + spacing);
  for (int i = 0; i < 3; ++i)
    outputExtent[i] /= spacing[i];

  mitk::Point3D origin = minOutputIndexInWorld;

  mitk::Vector3D right;
  mitk::FillVector3D(right, outputExtent[0], 0.0, 0.0);

  mitk::Vector3D down;
  mitk::FillVector3D(down, 0.0, outputExtent[1], 0.0);

  auto planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(right, down, &spacing);
  planeGeometry->SetOrigin(origin);
  planeGeometry->SetImageGeometry(true);

  auto slicedGeometry = mitk::SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, static_cast<unsigned int>(outputExtent[2]));

  auto outputGeometry = mitk::ProportionalTimeGeometry::New();
  outputGeometry->SetTimeStepGeometry(slicedGeometry, 0);

  auto pixelType = inputImage->GetPixelType();

  auto outputImage = mitk::Image::New();
  outputImage->Initialize(pixelType, *outputGeometry);

  try
  {
    switch (pixelType.GetComponentType())
    {
      case itk::ImageIOBase::CHAR:
        RectifyImage<char>(inputImage, outputImage);
        break;

      case itk::ImageIOBase::UCHAR:
        RectifyImage<unsigned char>(inputImage, outputImage);
        break;

      case itk::ImageIOBase::SHORT:
        RectifyImage<short>(inputImage, outputImage);
        break;

      case itk::ImageIOBase::USHORT:
        RectifyImage<unsigned short>(inputImage, outputImage);
        break;

      default:
        MITK_ERROR << "Pixel type is not supported.";
        return EXIT_FAILURE;
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
