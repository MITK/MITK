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

#include "QmitkSurfaceToImageWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkSurfaceToImageFilter.h>
#include "mitkToolManagerProvider.h"

#include <qmessagebox.h>

static const char* const HelpText = "Select a regular image and a surface";

QmitkSurfaceToImageWidget::QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SurfacePredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  this->EnableButtons(false);

  connect (m_Controls.btnSurface2Image, SIGNAL(pressed()), this, SLOT(OnSurface2ImagePressed()));
  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if( m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls.dataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.dataSelectionWidget->GetSelection(0));
  }
}

QmitkSurfaceToImageWidget::~QmitkSurfaceToImageWidget()
{
}

void QmitkSurfaceToImageWidget::EnableButtons(bool enable)
{
  m_Controls.btnSurface2Image->setEnabled(enable);
}

void QmitkSurfaceToImageWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer imageNode = dataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer surfaceNode = dataSelectionWidget->GetSelection(1);

  if (imageNode.IsNull() || surfaceNode.IsNull() )
  {
    dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
  else
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( dataSelectionWidget->GetSelection(0)->GetData() );
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( dataSelectionWidget->GetSelection(1)->GetData() );
    if( image->GetTimeSteps() != surface->GetTimeSteps() )
    {
      dataSelectionWidget->SetHelpText("Image and surface are of different size");
      this->EnableButtons(false);
    }
    else
    {
      dataSelectionWidget->SetHelpText("");
      this->EnableButtons();
    }
  }
}

void QmitkSurfaceToImageWidget::OnSurface2ImagePressed()
{
  this->EnableButtons(false);

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode* segNode = dataSelectionWidget->GetSelection(0);
  mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>( segNode->GetData() );
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( dataSelectionWidget->GetSelection(1)->GetData() );

  if( image.IsNull() || surface.IsNull())
  {
    MITK_ERROR << "Selection does not contain a segmentation and/or a surface";
    QMessageBox::information( this, "Surface To Image", "Selection does not contain a segmentation and/or a surface", QMessageBox::Ok );
    this->EnableButtons();
    return;
  }

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOff();
  surfaceToImageFilter->SetInput(surface);
  surfaceToImageFilter->SetImage(image);
  try
  {
    surfaceToImageFilter->Update();
  }
  catch(itk::ExceptionObject& except)
  {
    MITK_ERROR << except.GetDescription();
    return;
  }

  mitk::ProgressBar::GetInstance()->Progress();

  mitk::Image::Pointer resultImage = surfaceToImageFilter->GetOutput();

  if( resultImage.IsNull() )
  {
    MITK_ERROR << "Surface stamp failed";
    QMessageBox::information( this, "MITK", "Surface stamp failed", QMessageBox::Ok );
    this->EnableButtons();
    return;
  }

  segNode->SetData(image);

  this->EnableButtons();
}
