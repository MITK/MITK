/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutocropLabelSetImageAction.h"

#include <mitkImagePixelReadAccessor.h>
#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>
#include <mitkRenderingManager.h>

namespace
{
  // Iterate over all layers, time steps, and dimensions of a MultiLabelSegmentation to
  // determine the overall minimum and maximum indices of labeled pixels.
  //
  // Returns false if the input image is empty, minIndex and maxIndex contain
  // valid indices otherwise.
  //
  // Throws an mitk::Exception if read access was denied.
  //
  bool DetermineMinimumAndMaximumIndicesOfNonBackgroundPixels(const mitk::MultiLabelSegmentation* labelSetImage, itk::Index<3>& minIndex, itk::Index<3>& maxIndex)
  {
    const auto background = mitk::MultiLabelSegmentation::UNLABELED_VALUE;
    const auto numLayers = labelSetImage->GetNumberOfGroups();
    const auto numTimeSteps = labelSetImage->GetTimeSteps();

    const itk::Index<3> dim = {
      labelSetImage->GetDimensions()[0],
      labelSetImage->GetDimensions()[1],
      labelSetImage->GetDimensions()[2]
    };

    maxIndex = { 0, 0, 0 };
    minIndex = dim;
    itk::Index<3> index;

    bool labelSetImageIsEmpty = true;

    for (std::remove_const_t<decltype(numLayers)> layer = 0; layer < numLayers; ++layer)
    {
      for (std::remove_const_t<decltype(numTimeSteps)> timeStep = 0; timeStep < numTimeSteps; ++timeStep)
      {
        const mitk::Image* image = mitk::SelectImageByTimeStep(labelSetImage->GetGroupImage(layer), timeStep);
        mitk::ImagePixelReadAccessor<mitk::MultiLabelSegmentation::PixelType, 3> pixelReader(image);
        bool imageIsEmpty = true;

        for (index[2] = 0; index[2] < dim[2]; ++index[2])
        {
          for (index[1] = 0; index[1] < dim[1]; ++index[1])
          {
            for (index[0] = 0; index[0] < dim[0]; ++index[0])
            {
              if (background != pixelReader.GetPixelByIndex(index))
              {
                imageIsEmpty = false;
                minIndex = {
                  std::min(minIndex[0], index[0]),
                  std::min(minIndex[1], index[1]),
                  std::min(minIndex[2], index[2])
                };
                break;
              }
            }
          }
        }

        if (imageIsEmpty)
          continue;

        maxIndex = {
          std::max(maxIndex[0], minIndex[0]),
          std::max(maxIndex[1], minIndex[1]),
          std::max(maxIndex[2], minIndex[2])
        };

        for (index[2] = dim[2] - 1; index[2] >= 0; --index[2])
        {
          for (index[1] = dim[1] - 1; index[1] >= 0; --index[1])
          {
            for (index[0] = dim[0] - 1; index[0] >= 0; --index[0])
            {
              if (background != pixelReader.GetPixelByIndex(index))
              {
                maxIndex = {
                  std::max(maxIndex[0], index[0]),
                  std::max(maxIndex[1], index[1]),
                  std::max(maxIndex[2], index[2])
                };
                break;
              }
            }
          }
        }

        if (!imageIsEmpty)
          labelSetImageIsEmpty = false;
      }
    }

    return !labelSetImageIsEmpty;
  }

