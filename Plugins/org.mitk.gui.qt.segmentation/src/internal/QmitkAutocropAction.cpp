/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkAutocropAction.h"

#include <mitkAutoCropImageFilter.h>
#include <mitkImageCast.h>
#include <mitkProgressTask.h>
#include <mitkRenderingManager.h>

#include <QmitkRun.h>

#include <itkConstantPadImageFilter.h>

QmitkAutocropAction::QmitkAutocropAction()
{
}

QmitkAutocropAction::~QmitkAutocropAction()
{
}

void QmitkAutocropAction::Run( const QList<mitk::DataNode::Pointer> &selectedNodes )
{
  foreach ( mitk::DataNode::Pointer node, selectedNodes )
  {
    if (node)
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNull()) return;

      mitk::Image::Pointer croppedImage;

      try
      {
        // Off the GUI thread, so that the notification appears and keeps moving
        // while a large image is cropped. A crop that runs here instead reaches
        // no event loop, and the card is then only shown once the work it was
        // announcing is over.
        QmitkRunWithInputBlocked([&]()
          {
            // Neither the crop nor the padding below reports anything of its
            // own, so there are no steps to count.
            mitk::ProgressTask task("Cropping image");

            auto cropFilter = mitk::AutoCropImageFilter::New();
            cropFilter->SetInput(image);
            cropFilter->SetBackgroundValue(0);
            cropFilter->Update();

            mitk::Image::Pointer croppedInput = cropFilter->GetOutput();

            if (croppedInput.IsNull())
              return;

            if (4 == croppedInput->GetDimension())
            {
              MITK_ERROR << "4D autocrop does not work at the moment";
              return;
            }

            croppedImage = this->IncreaseCroppedImageSize(croppedInput);
          });
      }
      catch (...)
      {
        MITK_ERROR << "Cropping image failed...";
      }

      if (croppedImage.IsNotNull())
      {
        // Back on the GUI thread, which is where what the renderers read is
        // allowed to change.
        node->SetData(croppedImage);
        mitk::RenderingManager::GetInstance()->InitializeViews(croppedImage->GetTimeGeometry());
      }
    }
    else
    {
      MITK_INFO << "   a nullptr node selected";
    }
  }
}

mitk::Image::Pointer QmitkAutocropAction::IncreaseCroppedImageSize( mitk::Image::Pointer image )
{
  typedef itk::Image< short, 3 > ImageType;
  typedef itk::Image< unsigned char, 3 > PADOutputImageType;

  ImageType::Pointer itkTransformImage = ImageType::New();
  mitk::CastToItkImage( image, itkTransformImage );

  typedef itk::ConstantPadImageFilter< ImageType, PADOutputImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();

  itk::SizeValueType upperPad[3];
  itk::SizeValueType lowerPad[3];
  int borderLiner = 3;

  mitk::Point3D mitkOriginPoint;
  double origin[3];
  origin[0]=0;
  origin[1]=0;
  origin[2]=0;
  itkTransformImage->SetOrigin(origin);

  lowerPad[0]=borderLiner;
  lowerPad[1]=borderLiner;
  lowerPad[2]=borderLiner;

  upperPad[0]=borderLiner;
  upperPad[1]=borderLiner;
  upperPad[2]=borderLiner;

  padFilter->SetInput(itkTransformImage);
  padFilter->SetConstant(0);
  padFilter->SetPadUpperBound(upperPad);
  padFilter->SetPadLowerBound(lowerPad);
  padFilter->UpdateLargestPossibleRegion();


  mitk::Image::Pointer paddedImage = mitk::Image::New();
  paddedImage->InitializeByItk(padFilter->GetOutput());
  mitk::CastToMitkImage(padFilter->GetOutput(), paddedImage);

  //calculate translation according to padding to get the new origin
  mitk::Point3D paddedOrigin = image->GetGeometry()->GetOrigin();
  mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  paddedOrigin[0] -= (borderLiner)*spacing[0];
  paddedOrigin[1] -= (borderLiner)*spacing[1];
  paddedOrigin[2] -= (borderLiner)*spacing[2];

  paddedImage->GetGeometry()->SetOrigin( paddedOrigin );

  return paddedImage;
}

void QmitkAutocropAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
  //not needed
}

void QmitkAutocropAction::SetFunctionality(berry::QtViewPart* /*view*/)
{
  //not needed
}
