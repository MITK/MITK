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

// Qmitk
#include "QmitkToFScreenshotMaker.h"

// Qt
#include <QString>
#include <QStringList>
#include <QMessageBox>

#include <mitkToFCameraDevice.h>
#include <QmitkServiceListWidget.h>

#include <mitkIOUtil.h>
#include <mitkImageWriter.h>

const std::string QmitkToFScreenshotMaker::VIEW_ID = "org.mitk.views.tofscreenshotmaker";

QmitkToFScreenshotMaker::QmitkToFScreenshotMaker()
  : QmitkAbstractView(),
    m_SavingCounter(0)
{
//  m_ToFImageGrabber = mitk::ToFImageGrabber::New();
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
//  std::string filter = "(&(IsActive=true))";

  m_Controls.m_ConnectedDeviceServiceListWidget->Initialize<mitk::ToFImageSource>("ToFImageSourceName", filter);

  std::string defaultPath = "/tmp/";
#ifdef _WIN32
  defaultPath = "C:\tmp\";
#endif
  m_Controls.m_PathToSaveFiles->setText(defaultPath.c_str());
}

void QmitkToFScreenshotMaker::OnSelectCamera()
{
  //Update gui accordingly
  mitk::ToFImageGrabber* source = static_cast<mitk::ToFImageGrabber*>(m_Controls.m_ConnectedDeviceServiceListWidget->GetSelectedService<mitk::ToFImageSource>());
  mitk::ToFCameraDevice* device = source->GetCameraDevice();
//  m_Controls.m_MakeScreenshot->setEnabled(device->IsCameraActive());

  //usually you want to save depth data
  m_Controls.m_SaveDepth->setChecked(true);
  m_Controls.m_SaveDepth->setEnabled(true);

  //If the device provides an amplitude (or infrared) image, we propably want to save it
  bool amplitude = false;
  device->GetBoolProperty("HasAmplitudeImage", amplitude);
  m_Controls.m_SaveAmplitude->setChecked(amplitude);
  m_Controls.m_SaveAmplitude->setEnabled(amplitude);
  //If the device provides an intensity image, we propably want to save it
  bool intensity = false;
  device->GetBoolProperty("HasIntensityImage", intensity);
  m_Controls.m_SaveIntensity->setChecked(intensity);
  m_Controls.m_SaveIntensity->setEnabled(intensity);

  //If the device provides an RGB image, we propably want to save it
  bool color = false;
  device->GetBoolProperty("HasRGBImage", color);
  m_Controls.m_SaveColor->setChecked(color);
  m_Controls.m_SaveColor->setEnabled(color);

  //If the device provides Raw image, we propably want to save it
  bool raw = false;
  device->GetBoolProperty("HasRawImage", raw);
  m_Controls.m_SaveRaw->setChecked(raw);
  m_Controls.m_SaveRaw->setEnabled(raw);

  //todo: where do i get correct file extensions?
  mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
  std::vector<std::string> fileExtensions = imageWriter->GetPossibleFileExtensions();
  QStringList extensions;
  for( unsigned int i = 0; i < fileExtensions.size(); ++i)
  {
    extensions.append(QString(fileExtensions.at(i).c_str()));
  }
  this->UpdateFileExtensionComboBox(true, m_Controls.m_DepthFormat, extensions, ".nrrd"); //usually you want to save depth data
  this->UpdateFileExtensionComboBox(amplitude, m_Controls.m_AmplitudeFormat, extensions, ".nrrd");
  this->UpdateFileExtensionComboBox(intensity, m_Controls.m_IntensityFormat, extensions, ".nrrd");
  this->UpdateFileExtensionComboBox(color, m_Controls.m_ColorFormat, extensions, ".png"); //png is nice default for calibration
  this->UpdateFileExtensionComboBox(raw, m_Controls.m_RawFormat, extensions, ".nrrd");
}

void QmitkToFScreenshotMaker::UpdateFileExtensionComboBox(bool active, QComboBox* box, QStringList fileExentions, const char* preferredFormat)
{
  box->clear();
  box->setEnabled(active);
  box->addItems(fileExentions);
  int index = box->findText(preferredFormat);
  if ( index != -1 ) { // -1 for not found
     box->setCurrentIndex(index);
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
  MITK_INFO << m_SavingCounter;
}

void QmitkToFScreenshotMaker::SaveImage(mitk::Image::Pointer image, bool saveImage, std::string path, std::string name, std::string extension)
{
  if(saveImage)
  {
    std::stringstream outdepthimage;
    outdepthimage << path << name<< m_SavingCounter << extension;
    MITK_INFO << outdepthimage.str();
    mitk::IOUtil::SaveImage( image, outdepthimage.str() );
  }
}
