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

#include <memory>

QmitkAutocropLabelSetImageAction::QmitkAutocropLabelSetImageAction()
{
}

QmitkAutocropLabelSetImageAction::~QmitkAutocropLabelSetImageAction()
{
}

void QmitkAutocropLabelSetImageAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  using PixelType = mitk::LabelSetImage::PixelType;
  using PixelReaderType = mitk::ImagePixelReadAccessor<PixelType, 3>;

  for (const auto& dataNode : selectedNodes)
  {
    if (nullptr == dataNode->GetData())
      continue;

    auto* labelSetImage = dynamic_cast<mitk::LabelSetImage*>(dataNode->GetData());

    if (nullptr == labelSetImage)
      continue;

    // Backup currently active layer as we need to restore it later
    auto activeLayer = labelSetImage->GetActiveLayer();

    const auto background = labelSetImage->GetExteriorLabel()->GetValue();
    const auto numLayers = labelSetImage->GetNumberOfLayers();
    const auto numTimeSteps = labelSetImage->GetTimeSteps();

    // We need a time selector to handle 3d+t images. It is not used for 3d images, though.
    auto timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(labelSetImage);

    // Iterate over all layers, time steps, and dimensions to determine the overall minimum
    // and maximum indices of labeled pixels

    const itk::Index<3> dim = {
      labelSetImage->GetDimension(0),
      labelSetImage->GetDimension(1),
      labelSetImage->GetDimension(2)
    };

    itk::Index<3> maxIndex = { 0, 0, 0 };
    itk::Index<3> minIndex = dim;
    itk::Index<3> index;

    bool labelSetImageIsEmpty = true;

    for (std::remove_const_t<decltype(numLayers)> layer = 0; layer < numLayers; ++layer)
    {
      labelSetImage->SetActiveLayer(layer);

      for (std::remove_const_t<decltype(numTimeSteps)> timeStep = 0; timeStep < numTimeSteps; ++timeStep)
      {
        const mitk::Image* image = nullptr;

        if (numTimeSteps > 1)
        {
          timeSelector->SetTimeNr(timeStep);
          timeSelector->Update();
          image = timeSelector->GetOutput();
        }
        else
        {
          image = labelSetImage;
        }

        std::unique_ptr<PixelReaderType> pixelReader;

        try
        {
          pixelReader = std::make_unique<PixelReaderType>(image);
        }
        catch (const mitk::Exception&)
        {
          MITK_ERROR << "Autocrop was aborted: Image read access to \"" << dataNode->GetName() << "\" was denied.";
          return;
        }

        bool imageIsEmpty = true;

        for (index[2] = 0; index[2] < dim[2]; ++index[2])
        {
          for (index[1] = 0; index[1] < dim[1]; ++index[1])
          {
            for (index[0] = 0; index[0] < dim[0]; ++index[0])
            {
              if (background != pixelReader->GetPixelByIndex(index))
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
              if (background != pixelReader->GetPixelByIndex(index))
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

    if (labelSetImageIsEmpty)
    {
      MITK_WARN << "Autocrop was skipped: Image \"" << dataNode->GetName() << "\" is empty.";
      return;
    }

    // We are ready to construct a cropped LabelSetImage. Fasten your seat belts, please!

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

    auto croppedLabelSetImage = mitk::LabelSetImage::New();
    croppedLabelSetImage->Initialize(mitk::MakeScalarPixelType<PixelType>(), *croppedTimeGeometry);

    // Create cropped image volumes for all time steps in all layers

    for (std::remove_const_t<decltype(numLayers)> layer = 0; layer < numLayers; ++layer)
    {
      labelSetImage->SetActiveLayer(layer);
      croppedLabelSetImage->AddLayer();

      for (std::remove_const_t<decltype(numTimeSteps)> timeStep = 0; timeStep < numTimeSteps; ++timeStep)
      {
        const mitk::Image* image = nullptr;

        if (numTimeSteps > 1)
        {
          timeSelector->SetTimeNr(timeStep);
          timeSelector->Update();
          image = timeSelector->GetOutput();
        }
        else
        {
          image = labelSetImage;
        }

        std::unique_ptr<PixelReaderType> pixelReader;

        try
        {
          pixelReader = std::make_unique<PixelReaderType>(image);
        }
        catch (const mitk::Exception&)
        {
          MITK_ERROR << "Autocrop was aborted: Image read access to \"" << dataNode->GetName() << "\" was denied.";
          return;
        }

        auto* croppedVolume = new PixelType[numPixels];
        itk::Index<3> croppedIndex;

        for (croppedIndex[2] = 0; croppedIndex[2] < croppedDim[2]; ++croppedIndex[2])
        {
          for (croppedIndex[1] = 0; croppedIndex[1] < croppedDim[1]; ++croppedIndex[1])
          {
            // The inner loop may be replaced by a single memcpy, also using an ImageReadAccessor
            // instead of an ImagePixelReadAccessor. I wasn't able to correctly calculate the
            // source address, though. You like challenges? - Go ahead! :-)
            for (croppedIndex[0] = 0; croppedIndex[0] < croppedDim[0]; ++croppedIndex[0])
            {
              index[0] = croppedIndex[0] + minIndex[0];
              index[1] = croppedIndex[1] + minIndex[1];
              index[2] = croppedIndex[2] + minIndex[2];
              const auto& pixel = pixelReader->GetPixelByIndex(index);

              croppedVolume[croppedIndex[2] * croppedDim[1] * croppedDim[0] + croppedIndex[1] * croppedDim[0] + croppedIndex[0]] = pixel;
            }
          }
        }

        croppedLabelSetImage->SetVolume(croppedVolume, timeStep);

        // Make sure the labels still have their original properties like names and colors
        croppedLabelSetImage->AddLabelSetToLayer(layer, labelSetImage->GetLabelSet(layer));
      }
    }

    // Restore the originally active layer
    croppedLabelSetImage->SetActiveLayer(activeLayer);

    // Override the original LabelSetImage with the cropped LabelSetImage
    dataNode->SetData(croppedLabelSetImage);

    // If we cropped a single LabelSetImage, reinit the views to give a visible feedback to the user
    if (1 == selectedNodes.size())
      mitk::RenderingManager::GetInstance()->InitializeViews(croppedTimeGeometry, mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
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
