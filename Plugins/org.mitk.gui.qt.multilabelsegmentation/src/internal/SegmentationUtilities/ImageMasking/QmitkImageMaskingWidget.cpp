/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageMaskingWidget.h"
#include "../../Common/QmitkDataSelectionWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkMaskImageFilter.h>
#include <mitkPadImageFilter.h>
#include <mitkProgressBar.h>
#include <mitkIDataStorageService.h>
#include <mitkSliceNavigationController.h>
#include <mitkSurfaceToImageFilter.h>

#include <berryPlatform.h>

#include <qmessagebox.h>

static const char* const HelpText = "Select a patient image and a mask";

QmitkImageMaskingWidget::QmitkImageMaskingWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::MaskPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

//  mitk::IDataStorageService::Pointer service =
//    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

//  assert(service.IsNotNull());

  mitk::DataStorage::Pointer ds = m_Controls.dataSelectionWidget->GetDataStorage();
  m_Controls.m_LabelSetWidget->SetDataStorage(ds);
  m_Controls.m_LabelSetWidget->setEnabled(true);

  m_Controls.m_MaskStampWidget->SetDataStorage(ds);

  this->EnableButtons(false);

  connect (m_Controls.rbMaskImage, SIGNAL(toggled(bool)), this, SLOT(OnImageMaskingToggled(bool)));
  connect (m_Controls.rbMaskSurface, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceMaskingToggled(bool)));
  connect (m_Controls.btnMaskImage, SIGNAL(pressed()), this, SLOT(OnMaskImagePressed()));

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkImageMaskingWidget::~QmitkImageMaskingWidget()
{
}

void QmitkImageMaskingWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node0 = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer node1 = dataSelectionWidget->GetSelection(1);

  if (node0.IsNull() || node1.IsNull() )
  {
    if( m_Controls.rbMaskImage->isChecked() )
    {
      dataSelectionWidget->SetHelpText(HelpText);
    }
    else
    {
      dataSelectionWidget->SetHelpText("Select a patient image and a surface");
    }
    this->EnableButtons(false);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkImageMaskingWidget::SelectionControl(unsigned int index, const mitk::DataNode*)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  //if Image-Masking is enabled, check if image-dimension of reference and binary image is identical
  if( m_Controls.rbMaskImage->isChecked() )
  {
    if( dataSelectionWidget->GetSelection(0) == dataSelectionWidget->GetSelection(1) )
    {
      dataSelectionWidget->SetHelpText("Select two different images above");
      this->EnableButtons(false);
      return;
    }
  }

  dataSelectionWidget->SetHelpText("");
  this->EnableButtons();
}

void QmitkImageMaskingWidget::EnableButtons(bool enable)
{
  m_Controls.btnMaskImage->setEnabled(enable);
}

void QmitkImageMaskingWidget::OnImageMaskingToggled(bool status)
{
  if (status)
  {
    m_Controls.dataSelectionWidget->SetHelpText("Select a patient image and a mask");
    m_Controls.dataSelectionWidget->SetPredicate(1, QmitkDataSelectionWidget::MaskPredicate);
  }
}

void QmitkImageMaskingWidget::OnSurfaceMaskingToggled(bool status)
{
  if (status)
  {
    m_Controls.dataSelectionWidget->SetHelpText("Select a patient image and a surface");
    m_Controls.dataSelectionWidget->SetPredicate(1, QmitkDataSelectionWidget::SurfacePredicate);
  }
}


