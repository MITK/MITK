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

#include "QmitkImageMaskingWidget.h"
#include "mitkImage.h"
#include "../../Common/QmitkDataSelectionWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkMaskImageFilter.h>
#include <mitkProgressBar.h>
#include <mitkSliceNavigationController.h>
#include <mitkSurfaceToImageFilter.h>

#include <qmessagebox.h>

static QString const HelpText = QWidget::tr("Select a regular image and a binary image");
static QString const TR_SELECT_IGM_SRF = QWidget::tr("Select a regular image and a surface");
static QString const TR_SELECT_TWO_DIFF = QWidget::tr("Select two different images above");
static QString const TR_DIF_IMG_SIZE = QWidget::tr("Different image sizes cannot be masked");
static QString const TR_SELECT_REG_IMAGE_OR_BIN = QWidget::tr("Select a regular image and a binary image");

static QString const TR_IMAGE_SURF_MASKING = QMessageBox::tr("Image and Surface Masking");
static QString const TR_SELECTION_NOT_CONTAIN_IMG = QMessageBox::tr("Selection does not contain an image");
static QString const TR_SELECTION_NOT_CONTAIN_BIN_IMG = QMessageBox::tr("Selection does not contain a binary image");
static QString const TR_SELECTION_NOT_CONTAIN_SURFACE = QMessageBox::tr("Selection does not contain a surface");
static QString const TR_MASKING_FAILED = QMessageBox::tr("Masking failed. For more information please see logging window.");

QmitkImageMaskingWidget::QmitkImageMaskingWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.rbMaskImage, SIGNAL(toggled(bool)), this, SLOT(OnImageMaskingToggled(bool)));
  connect (m_Controls.rbMaskSurface, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceMaskingToggled(bool)));
  connect (m_Controls.btnMaskImage, SIGNAL(clicked()), this, SLOT(OnMaskImagePressed()));

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
      dataSelectionWidget->SetHelpText(TR_SELECT_IGM_SRF);
    }
    this->EnableButtons(false);
  }
  else
  {
    this->SelectionControl(index, selection);
  }
}

void QmitkImageMaskingWidget::SelectionControl(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(index);

  //if Image-Masking is enabled, check if image-dimension of reference and binary image is identical
  if( m_Controls.rbMaskImage->isChecked() )
  {
    if( dataSelectionWidget->GetSelection(0) == dataSelectionWidget->GetSelection(1) )
    {
      dataSelectionWidget->SetHelpText(TR_SELECT_TWO_DIFF);
      this->EnableButtons(false);
      return;
    }

    else if( node.IsNotNull() && selection )
    {
      mitk::Image::Pointer referenceImage = dynamic_cast<mitk::Image*> ( dataSelectionWidget->GetSelection(0)->GetData() );
      mitk::Image::Pointer maskImage = dynamic_cast<mitk::Image*> ( dataSelectionWidget->GetSelection(1)->GetData() );

      if( maskImage.IsNull() || referenceImage->GetLargestPossibleRegion().GetSize() != maskImage->GetLargestPossibleRegion().GetSize() )
      {
        dataSelectionWidget->SetHelpText(TR_DIF_IMG_SIZE);
        this->EnableButtons(false);
        return;
      }
    }

    else
    {
      dataSelectionWidget->SetHelpText(HelpText);
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
    m_Controls.dataSelectionWidget->SetHelpText(TR_SELECT_REG_IMAGE_OR_BIN);
    m_Controls.dataSelectionWidget->SetPredicate(1, QmitkDataSelectionWidget::SegmentationPredicate);
  }
}

void QmitkImageMaskingWidget::OnSurfaceMaskingToggled(bool status)
{
  if (status)
  {
    m_Controls.dataSelectionWidget->SetHelpText(TR_SELECT_IGM_SRF);
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
    QMessageBox::information( this, TR_IMAGE_SURF_MASKING, TR_SELECTION_NOT_CONTAIN_IMG, QMessageBox::Ok );
    m_Controls.btnMaskImage->setEnabled(true);
    return;
  }

  //Do Image-Masking
  if (m_Controls.rbMaskImage->isChecked())
  {
    mitk::ProgressBar::GetInstance()->Progress();

    mitk::Image::Pointer maskImage = dynamic_cast<mitk::Image*> ( maskingNode->GetData() );

    if(maskImage.IsNull() )
    {
      MITK_ERROR << "Selection does not contain a binary image";
      QMessageBox::information( this, TR_IMAGE_SURF_MASKING, TR_SELECTION_NOT_CONTAIN_BIN_IMG, QMessageBox::Ok );
      this->EnableButtons();
      return;
    }

    if( referenceImage->GetLargestPossibleRegion().GetSize() == maskImage->GetLargestPossibleRegion().GetSize() )
    {
      resultImage = this->MaskImage( referenceImage, maskImage );
    }

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
      QMessageBox::information( this, TR_IMAGE_SURF_MASKING, TR_SELECTION_NOT_CONTAIN_SURFACE, QMessageBox::Ok );
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
    QMessageBox::information( this, TR_IMAGE_SURF_MASKING, TR_MASKING_FAILED, QMessageBox::Ok );
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
  maskFilter->OverrideOutsideValueOn();
  maskFilter->SetOutsideValue( referenceImage->GetStatistics()->GetScalarValueMin() );
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
