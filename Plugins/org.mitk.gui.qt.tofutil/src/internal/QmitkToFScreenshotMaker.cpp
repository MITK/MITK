/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Qmitk
#include "QmitkToFScreenshotMaker.h"
#include <QmitkServiceListWidget.h>

// Mitk
#include <mitkToFCameraDevice.h>
#include <mitkIOUtil.h>

// Qt
#include <QString>
#include <QStringList>
#include <QMessageBox>

const std::string QmitkToFScreenshotMaker::VIEW_ID = "org.mitk.views.tofscreenshotmaker";

QmitkToFScreenshotMaker::QmitkToFScreenshotMaker()
  : QmitkAbstractView(),
    m_SavingCounter(0)
{
}

QmitkToFScreenshotMaker::~QmitkToFScreenshotMaker()
{
}

void QmitkToFScreenshotMaker::SetFocus()
{
}

void QmitkToFScreenshotMaker::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( (QObject*)(m_Controls.m_MakeScreenshot), SIGNAL(clicked()), this, SLOT(OnMakeScreenshotClicked()) );

  connect( m_Controls.m_ConnectedDeviceServiceListWidget, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnSelectCamera()));

  std::string filter = "";
  m_Controls.m_ConnectedDeviceServiceListWidget->Initialize<mitk::ToFImageSource>("ToFImageSourceName", filter);

  std::string defaultPath = "/tmp/";
#ifdef _WIN32
  defaultPath = "C:/tmp/";
#endif
  m_Controls.m_PathToSaveFiles->setText(defaultPath.c_str());
}

void QmitkToFScreenshotMaker::OnSelectCamera()
{
  //Update gui according to device properties
  mitk::ToFImageGrabber* source = static_cast<mitk::ToFImageGrabber*>(m_Controls.m_ConnectedDeviceServiceListWidget->GetSelectedService<mitk::ToFImageSource>());
  mitk::ToFCameraDevice* device = source->GetCameraDevice();
  m_Controls.m_MakeScreenshot->setEnabled(device->IsCameraActive());

  //todo: where do i get correct file extensions?
  std::vector<std::string> fileExtensions;
  fileExtensions.push_back(".png");
  fileExtensions.push_back(".nrrd");
  fileExtensions.push_back(".jpg");
  QStringList extensions;
  for( unsigned int i = 0; i < fileExtensions.size(); ++i)
  {
    extensions.append(QString(fileExtensions.at(i).c_str()));
  }
  this->UpdateGUIElements(device, "no depth property available", m_Controls.m_SaveDepth, m_Controls.m_DepthFormat, extensions, ".nrrd");
  //usually you want to save depth data, but there is no "HasDepthImage" property, because every depth
  //camera should provide a depth iamge
  m_Controls.m_SaveDepth->setChecked(true);
  m_Controls.m_SaveDepth->setEnabled(true);
  m_Controls.m_DepthFormat->setEnabled(true);

  this->UpdateGUIElements(device, "HasAmplitudeImage", m_Controls.m_SaveAmplitude ,
                          m_Controls.m_AmplitudeFormat, extensions, ".nrrd");
  this->UpdateGUIElements(device, "HasIntensityImage", m_Controls.m_SaveIntensity,
                          m_Controls.m_IntensityFormat, extensions, ".nrrd");
  this->UpdateGUIElements(device, "HasRGBImage", m_Controls.m_SaveColor,
                          m_Controls.m_ColorFormat, extensions, ".png"); //png is nice default for calibration
  this->UpdateGUIElements(device, "HasRawImage", m_Controls.m_SaveRaw,
                          m_Controls.m_RawFormat, extensions, ".nrrd");
}

void QmitkToFScreenshotMaker::UpdateGUIElements(mitk::ToFCameraDevice* device, const char* ToFImageType,
                                                QCheckBox* saveCheckBox, QComboBox* saveTypeComboBox,
                                                QStringList fileExentions, const char* preferredFormat)
{
  bool isTypeProvidedByDevice = false;
  device->GetBoolProperty(ToFImageType, isTypeProvidedByDevice);
  saveCheckBox->setChecked(isTypeProvidedByDevice);
  saveCheckBox->setEnabled(isTypeProvidedByDevice);

  saveTypeComboBox->clear();
  saveTypeComboBox->setEnabled(isTypeProvidedByDevice);
  saveTypeComboBox->addItems(fileExentions);
  int index = saveTypeComboBox->findText(preferredFormat);
  if ( index != -1 ) { // -1 for not found
     saveTypeComboBox->setCurrentIndex(index);
  }
}

void QmitkToFScreenshotMaker::OnMakeScreenshotClicked()
{
  mitk::ToFImageGrabber* source = static_cast<mitk::ToFImageGrabber*>(m_Controls.m_ConnectedDeviceServiceListWidget->GetSelectedService<mitk::ToFImageSource>());
  source->Update();

  //### Save Images
  this->SaveImage(source->GetOutput(0), m_Controls.m_SaveDepth->isChecked(),
                  m_Controls.m_PathToSaveFiles->text().toStdString(), std::string("Depth_"),
                  m_Controls.m_DepthFormat->currentText().toStdString());
  this->SaveImage(source->GetOutput(1), m_Controls.m_SaveAmplitude->isChecked(),
                  m_Controls.m_PathToSaveFiles->text().toStdString(), std::string("Amplitude_"),
                  m_Controls.m_AmplitudeFormat->currentText().toStdString());
  this->SaveImage(source->GetOutput(2), m_Controls.m_SaveIntensity->isChecked(),
                  m_Controls.m_PathToSaveFiles->text().toStdString(), std::string("Intensity_"),
                  m_Controls.m_IntensityFormat->currentText().toStdString());
  this->SaveImage(source->GetOutput(3), m_Controls.m_SaveColor->isChecked(),
                  m_Controls.m_PathToSaveFiles->text().toStdString(), std::string("Color_"),
                  m_Controls.m_ColorFormat->currentText().toStdString());
  //todo, where is the raw data?

  //todo what about the surface or pre-processed data?
  m_SavingCounter++;
}

void QmitkToFScreenshotMaker::SaveImage(mitk::Image::Pointer image, bool saveImage, std::string path, std::string name, std::string extension)
{
  if(saveImage)
  {
    std::stringstream outdepthimage;
    outdepthimage << path << name<< m_SavingCounter << extension;
    mitk::IOUtil::Save( image, outdepthimage.str() );
  }
}
