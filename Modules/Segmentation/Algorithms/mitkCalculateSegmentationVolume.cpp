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

#include <mitkImageAccessByItk.h>

#include "itkImageRegionConstIteratorWithIndex.h"
#include "mitkCalculateSegmentationVolume.h"

#include <limits>

namespace mitk
{
  CalculateSegmentationVolume::CalculateSegmentationVolume() : m_Volume(0){}
  CalculateSegmentationVolume::~CalculateSegmentationVolume() {}
  template <typename TPixel, unsigned int VImageDimension>
  void CalculateSegmentationVolume::ItkImageProcessing(itk::Image<TPixel, VImageDimension> *itkImage,
                                                       TPixel *itkNotUsed(dummy))
  {
    itk::ImageRegionConstIteratorWithIndex<itk::Image<TPixel, VImageDimension>> iterBinaryImage(
      itkImage, itkImage->GetLargestPossibleRegion());
    typename itk::ImageRegionConstIteratorWithIndex<itk::Image<TPixel, VImageDimension>>::IndexType currentIndex;
    typename itk::ImageRegionConstIteratorWithIndex<itk::Image<TPixel, VImageDimension>>::IndexType minIndex;
    for (unsigned int i = 0; i < VImageDimension; ++i)
      minIndex[i] = std::numeric_limits<long int>::max();

    typename itk::ImageRegionConstIteratorWithIndex<itk::Image<TPixel, VImageDimension>>::IndexType maxIndex;
    for (unsigned int i = 0; i < VImageDimension; ++i)
      maxIndex[i] = std::numeric_limits<long int>::min();

    m_CenterOfMass.Fill(0.0);

    m_Volume = 0;
    while (!iterBinaryImage.IsAtEnd())
    {
      if (iterBinaryImage.Get() > static_cast<TPixel>(0.0))
      {
        // update center of mass
        currentIndex = iterBinaryImage.GetIndex();
        itk::Vector<ScalarType, VImageDimension> currentPoint;
        for (unsigned int i = 0; i < VImageDimension; ++i)
          currentPoint[i] = currentIndex[i];

        m_CenterOfMass =
          (m_CenterOfMass *
           (static_cast<ScalarType>(m_Volume) /
            static_cast<ScalarType>(m_Volume + 1))) // e.g. 3 points:   old center * 2/3 + currentPoint * 1/3;
          + currentPoint / static_cast<ScalarType>(m_Volume + 1);

        // update number of voxels
        ++m_Volume;

        // update bounding box
        for (unsigned int i = 0; i < VImageDimension; ++i)
        {
          if (currentIndex[i] < minIndex[i])
            minIndex[i] = currentIndex[i];
          if (currentIndex[i] > maxIndex[i])
            maxIndex[i] = currentIndex[i];
        }
      }

      ++iterBinaryImage;
    }

    m_MinIndexOfBoundingBox[2] = 0.0;
    m_MaxIndexOfBoundingBox[2] = 0.0;
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      m_MinIndexOfBoundingBox[i] = minIndex[i];
      m_MaxIndexOfBoundingBox[i] = maxIndex[i];
    }
  }

  bool CalculateSegmentationVolume::ReadyToRun()
  {
    Image::Pointer image;
    GetPointerParameter("Input", image);

    return image.IsNotNull() && GetGroupNode();
  }

  bool CalculateSegmentationVolume::ThreadedUpdateFunction()
  {
    // get image
    Image::Pointer image;
    GetPointerParameter("Input", image);

    AccessFixedDimensionByItk(image.GetPointer(),
                              ItkImageProcessing,
                              3); // some magic to call the correctly templated function (we only do 3D images here!)

    // consider single voxel volume
    Vector3D spacing = image->GetSlicedGeometry()->GetSpacing();                           // spacing in mm
    float volumeML = (ScalarType)m_Volume * spacing[0] * spacing[1] * spacing[2] / 1000.0; // convert to ml

    DataNode *groupNode = GetGroupNode();
    if (groupNode)
    {
      groupNode->SetProperty("volume", FloatProperty::New(volumeML));
      groupNode->SetProperty("centerOfMass", Vector3DProperty::New(m_CenterOfMass));
      groupNode->SetProperty("boundingBoxMinimum", Vector3DProperty::New(m_MinIndexOfBoundingBox));
      groupNode->SetProperty("boundingBoxMaximum", Vector3DProperty::New(m_MaxIndexOfBoundingBox));
      groupNode->SetProperty("showVolume", BoolProperty::New(true));
    }

    return true;
  }

} // namespace
