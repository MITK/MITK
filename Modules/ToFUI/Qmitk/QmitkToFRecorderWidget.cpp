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

#define _USE_MATH_DEFINES
#include "QmitkToFRecorderWidget.h"

//QT headers
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QFileDialog>

//mitk headers
#include <mitkToFImageWriter.h>

//itk headers
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop
#include <itksys/SystemTools.hxx>

struct QFileDialogArgs;
class QFileDialogPrivate;

const std::string QmitkToFRecorderWidget::VIEW_ID = "org.mitk.views.qmitktofrecorderwidget";

QmitkToFRecorderWidget::QmitkToFRecorderWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_ToFImageRecorder = NULL;
  this->m_ToFImageGrabber = NULL;
  this->m_RecordMode = mitk::ToFImageRecorder::PerFrames;

  this-> m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFRecorderWidget::~QmitkToFRecorderWidget()
{
}

void QmitkToFRecorderWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    this->m_Controls = new Ui::QmitkToFRecorderWidgetControls;
    this->m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkToFRecorderWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_PlayButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnPlay()) );
    connect( (QObject*)(m_Controls->m_StopButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStop()) );
    connect( (QObject*)(m_Controls->m_StartRecordingButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStartRecorder()) );
    connect( (QObject*)(m_Controls->m_RecordModeComboBox), SIGNAL(currentIndexChanged(int)),(QObject*) this, SLOT(OnChangeRecordModeComboBox(int)) );

    connect(this, SIGNAL(RecordingStopped()), this, SLOT(OnRecordingStopped()), Qt::BlockingQueuedConnection);
  }
}

void QmitkToFRecorderWidget::SetParameter(mitk::ToFImageGrabber* toFImageGrabber, mitk::ToFImageRecorder* toFImageRecorder)
{
  this->m_ToFImageGrabber = toFImageGrabber;

  this->m_ToFImageRecorder = toFImageRecorder;

  this->m_StopRecordingCommand = CommandType::New();
  this->m_StopRecordingCommand->SetCallbackFunction(this, &QmitkToFRecorderWidget::StopRecordingCallback);
  this->m_ToFImageRecorder->RemoveAllObservers();
  this->m_ToFImageRecorder->AddObserver(itk::AbortEvent(), this->m_StopRecordingCommand);

  m_Controls->m_PlayButton->setChecked(false);
  m_Controls->m_PlayButton->setEnabled(true);
  m_Controls->m_StartRecordingButton->setChecked(false);
  m_Controls->m_RecorderGroupBox->setEnabled(true);
}

void QmitkToFRecorderWidget::StopRecordingCallback()
{
  emit RecordingStopped();
}

void QmitkToFRecorderWidget::ResetGUIToInitial()
{
  m_Controls->m_PlayButton->setChecked(false);
  m_Controls->m_PlayButton->setEnabled(true);
  m_Controls->m_RecorderGroupBox->setEnabled(false);
}

void QmitkToFRecorderWidget::OnRecordingStopped()
{
  m_Controls->m_StartRecordingButton->setChecked(false);
  m_Controls->m_RecorderGroupBox->setEnabled(true);
}

void QmitkToFRecorderWidget::OnStop()
{
  StopCamera();
  StopRecorder();

  ResetGUIToInitial();

  emit ToFCameraStopped();
}

void QmitkToFRecorderWidget::OnPlay()
{
  m_Controls->m_PlayButton->setChecked(true);
  m_Controls->m_PlayButton->setEnabled(false);
  m_Controls->m_RecorderGroupBox->setEnabled(true);
  this->repaint();

  StartCamera();

  emit ToFCameraStarted();
}

void QmitkToFRecorderWidget::StartCamera()
{
  if (!m_ToFImageGrabber->IsCameraActive())
  {
    m_ToFImageGrabber->StartCamera();
  }
}

void QmitkToFRecorderWidget::StopCamera()
{
  if( m_ToFImageGrabber.IsNotNull() )
    m_ToFImageGrabber->StopCamera();
}

void QmitkToFRecorderWidget::StopRecorder()
{
  if( m_ToFImageRecorder.IsNotNull() )
  {
    this->m_ToFImageRecorder->StopRecording();
  }
}

