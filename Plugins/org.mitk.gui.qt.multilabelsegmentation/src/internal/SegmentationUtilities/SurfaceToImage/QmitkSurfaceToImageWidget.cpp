/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSurfaceToImageWidget.h"

#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkIDataStorageService.h>
#include <mitkSurfaceStampImageFilter.h>

#include <berryPlatform.h>

#include <qmessagebox.h>

static const char* const HelpText = "Select a patient image and a surface";

QmitkSurfaceToImageWidget::QmitkSurfaceToImageWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.m_DataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::ImagePredicate);
  m_Controls.m_DataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::SurfacePredicate);
  m_Controls.m_DataSelectionWidget->SetHelpText(HelpText);

//  mitk::IDataStorageService* service =
//    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

//  assert(service.IsNotNull());

  mitk::DataStorage::Pointer ds = m_Controls.m_DataSelectionWidget->GetDataStorage();
  m_Controls.m_LabelSetWidget->SetDataStorage(ds);
  m_Controls.m_LabelSetWidget->setEnabled(true);

  m_Controls.m_SurfaceStampWidget->SetDataStorage(ds);

  this->EnableButtons(false);

  m_Controls.m_chkMakeOutputBinary->setChecked(true);

  connect (m_Controls.btnSurface2Image, SIGNAL(pressed()), this, SLOT(OnSurface2ImagePressed()));
  connect(m_Controls.m_DataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)),
    this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
  connect (m_Controls.m_chkMakeOutputBinary, SIGNAL(toggled(bool)), this, SLOT(OnMakeOutputBinaryChanged(bool)));

  if( m_Controls.m_DataSelectionWidget->GetSelection(0).IsNotNull() &&
    m_Controls.m_DataSelectionWidget->GetSelection(1).IsNotNull() )
  {
    this->OnSelectionChanged( 0, m_Controls.m_DataSelectionWidget->GetSelection(0));
  }
}

QmitkSurfaceToImageWidget::~QmitkSurfaceToImageWidget()
{
}

void QmitkSurfaceToImageWidget::EnableButtons(bool enable)
{
  m_Controls.btnSurface2Image->setEnabled(enable);
  m_Controls.m_chkMakeOutputBinary->setEnabled(enable);
  m_Controls.m_chkOverwriteBackground->setEnabled(enable && !m_Controls.m_chkMakeOutputBinary->isChecked());
  m_Controls.m_leForegroundValue->setEnabled(enable && !m_Controls.m_chkMakeOutputBinary->isChecked());
  m_Controls.m_leBackgroundValue->setEnabled(enable && !m_Controls.m_chkMakeOutputBinary->isChecked());
}

void QmitkSurfaceToImageWidget::OnSelectionChanged(unsigned int, const mitk::DataNode*)
{
  mitk::DataNode::Pointer imageNode = m_Controls.m_DataSelectionWidget->GetSelection(0);
  mitk::DataNode::Pointer surfaceNode = m_Controls.m_DataSelectionWidget->GetSelection(1);

  if (imageNode.IsNull() || surfaceNode.IsNull() )
  {
    m_Controls.m_DataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
  else
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( m_Controls.m_DataSelectionWidget->GetSelection(0)->GetData() );
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( m_Controls.m_DataSelectionWidget->GetSelection(1)->GetData() );
    if( image->GetTimeSteps() != surface->GetTimeSteps() )
    {
      m_Controls.m_DataSelectionWidget->SetHelpText("Image and surface are of different size");
      this->EnableButtons(false);
    }
    else
    {
      m_Controls.m_DataSelectionWidget->SetHelpText("");
      this->EnableButtons();
    }
  }
}

void QmitkSurfaceToImageWidget::OnSurface2ImagePressed()
{
  this->EnableButtons(false);

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( m_Controls.m_DataSelectionWidget->GetSelection(0)->GetData() );
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( m_Controls.m_DataSelectionWidget->GetSelection(1)->GetData() );

  if( image.IsNull() || surface.IsNull())
  {
    MITK_ERROR << "Selection does not contain an image and/or a surface";
    QMessageBox::information( this, "Surface To Image", "Selection does not contain an image and/or a surface", QMessageBox::Ok );
    this->EnableButtons();
    return;
  }

  mitk::Image::Pointer resultImage(nullptr);
  resultImage = this->ConvertSurfaceToImage( image, surface );

  if( resultImage.IsNull() )
  {
    QMessageBox::information( this, "Surface to Image", "Could not stamp surface.\n See error log for details.", QMessageBox::Ok );
    this->EnableButtons();
    return;
  }

  //create name for result node
  std::string nameOfResultImage = m_Controls.m_DataSelectionWidget->GetSelection(0)->GetName();
  nameOfResultImage.append("_");
  nameOfResultImage.append(m_Controls.m_DataSelectionWidget->GetSelection(1)->GetName());

  //create data node and add to data storage
  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  resultNode->SetData( resultImage );
  resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
  resultNode->SetProperty("binary", mitk::BoolProperty::New(m_Controls.m_chkMakeOutputBinary->isChecked()) );

  m_Controls.m_DataSelectionWidget->GetDataStorage()->Add(resultNode, m_Controls.m_DataSelectionWidget->GetSelection(0));

  this->EnableButtons();
}

mitk::Image::Pointer QmitkSurfaceToImageWidget::ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface )
{
  mitk::SurfaceStampImageFilter::Pointer filter = mitk::SurfaceStampImageFilter::New();
  filter->SetInput(image);
  filter->SetSurface(surface);
  filter->SetMakeOutputBinary(m_Controls.m_chkMakeOutputBinary->isChecked());
  filter->SetOverwriteBackground(m_Controls.m_chkOverwriteBackground->isChecked());
  filter->SetForegroundValue(m_Controls.m_leForegroundValue->text().toFloat());
  filter->SetBackgroundValue(m_Controls.m_leBackgroundValue->text().toFloat());

  try
  {
    filter->Update();
  }
  catch(mitk::Exception& e)
  {
    MITK_ERROR << "exception caught: " << e.GetDescription();
    return nullptr;
  }

  mitk::Image::Pointer resultImage = filter->GetOutput();

  return resultImage;
}

void QmitkSurfaceToImageWidget::OnMakeOutputBinaryChanged(bool value)
{
  m_Controls.m_chkOverwriteBackground->setEnabled(!value);
  m_Controls.m_leForegroundValue->setEnabled(!value);
  m_Controls.m_leBackgroundValue->setEnabled(!value);
}
