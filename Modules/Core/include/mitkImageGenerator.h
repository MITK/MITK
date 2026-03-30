/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageGenerator_h
#define mitkImageGenerator_h

#include <mitkImageWriteAccessor.h>
#include <MitkCoreExports.h>
#include <itkImageRegionIterator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>
#include <mitkImage.h>

namespace mitk
{
  /**
   * \brief Generator for synthetic MITK images.
   *
   * This is a helper class to generate synthetic MITK images (random or gradient).
   *
   * \ingroup IO
   * \sa Image
   */
  class MITKCORE_EXPORT ImageGenerator
  {
  public:
    /**
     * \brief Generates a 3D gradient image with the defined size and spacing.
     *
     * Pixel values are filled sequentially starting from 0 and incrementing by 1
     * for each voxel.
     *
     * \tparam TPixelType The pixel type of the generated image.
     * \param dimX Image dimension in X direction (number of columns).
     * \param dimY Image dimension in Y direction (number of rows).
     * \param dimZ Image dimension in Z direction (number of slices).
     * \param spacingX Voxel spacing in X direction. Defaults to 1.
     * \param spacingY Voxel spacing in Y direction. Defaults to 1.
     * \param spacingZ Voxel spacing in Z direction. Defaults to 1.
     * \return A newly created MITK image filled with gradient values.
     */
    template <typename TPixelType>
    static mitk::Image::Pointer GenerateGradientImage(unsigned int dimX,
                                                      unsigned int dimY,
                                                      unsigned int dimZ,
                                                      float spacingX = 1,
                                                      float spacingY = 1,
                                                      float spacingZ = 1)
    {
      typedef itk::Image<TPixelType, 3> ImageType;
      typename ImageType::RegionType imageRegion;
      imageRegion.SetSize(0, dimX);
      imageRegion.SetSize(1, dimY);
      imageRegion.SetSize(2, dimZ);
      typename ImageType::SpacingType spacing;
      spacing[0] = spacingX;
      spacing[1] = spacingY;
      spacing[2] = spacingZ;

      mitk::Point3D origin;
      origin.Fill(0.0);
      itk::Matrix<double, 3, 3> directionMatrix;
      directionMatrix.SetIdentity();

      typename ImageType::Pointer image = ImageType::New();
      image->SetSpacing(spacing);
      image->SetOrigin(origin);
      image->SetDirection(directionMatrix);
      image->SetLargestPossibleRegion(imageRegion);
      image->SetBufferedRegion(imageRegion);
      image->SetRequestedRegion(imageRegion);
      image->Allocate();
      image->FillBuffer(0.0);

      typedef itk::ImageRegionIterator<ImageType> IteratorOutputType;
      IteratorOutputType it(image, imageRegion);
      it.GoToBegin();

      TPixelType val = 0;
      while (!it.IsAtEnd())
      {
        it.Set(val);
        val++;
        ++it;
      }

      mitk::Image::Pointer mitkImage = mitk::Image::New();
      mitkImage->InitializeByItk(image.GetPointer());
      mitkImage->SetVolume(image->GetBufferPointer());
      return mitkImage;
    }

    /**
     * \brief Generates an image with the same geometry as the given reference image.
     *
     * The image buffer is filled uniformly with the specified fill value.
     *
     * \tparam TPixelType The pixel type of the generated image.
     * \param reference The reference image whose geometry is used for the new image.
     * \param fill_value The value to fill every voxel with.
     * \return A newly created MITK image with the same geometry as the reference.
     */
    template <typename TPixelType>
    static mitk::Image::Pointer GenerateImageFromReference(mitk::Image::Pointer reference, TPixelType fill_value)
    {
      mitk::Image::Pointer output = mitk::Image::New();
      mitk::PixelType output_type = MakeScalarPixelType<TPixelType>();

      // all metadata (except type) come from reference image
      output->SetGeometry(reference->GetGeometry());
      output->Initialize(output_type, reference->GetDimension(), reference->GetDimensions());

      // get a pointer to the image buffer to write into
      TPixelType *imageBuffer = nullptr;
      try
      {
        mitk::ImageWriteAccessor writeAccess(output);
        imageBuffer = static_cast<TPixelType *>(writeAccess.GetData());
      }
      catch (...)
      {
        MITK_ERROR << "Write access not granted on mitk::Image.";
      }

      // fill the buffer with the specified value
      for (unsigned int i = 0; i < output->GetVolumeData(0)->GetSize(); i++)
      {
        imageBuffer[i] = fill_value;
      }

      return output;
    }