void QmitkToFRecorderWidget::OnStartRecorder()
{
  m_Controls->m_StartRecordingButton->setChecked(true);
  m_Controls->m_RecorderGroupBox->setEnabled(false);
  this->repaint();

  int numOfFrames = m_Controls->m_NumOfFramesSpinBox->value();
  try
  {
    bool fileOK = true;
    bool distanceImageSelected = true;
    bool amplitudeImageSelected = false;
    bool intensityImageSelected = false;
    bool rgbImageSelected = false;
    bool rawDataSelected = false;

    //Set check boxes in dialog according to device properties
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasAmplitudeImage",amplitudeImageSelected);
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasIntensityImage",intensityImageSelected);
    m_ToFImageGrabber->GetCameraDevice()->GetBoolProperty("HasRGBImage",rgbImageSelected);

    QString tmpFileName("");
    QString selectedFilter("");
    QString imageFileName("");
    mitk::ToFImageWriter::ToFImageType tofImageType;
    tmpFileName = QmitkToFRecorderWidget::getSaveFileName(tofImageType,
      distanceImageSelected, amplitudeImageSelected, intensityImageSelected, rgbImageSelected, rawDataSelected,
      NULL, "Save Image To...", imageFileName, "NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic);;Text (*.csv)", &selectedFilter);

    if (tmpFileName.isEmpty())
    {
      fileOK = false;
    }
    else
    {
      imageFileName = tmpFileName;
    }

    if (fileOK)
    {
      std::string dir = itksys::SystemTools::GetFilenamePath( imageFileName.toStdString() );
      std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( imageFileName.toStdString() );
      std::string extension = itksys::SystemTools::GetFilenameLastExtension( imageFileName.toStdString() );

      int integrationTime = this->m_ToFImageGrabber->GetIntegrationTime();
      int modulationFreq = this->m_ToFImageGrabber->GetModulationFrequency();

      QString integrationTimeStr;
      integrationTimeStr.setNum(integrationTime);
      QString modulationFreqStr;
      modulationFreqStr.setNum(modulationFreq);
      QString numOfFramesStr("");
      if (this->m_RecordMode == mitk::ToFImageRecorder::PerFrames)
      {
        numOfFramesStr.setNum(numOfFrames);
      }

      std::string distImageFileName = prepareFilename(dir, baseFilename, modulationFreqStr.toStdString(),
        integrationTimeStr.toStdString(), numOfFramesStr.toStdString(), extension, "_DistanceImage");
      MITK_INFO << "Save distance data to: " << distImageFileName;

      std::string amplImageFileName = prepareFilename(dir, baseFilename, modulationFreqStr.toStdString(),
        integrationTimeStr.toStdString(), numOfFramesStr.toStdString(), extension, "_AmplitudeImage");
      MITK_INFO << "Save amplitude data to: " << amplImageFileName;

      std::string intenImageFileName = prepareFilename(dir, baseFilename, modulationFreqStr.toStdString(),
        integrationTimeStr.toStdString(), numOfFramesStr.toStdString(), extension, "_IntensityImage");
      MITK_INFO << "Save intensity data to: " << intenImageFileName;

      std::string rgbImageFileName = prepareFilename(dir, baseFilename, modulationFreqStr.toStdString(),
        integrationTimeStr.toStdString(), numOfFramesStr.toStdString(), extension, "_RGBImage");
      MITK_INFO << "Save intensity data to: " << rgbImageFileName;

      if (selectedFilter.compare("Text (*.csv)") == 0)
      {
        this->m_ToFImageRecorder->SetFileFormat(".csv");
      }
      else if (selectedFilter.compare("PIC Images - deprecated (*.pic)") == 0)
      {
        //default
        this->m_ToFImageRecorder->SetFileFormat(".pic");

        QMessageBox::warning(NULL, "Deprecated File Format!",
          "Please note that *.pic file format is deprecated and not longer supported! The suggested file format for images is *.nrrd!");
      }
      else if (selectedFilter.compare("NRRD Images (*.nrrd)") == 0)
      {
        this->m_ToFImageRecorder->SetFileFormat(".nrrd");
      }
      else
      {
        QMessageBox::warning(NULL, "Unsupported file format!", "Please specify one of the supported file formats *.nrrd, *.csv!");
        return;
      }

      numOfFrames = m_Controls->m_NumOfFramesSpinBox->value();
      this->m_ToFImageRecorder->SetDistanceImageFileName(distImageFileName);
      this->m_ToFImageRecorder->SetAmplitudeImageFileName(amplImageFileName);
      this->m_ToFImageRecorder->SetIntensityImageFileName(intenImageFileName);
      this->m_ToFImageRecorder->SetRGBImageFileName(rgbImageFileName);
      this->m_ToFImageRecorder->SetToFImageType(tofImageType);
      this->m_ToFImageRecorder->SetDistanceImageSelected(distanceImageSelected);
      this->m_ToFImageRecorder->SetAmplitudeImageSelected(amplitudeImageSelected);
      this->m_ToFImageRecorder->SetIntensityImageSelected(intensityImageSelected);
      this->m_ToFImageRecorder->SetRGBImageSelected(rgbImageSelected);
      this->m_ToFImageRecorder->SetRecordMode(this->m_RecordMode);
      this->m_ToFImageRecorder->SetNumOfFrames(numOfFrames);

      emit RecordingStarted();
      this->m_ToFImageRecorder->StartRecording();
    }
    else
    {
      this->OnRecordingStopped();
    }
  }
  catch(std::exception& e)
  {
    QMessageBox::critical(NULL, "Error", QString(e.what()));
    this->OnRecordingStopped();
  }
}