  // Crop a MultiLabelSegmentation. Labels in the cropped MultiLabelSegmentation will still have
  // their original properties like names and colors.
  //
  // Returns a cropped MultiLabelSegmentation.
  //
  // Throws an mitk::Exception if read access was denied.
  //
  mitk::MultiLabelSegmentation::Pointer Crop(mitk::MultiLabelSegmentation::Pointer labelSetImage, const itk::Index<3>& minIndex, const itk::Index<3>& maxIndex)
  {
    const auto numLayers = labelSetImage->GetNumberOfGroups();
    const auto numTimeSteps = labelSetImage->GetTimeSteps();

    const itk::Index<3> croppedDim = {
      1 + maxIndex[0] - minIndex[0],
      1 + maxIndex[1] - minIndex[1],
      1 + maxIndex[2] - minIndex[2]
    };

    const auto numPixels = croppedDim[0] * croppedDim[1] * croppedDim[2];

    mitk::BaseGeometry::BoundsArrayType croppedBounds;
    croppedBounds[0] = 0;
    croppedBounds[1] = croppedDim[0];
    croppedBounds[2] = 0;
    croppedBounds[3] = croppedDim[1];
    croppedBounds[4] = 0;
    croppedBounds[5] = croppedDim[2];

    // Clone and adapt the original TimeGeometry to the cropped region

    auto croppedTimeGeometry = labelSetImage->GetTimeGeometry()->Clone();

    for (std::remove_const_t<decltype(numTimeSteps)> timeStep = 0; timeStep < numTimeSteps; ++timeStep)
    {
      auto geometry = croppedTimeGeometry->GetGeometryForTimeStep(timeStep);

      mitk::Point3D croppedOrigin;
      geometry->IndexToWorld(minIndex, croppedOrigin);
      geometry->SetOrigin(croppedOrigin);

      geometry->SetBounds(croppedBounds);
    }
    croppedTimeGeometry->UpdateBoundingBox();

    auto croppedLabelSetImage = mitk::MultiLabelSegmentation::New();
    croppedLabelSetImage->Initialize(croppedTimeGeometry);

    // Create cropped image volumes for all time steps in all layers

    for (std::remove_const_t<decltype(numLayers)> layer = 0; layer < numLayers; ++layer)
    {
      auto groupID = croppedLabelSetImage->AddGroup();

      for (std::remove_const_t<decltype(numTimeSteps)> timeStep = 0; timeStep < numTimeSteps; ++timeStep)
      {
        const mitk::Image* image = mitk::SelectImageByTimeStep(labelSetImage->GetGroupImage(layer), timeStep);

        mitk::ImagePixelReadAccessor<mitk::MultiLabelSegmentation::PixelType, 3> pixelReader(image);
        auto* croppedVolume = new mitk::MultiLabelSegmentation::PixelType[numPixels];
        itk::Index<3> croppedIndex;
        itk::Index<3> index;

        for (croppedIndex[2] = 0; croppedIndex[2] < croppedDim[2]; ++croppedIndex[2])
        {
          for (croppedIndex[1] = 0; croppedIndex[1] < croppedDim[1]; ++croppedIndex[1])
          {
            for (croppedIndex[0] = 0; croppedIndex[0] < croppedDim[0]; ++croppedIndex[0])
            {
              index[0] = croppedIndex[0] + minIndex[0];
              index[1] = croppedIndex[1] + minIndex[1];
              index[2] = croppedIndex[2] + minIndex[2];
              const auto& pixel = pixelReader.GetPixelByIndex(index);

              croppedVolume[croppedIndex[2] * croppedDim[1] * croppedDim[0] + croppedIndex[1] * croppedDim[0] + croppedIndex[0]] = pixel;
            }
          }
        }

        croppedLabelSetImage->GetGroupImage(groupID)->SetImportVolume(croppedVolume, timeStep, 0, mitk::Image::ReferenceMemory);
      }
      croppedLabelSetImage->ReplaceGroupLabels(layer, labelSetImage->GetConstLabelsByValue(labelSetImage->GetLabelValuesByGroup(layer)));
    }

    return croppedLabelSetImage;
  }
}

QmitkAutocropLabelSetImageAction::QmitkAutocropLabelSetImageAction()
{
}

QmitkAutocropLabelSetImageAction::~QmitkAutocropLabelSetImageAction()
{
}

void QmitkAutocropLabelSetImageAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  for (const auto& dataNode : selectedNodes)
  {
    mitk::MultiLabelSegmentation::Pointer labelSetImage = dynamic_cast<mitk::MultiLabelSegmentation*>(dataNode->GetData());

    if (labelSetImage.IsNull())
      continue;

    mitk::MultiLabelSegmentation::Pointer croppedLabelSetImage;
    itk::Index<3> minIndex;
    itk::Index<3> maxIndex;

    try
    {
      if (!DetermineMinimumAndMaximumIndicesOfNonBackgroundPixels(labelSetImage, minIndex, maxIndex))
      {
        MITK_WARN << "Autocrop was skipped: Image \"" << dataNode->GetName() << "\" is empty.";
        return;
      }

      croppedLabelSetImage = Crop(labelSetImage, minIndex, maxIndex);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "Autocrop was aborted: Image read access to \"" << dataNode->GetName() << "\" was denied. Details: "<< e.what();
      return;
    }

    // Override the original MultiLabelSegmentation with the cropped MultiLabelSegmentation
    dataNode->SetData(croppedLabelSetImage);

    // If we cropped a single MultiLabelSegmentation, reinit the views to give a visible feedback to the user
    if (1 == selectedNodes.size())
      mitk::RenderingManager::GetInstance()->InitializeViews(croppedLabelSetImage->GetTimeGeometry());
  }
}

void QmitkAutocropLabelSetImageAction::SetSmoothed(bool)
{
}

void QmitkAutocropLabelSetImageAction::SetDecimated(bool)
{
}

void QmitkAutocropLabelSetImageAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkAutocropLabelSetImageAction::SetFunctionality(berry::QtViewPart*)
{
}