void QmitkImageMaskingWidget::OnMaskImagePressed()
{
  //Disable Buttons during calculation and initialize Progressbar
  this->EnableButtons(false);
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  mitk::ProgressBar::GetInstance()->Progress();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;

  //create result image, get mask node and reference image
  mitk::Image::Pointer resultImage(nullptr);
  mitk::DataNode::Pointer maskingNode = dataSelectionWidget->GetSelection(1);
  mitk::Image::Pointer referenceImage = static_cast<mitk::Image*>(dataSelectionWidget->GetSelection(0)->GetData());

  if(referenceImage.IsNull() || maskingNode.IsNull() )
  {
    MITK_ERROR << "Selection does not contain an image";
    QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain an image", QMessageBox::Ok );
    m_Controls.btnMaskImage->setEnabled(true);
    return;
  }

  //Do Image-Masking
  if (m_Controls.rbMaskImage->isChecked())
  {
    mitk::ProgressBar::GetInstance()->Progress();

    mitk::Image::Pointer maskImage = dynamic_cast<mitk::Image*> ( maskingNode->GetData() );

    if( referenceImage->GetLargestPossibleRegion().GetSize() != maskImage->GetLargestPossibleRegion().GetSize() )
    {
      mitk::PadImageFilter::Pointer padImageFilter = mitk::PadImageFilter::New();
      padImageFilter->SetInput(0, maskImage);
      padImageFilter->SetInput(1, referenceImage);
      padImageFilter->SetPadConstant(0);
      padImageFilter->SetBinaryFilter(false);
      padImageFilter->SetLowerThreshold(0);
      padImageFilter->SetUpperThreshold(1);

      MITK_INFO << "Padding mask ...";

      padImageFilter->Update();

      maskImage = padImageFilter->GetOutput();
    }

    if(maskImage.IsNull() )
    {
      MITK_ERROR << "Selection does not contain a binary image";
      QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain a binary image", QMessageBox::Ok );
      this->EnableButtons();
      return;
    }

    resultImage = this->MaskImage( referenceImage, maskImage );
  }

  //Do Surface-Masking
  else
  {
    mitk::ProgressBar::GetInstance()->Progress();

    //1. convert surface to image
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*> ( maskingNode->GetData() );

    //TODO Get 3D Surface of current time step

    if(surface.IsNull())
    {
      MITK_ERROR << "Selection does not contain a surface";
      QMessageBox::information( this, "Image and Surface Masking", "Selection does not contain a surface", QMessageBox::Ok );
      this->EnableButtons();
      return;
    }

    mitk::Image::Pointer maskImage = this->ConvertSurfaceToImage( referenceImage, surface );

    //2. mask reference image with mask image
    if(maskImage.IsNotNull() &&
       referenceImage->GetLargestPossibleRegion().GetSize() == maskImage->GetLargestPossibleRegion().GetSize() )
    {
      resultImage = this->MaskImage( referenceImage, maskImage );
    }
  }

  mitk::ProgressBar::GetInstance()->Progress();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Masking failed";
    QMessageBox::information( this, "Image and Surface Masking", "Masking failed. For more information please see logging window.", QMessageBox::Ok );
    this->EnableButtons();
    mitk::ProgressBar::GetInstance()->Progress(4);
    return;
  }

  //Add result to data storage
  this->AddToDataStorage(
    dataSelectionWidget->GetDataStorage(),
    resultImage,
    dataSelectionWidget->GetSelection(0)->GetName() + "_" + dataSelectionWidget->GetSelection(1)->GetName(),
    dataSelectionWidget->GetSelection(0));

  this->EnableButtons();

  mitk::ProgressBar::GetInstance()->Progress();
}

mitk::Image::Pointer QmitkImageMaskingWidget::MaskImage(mitk::Image::Pointer referenceImage, mitk::Image::Pointer maskImage )
{
  mitk::Image::Pointer resultImage(nullptr);

  mitk::MaskImageFilter::Pointer maskFilter = mitk::MaskImageFilter::New();
  maskFilter->SetInput( referenceImage );
  maskFilter->SetMask( maskImage );
  //MLI TODO
  //if ( m_Controls.m_chkMakeOutputBinary->isChecked() )
  //{
  //  maskFilter->SetInsideValue(1.0);
  //  maskFilter->SetOutsideValue(0.0);
  //  maskFilter->SetOverrideOutsideValue(true);
  //  maskFilter->SetOverrideInsideValue(true);
  //}

  //maskFilter->SetOverrideOutsideValue( m_Controls.m_chkOverwriteBackground->isChecked() );
  //maskFilter->SetOverrideInsideValue( m_Controls.m_chkOverwriteForeground->isChecked() );
  //maskFilter->SetInsideValue( m_Controls.m_leForegroundValue->text().toFloat() );//referenceImage->GetStatistics()->GetScalarValueMin() );
  //maskFilter->SetOutsideValue( m_Controls.m_leBackgroundValue->text().toFloat() );//referenceImage->GetStatistics()->GetScalarValueMin() );

  try
  {
    maskFilter->Update();
  }
  catch(itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return nullptr;
  }

  resultImage = maskFilter->GetOutput();

  return resultImage;
}

mitk::Image::Pointer QmitkImageMaskingWidget::ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface )
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
  mitk::ProgressBar::GetInstance()->Progress();

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(image);
  try
  {
    surfaceToImageFilter->Update();
  }
  catch(itk::ExceptionObject& excpt)
  {
    MITK_ERROR << excpt.GetDescription();
    return nullptr;
  }

  mitk::ProgressBar::GetInstance()->Progress();
  mitk::Image::Pointer resultImage = mitk::Image::New();
  resultImage = surfaceToImageFilter->GetOutput();

  return resultImage;
}

void QmitkImageMaskingWidget::AddToDataStorage(mitk::DataStorage::Pointer dataStorage, mitk::Image::Pointer segmentation, const std::string& name, mitk::DataNode::Pointer parent )
{
  mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

  dataNode->SetName(name);
  dataNode->SetData(segmentation);

  dataStorage->Add(dataNode, parent);
}