QString QmitkToFRecorderWidget::getSaveFileName(mitk::ToFImageWriter::ToFImageType& tofImageType,
                                     bool& distanceImageSelected,
                                     bool& amplitudeImageSelected,
                                     bool& intensityImageSelected,
                                     bool& rgbImageSelected,
                                     bool& rawDataSelected,
                                     QWidget *parent,
                                     const QString &caption,
                                     const QString &dir,
                                     const QString &filter,
                                     QString *selectedFilter,
                                     QFileDialog::Options options
                                     )
{
  QString selectedFileName = "";
  QComboBox* combo = new QComboBox;
  combo->addItem("3D");
  combo->addItem("2D + t");

  QHBoxLayout* checkBoxGroup = new QHBoxLayout();

  QCheckBox* distanceImageCheckBox = new QCheckBox;
  distanceImageCheckBox->setText("Distance image");
  distanceImageCheckBox->setChecked(distanceImageSelected);

  QCheckBox* amplitudeImageCheckBox = new QCheckBox;
  amplitudeImageCheckBox->setText("Amplitude image");
  amplitudeImageCheckBox->setChecked(amplitudeImageSelected);
  amplitudeImageCheckBox->setEnabled(amplitudeImageSelected);
  if(!amplitudeImageSelected)
    amplitudeImageCheckBox->setToolTip(QString("This device does not provide amplitude data."));

  QCheckBox* intensityImageCheckBox = new QCheckBox;
  intensityImageCheckBox->setText("Intensity image");
  intensityImageCheckBox->setChecked(intensityImageSelected);
  intensityImageCheckBox->setEnabled(intensityImageSelected);
  if(!intensityImageSelected)
    intensityImageCheckBox->setToolTip(QString("This device does not provide intensity data."));

  QCheckBox* rgbImageCheckBox = new QCheckBox;
  rgbImageCheckBox->setText("RGB image");
  rgbImageCheckBox->setChecked(rgbImageSelected);
  rgbImageCheckBox->setEnabled(rgbImageSelected);
  if(!rgbImageSelected)
    rgbImageCheckBox->setToolTip(QString("This device does not provide RGB data."));

  QCheckBox* rawDataCheckBox = new QCheckBox;
  rawDataCheckBox->setText("Raw data");
  rawDataCheckBox->setChecked(false);
  rawDataCheckBox->setEnabled(false);

  checkBoxGroup->addWidget(distanceImageCheckBox);
  checkBoxGroup->addWidget(amplitudeImageCheckBox);
  checkBoxGroup->addWidget(intensityImageCheckBox);
  checkBoxGroup->addWidget(rgbImageCheckBox);
  checkBoxGroup->addWidget(rawDataCheckBox);

  QFileDialog* fileDialog = new QFileDialog(parent, caption, dir, filter);

  QLayout* layout = fileDialog->layout();
  QGridLayout* gridbox = qobject_cast<QGridLayout*>(layout);

  if (gridbox)
  {
    gridbox->addWidget(new QLabel("ToF-Image type:"));
    gridbox->addWidget(combo);
    int lastRow = gridbox->rowCount();
    gridbox->addLayout(checkBoxGroup, lastRow, 0, 1, -1);
  }

  fileDialog->setLayout(gridbox);
  fileDialog->setAcceptMode(QFileDialog::AcceptSave);

  if (selectedFilter)
  {
    fileDialog->selectNameFilter(*selectedFilter);
  }

  if (fileDialog->exec() == QDialog::Accepted)
  {
    if (selectedFilter)
    {
      *selectedFilter = fileDialog->selectedNameFilter();
    }

    if (combo->currentIndex() == 0)
    {
      tofImageType = mitk::ToFImageWriter::ToFImageType3D;
    }

    else
    {
      tofImageType = mitk::ToFImageWriter::ToFImageType2DPlusT;
    }

    distanceImageSelected = distanceImageCheckBox->isChecked();
    amplitudeImageSelected = amplitudeImageCheckBox->isChecked();
    intensityImageSelected = intensityImageCheckBox->isChecked();
    rgbImageSelected = rgbImageCheckBox->isChecked();
    rawDataSelected = rawDataCheckBox->isChecked();

    selectedFileName = fileDialog->selectedFiles().value(0);
  }

  delete fileDialog;
  return selectedFileName;
}

std::string QmitkToFRecorderWidget::prepareFilename(std::string dir,
                                                    std::string baseFilename,
                                                    std::string modulationFreq,
                                                    std::string integrationTime,
                                                    std::string numOfFrames,
                                                    std::string extension,
                                                    std::string imageType)
{
  std::string filenName("");
  filenName.append(dir);
  filenName.append("/");
  filenName.append(baseFilename);
  filenName.append("_MF");
  filenName.append(modulationFreq);
  filenName.append("_IT");
  filenName.append(integrationTime);
  filenName.append("_");
  filenName.append(numOfFrames);
  filenName.append("Images");
  filenName.append(imageType);
  filenName.append(extension);
  return filenName;
}

void QmitkToFRecorderWidget::OnChangeRecordModeComboBox(int index)
{
  if (index == 0)
  {
    this->m_RecordMode = mitk::ToFImageRecorder::PerFrames;
    m_Controls->m_NumOfFramesSpinBox->setEnabled(true);
  }
  else
  {
    this->m_RecordMode = mitk::ToFImageRecorder::Infinite;
    m_Controls->m_NumOfFramesSpinBox->setEnabled(false);
  }
}