    /**
     * \brief Generates a random image with the defined size and spacing.
     *
     * Creates a 2D, 3D, or 4D image depending on the dimension parameters.
     * Supported pixel types are int, float, double, unsigned char, and unsigned short.
     *
     * \tparam TPixelType The pixel type of the generated image.
     * \param dimX Image dimension in X direction (number of columns).
     * \param dimY Image dimension in Y direction (number of rows).
     * \param dimZ Image dimension in Z direction. If <= 1, a 2D image is created. Defaults to 1.
     * \param dimT Image dimension in time. If <= 1, no time dimension is added. Defaults to 1.
     * \param spacingX Voxel spacing in X direction. Defaults to 1.
     * \param spacingY Voxel spacing in Y direction. Defaults to 1.
     * \param spacingZ Voxel spacing in Z direction. Defaults to 1.
     * \param randomMax The upper bound for random values. Defaults to 1000.
     * \param randMin The lower bound for random values (used for float/double types). Defaults to 0.
     * \return A newly created MITK image filled with random values.
     */
    template <typename TPixelType>
    static mitk::Image::Pointer GenerateRandomImage(unsigned int dimX,
                                                    unsigned int dimY,
                                                    unsigned int dimZ = 1,
                                                    unsigned int dimT = 1,
                                                    mitk::ScalarType spacingX = 1,
                                                    mitk::ScalarType spacingY = 1,
                                                    mitk::ScalarType spacingZ = 1,
                                                    const double randomMax = 1000.0f,
                                                    const double randMin = 0.0f)
    {
      // set the data type according to the template
      mitk::PixelType type = MakeScalarPixelType<TPixelType>();
      // type.Initialize(typeid(TPixelType));

      // initialize the MITK image with given dimension and data type
      mitk::Image::Pointer output = mitk::Image::New();
      auto dimensions = new unsigned int[4];
      unsigned int numberOfDimensions = 0;
      unsigned int bufferSize = 0;

      // check which dimension is needed
      if (dimT <= 1)
      {
        if (dimZ <= 1)
        { // 2D
          numberOfDimensions = 2;
          dimensions[0] = dimX;
          dimensions[1] = dimY;
          bufferSize = dimX * dimY;
        }
        else
        { // 3D
          numberOfDimensions = 3;
          dimensions[0] = dimX;
          dimensions[1] = dimY;
          dimensions[2] = dimZ;
          bufferSize = dimX * dimY * dimZ;
        }
      }
      else
      { // 4D
        numberOfDimensions = 4;
        dimensions[0] = dimX;
        dimensions[1] = dimY;
        dimensions[2] = dimZ;
        dimensions[3] = dimT;
        bufferSize = dimX * dimY * dimZ * dimT;
      }
      output->Initialize(type, numberOfDimensions, dimensions);
      mitk::Vector3D spacing;
      spacing[0] = spacingX;
      spacing[1] = spacingY;
      spacing[2] = spacingZ;
      output->SetSpacing(spacing);

      // get a pointer to the image buffer to write into
      TPixelType *imageBuffer = nullptr;
      try
      {
        mitk::ImageWriteAccessor writeAccess(output);
        imageBuffer = static_cast<TPixelType *>(writeAccess.GetData());
      }
      catch (...)
      {
        MITK_ERROR << "Write access not granted on mitk::Image.";
      }

      // initialize the random generator
      itk::Statistics::MersenneTwisterRandomVariateGenerator::Pointer randomGenerator =
        itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
      randomGenerator->Initialize();

      // fill the buffer for each pixel/voxel
      for (unsigned int i = 0; i < bufferSize; i++)
      {
        // the comparison of the component type is sufficient enough since the mitk::PixelType type object is
        // created as SCALAR and hence does not need the comparison against type.GetPixelTypeId() ==
        // itk::IOPixelEnum::SCALAR
        if (type.GetComponentType() == itk::IOComponentEnum::INT) // call integer function
        {
          imageBuffer[i] = (TPixelType)randomGenerator->GetIntegerVariate((int)randomMax);
          // TODO random generator does not support integer values in a given range (e.g. from 5-10)
          // range is always [0, (int)randomMax]
        }
        else if ((type.GetComponentType() == itk::IOComponentEnum::DOUBLE) ||
                 (type.GetComponentType() == itk::IOComponentEnum::FLOAT)) // call floating point function
        {
          imageBuffer[i] = (TPixelType)randomGenerator->GetUniformVariate(randMin, randomMax);
        }
        else if (type.GetComponentType() == itk::IOComponentEnum::UCHAR)
        {
          // use the integer randomGenerator with mod 256 to generate unsigned char values
          imageBuffer[i] = (unsigned char)((int)randomGenerator->GetIntegerVariate((int)randomMax)) % 256;
        }
        else if (type.GetComponentType() == itk::IOComponentEnum::USHORT)
        {
          imageBuffer[i] = (unsigned short)((int)randomGenerator->GetIntegerVariate((int)randomMax)) % 65536;
        }
        else
        {
          MITK_ERROR << "Datatype not supported yet.";
          // TODO call different methods for other datatypes
        }
      }
      return output;
    }
  };
} // namespace mitk

#endif
