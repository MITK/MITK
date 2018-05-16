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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "PAImageProcessing.h"

// Qt
#include <QMessageBox>
#include <QApplication>
#include <QMetaType>
#include <QFileDialog>

//mitk image
#include <mitkImage.h>
#include "mitkPhotoacousticImage.h"
#include "mitkPhotoacousticBeamformingFilter.h"

//other
#include <thread>
#include <functional>
#include <mitkIOUtil.h>

const std::string PAImageProcessing::VIEW_ID = "org.mitk.views.paimageprocessing";

PAImageProcessing::PAImageProcessing() : m_ResampleSpacing(0), m_UseLogfilter(false), m_FilterBank(mitk::PhotoacousticImage::New())
{
  qRegisterMetaType<mitk::Image::Pointer>();
  qRegisterMetaType<std::string>();
}

void PAImageProcessing::SetFocus()
{
  m_Controls.buttonApplyBModeFilter->setFocus();
}

void PAImageProcessing::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonApplyBModeFilter, SIGNAL(clicked()), this, SLOT(StartBmodeThread()));
  connect(m_Controls.DoResampling, SIGNAL(clicked()), this, SLOT(UseResampling()));
  connect(m_Controls.Logfilter, SIGNAL(clicked()), this, SLOT(UseLogfilter()));
  connect(m_Controls.ResamplingValue, SIGNAL(valueChanged(double)), this, SLOT(SetResampling()));
  connect(m_Controls.buttonApplyBeamforming, SIGNAL(clicked()), this, SLOT(StartBeamformingThread()));
  connect(m_Controls.buttonApplyCropFilter, SIGNAL(clicked()), this, SLOT(StartCropThread()));
  connect(m_Controls.buttonApplyBandpass, SIGNAL(clicked()), this, SLOT(StartBandpassThread()));
  connect(m_Controls.UseImageSpacing, SIGNAL(clicked()), this, SLOT(UseImageSpacing()));
  connect(m_Controls.ScanDepth, SIGNAL(valueChanged(double)), this, SLOT(UpdateImageInfo()));
  connect(m_Controls.SpeedOfSound, SIGNAL(valueChanged(double)), this, SLOT(UpdateImageInfo()));
  connect(m_Controls.SpeedOfSound, SIGNAL(valueChanged(double)), this, SLOT(ChangedSOSBeamforming()));
  connect(m_Controls.BPSpeedOfSound, SIGNAL(valueChanged(double)), this, SLOT(ChangedSOSBandpass()));
  connect(m_Controls.Samples, SIGNAL(valueChanged(int)), this, SLOT(UpdateImageInfo()));
  connect(m_Controls.UseImageSpacing, SIGNAL(clicked()), this, SLOT(UpdateImageInfo()));
  connect(m_Controls.boundLow, SIGNAL(valueChanged(int)), this, SLOT(LowerSliceBoundChanged()));
  connect(m_Controls.boundHigh, SIGNAL(valueChanged(int)), this, SLOT(UpperSliceBoundChanged()));
  connect(m_Controls.Partial, SIGNAL(clicked()), this, SLOT(SliceBoundsEnabled()));
  connect(m_Controls.BatchProcessing, SIGNAL(clicked()), this, SLOT(BatchProcessing()));
  connect(m_Controls.StepBeamforming, SIGNAL(clicked()), this, SLOT(UpdateSaveBoxes()));
  connect(m_Controls.StepCropping, SIGNAL(clicked()), this, SLOT(UpdateSaveBoxes()));
  connect(m_Controls.StepBandpass, SIGNAL(clicked()), this, SLOT(UpdateSaveBoxes()));
  connect(m_Controls.StepBMode, SIGNAL(clicked()), this, SLOT(UpdateSaveBoxes()));

  UpdateSaveBoxes();
  m_Controls.DoResampling->setChecked(false);
  m_Controls.ResamplingValue->setEnabled(false);
  m_Controls.progressBar->setMinimum(0);
  m_Controls.progressBar->setMaximum(100);
  m_Controls.progressBar->setVisible(false);
  m_Controls.UseImageSpacing->setToolTip("Image spacing of y-Axis must be in us, x-Axis in mm.");
  m_Controls.UseImageSpacing->setToolTipDuration(5000);
  m_Controls.ProgressInfo->setVisible(false);
  m_Controls.UseBP->hide();
  m_Controls.UseGPUBmode->hide();

  #ifndef PHOTOACOUSTICS_USE_GPU
    m_Controls.UseGPUBf->setEnabled(false);
    m_Controls.UseGPUBf->setChecked(false);
    m_Controls.UseGPUBmode->setEnabled(false);
    m_Controls.UseGPUBmode->setChecked(false);
  #endif
  
  UseImageSpacing();
}

void PAImageProcessing::ChangedSOSBandpass()
{
  m_Controls.SpeedOfSound->setValue(m_Controls.BPSpeedOfSound->value());
}

void PAImageProcessing::ChangedSOSBeamforming()
{
  m_Controls.BPSpeedOfSound->setValue(m_Controls.SpeedOfSound->value());
}

std::vector<std::string> splitpath(
  const std::string& str
  , const std::set<char> delimiters)
{
  std::vector<std::string> result;

  char const* pch = str.c_str();
  char const* start = pch;
  for (; *pch; ++pch)
  {
    if (delimiters.find(*pch) != delimiters.end())
    {
      if (start != pch)
      {
        std::string str(start, pch);
        result.push_back(str);
      }
      else
      {
        result.push_back("");
      }
      start = pch + 1;
    }
  }
  result.push_back(start);

  return result;
}

void PAImageProcessing::UpdateSaveBoxes()
{
  if (m_Controls.StepBeamforming->isChecked())
    m_Controls.SaveBeamforming->setEnabled(true);
  else
    m_Controls.SaveBeamforming->setEnabled(false);

  if (m_Controls.StepCropping->isChecked())
    m_Controls.SaveCropping->setEnabled(true);
  else
    m_Controls.SaveCropping->setEnabled(false);

  if (m_Controls.StepBandpass->isChecked())
    m_Controls.SaveBandpass->setEnabled(true);
  else
    m_Controls.SaveBandpass->setEnabled(false);

  if (m_Controls.StepBMode->isChecked())
    m_Controls.SaveBMode->setEnabled(true);
  else
    m_Controls.SaveBMode->setEnabled(false);
}

void PAImageProcessing::BatchProcessing()
{
  QFileDialog LoadDialog(nullptr, "Select Files to be processed");
  LoadDialog.setFileMode(QFileDialog::FileMode::ExistingFiles);
  LoadDialog.setNameFilter(tr("Images (*.nrrd)"));
  LoadDialog.setViewMode(QFileDialog::Detail);

  QStringList fileNames;
  if (LoadDialog.exec())
    fileNames = LoadDialog.selectedFiles();

  QString saveDir = QFileDialog::getExistingDirectory(nullptr, tr("Select Directory To Save To"),
    "",
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  DisableControls();

  std::set<char> delims{'/'};

  bool doSteps[] = { m_Controls.StepBeamforming->isChecked(), m_Controls.StepCropping->isChecked() , m_Controls.StepBandpass->isChecked(), m_Controls.StepBMode->isChecked() };
  bool saveSteps[] = { m_Controls.SaveBeamforming->isChecked(), m_Controls.SaveCropping->isChecked() , m_Controls.SaveBandpass->isChecked(), m_Controls.SaveBMode->isChecked() };

  for (int fileNumber = 0; fileNumber < fileNames.size(); ++fileNumber)
  {
    m_Controls.progressBar->setValue(0);
    m_Controls.progressBar->setVisible(true);
    m_Controls.ProgressInfo->setVisible(true);
    m_Controls.ProgressInfo->setText("loading file");

    QString filename = fileNames.at(fileNumber);
    auto split = splitpath(filename.toStdString(), delims);
    std::string imageName = split.at(split.size()-1);

    // remove ".nrrd"
    imageName = imageName.substr(0, imageName.size()-5);

    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(filename.toStdString().c_str());

    UpdateBFSettings(image);
    // Beamforming
    if (doSteps[0])
    {
      std::function<void(int, std::string)> progressHandle = [this](int progress, std::string progressInfo) {
        this->UpdateProgress(progress, progressInfo);
      };
      m_Controls.progressBar->setValue(100);
      std::string errorMessage = "";

      image = m_FilterBank->ApplyBeamforming(image, BFconfig, errorMessage, progressHandle);

      if (saveSteps[0])
      {
        std::string saveFileName = saveDir.toStdString() + "/" + imageName + " beamformed" + ".nrrd";
        mitk::IOUtil::Save(image, saveFileName);
      }
    }

    // Cropping
    if (doSteps[1])
    {
      m_Controls.ProgressInfo->setText("cropping image");

      image = m_FilterBank->ApplyCropping(image, m_Controls.CutoffAbove->value(), m_Controls.CutoffBelow->value(), 0, 0, 0, image->GetDimension(2) - 1);

      if (saveSteps[1])
      {
        std::string saveFileName = saveDir.toStdString() + "/" + imageName + " cropped" + ".nrrd";
        mitk::IOUtil::Save(image, saveFileName);
      }
    }

    // Bandpass
    if (doSteps[2])
    {
      m_Controls.ProgressInfo->setText("applying bandpass");
      float recordTime = image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] / 1000 / m_Controls.BPSpeedOfSound->value();
      // add a safeguard so the program does not chrash when applying a Bandpass that reaches out of the bounds of the image
      float maxFrequency = 1 / (recordTime / image->GetDimension(1)) * image->GetDimension(1) / 2 / 2 / 1000;
      float BPHighPass = 1000000 * m_Controls.BPhigh->value(); // [Hz]
      float BPLowPass = maxFrequency - 1000000 * m_Controls.BPlow->value(); // [Hz]

      if (BPLowPass > maxFrequency && m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too low, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPLowPass < 0 && m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too high, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPHighPass > maxFrequency &&  m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass too high, disabled it.");
        Msgbox.exec();

        BPHighPass = 0;
      }
      if (BPHighPass > maxFrequency - BFconfig.BPLowPass)
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass higher than LowPass, disabled both.");
        Msgbox.exec();

        BPHighPass = 0;
        BPLowPass = 0;
      }

      image = m_FilterBank->BandpassFilter(image, recordTime, BPHighPass, BPLowPass, m_Controls.BPFalloff->value());

      if (saveSteps[2])
      {
        std::string saveFileName = saveDir.toStdString() + "/" + imageName + " bandpassed" + ".nrrd";
        mitk::IOUtil::Save(image, saveFileName);
      }
    }
    // Bmode    
    if (doSteps[3])
    {
      m_Controls.ProgressInfo->setText("applying bmode filter");
      bool useGPU = m_Controls.UseGPUBmode->isChecked();
      
      if (m_Controls.BModeMethod->currentText() == "Absolute Filter")
        image = m_FilterBank->ApplyBmodeFilter(image, mitk::PhotoacousticImage::BModeMethod::Abs, useGPU, m_UseLogfilter, m_ResampleSpacing);
      else if (m_Controls.BModeMethod->currentText() == "Envelope Detection")
        image = m_FilterBank->ApplyBmodeFilter(image, mitk::PhotoacousticImage::BModeMethod::EnvelopeDetection, useGPU, m_UseLogfilter, m_ResampleSpacing);
      
      if (saveSteps[3])
      {
        std::string saveFileName = saveDir.toStdString() + "/" + imageName + " bmode" + ".nrrd";
        mitk::IOUtil::Save(image, saveFileName);
      }
    }
    m_Controls.progressBar->setVisible(false);
  }

  EnableControls();
}

void PAImageProcessing::StartBeamformingThread()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataStorage::Pointer storage = this->GetDataStorage();

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(NULL, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      UpdateBFSettings(image);
      std::stringstream message;
      std::string name;
      message << "Performing beamforming for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
        m_OldNodeName = name;
      }
      else
        m_OldNodeName = " ";

      message << ".";
      MITK_INFO << message.str();

      m_Controls.progressBar->setValue(0);
      m_Controls.progressBar->setVisible(true);
      m_Controls.ProgressInfo->setVisible(true);
      m_Controls.ProgressInfo->setText("started");
      m_Controls.buttonApplyBeamforming->setText("working...");
      DisableControls();

      BeamformingThread *thread = new BeamformingThread();
      connect(thread, &BeamformingThread::result, this, &PAImageProcessing::HandleBeamformingResults);
      connect(thread, &BeamformingThread::updateProgress, this, &PAImageProcessing::UpdateProgress);
      connect(thread, &BeamformingThread::message, this, &PAImageProcessing::PAMessageBox);
      connect(thread, &BeamformingThread::finished, thread, &QObject::deleteLater);

      thread->setConfig(BFconfig);
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Beamforming";
      thread->start();
    }
  }
}

void PAImageProcessing::HandleBeamformingResults(mitk::Image::Pointer image)
{
  auto newNode = mitk::DataNode::New();
  newNode->SetData(image);

  // name the new Data node
  std::stringstream newNodeName;

  newNodeName << m_OldNodeName << " ";

  if (BFconfig.Algorithm == mitk::BeamformingSettings::BeamformingAlgorithm::DAS)
    newNodeName << "DAS bf, ";
  else if (BFconfig.Algorithm == mitk::BeamformingSettings::BeamformingAlgorithm::DMAS)
    newNodeName << "DMAS bf, ";

  if (BFconfig.DelayCalculationMethod == mitk::BeamformingSettings::DelayCalc::QuadApprox)
    newNodeName << "q. delay";
  if (BFconfig.DelayCalculationMethod == mitk::BeamformingSettings::DelayCalc::Spherical)
    newNodeName << "s. delay";

  newNode->SetName(newNodeName.str());

  // update level window for the current dynamic range
  mitk::LevelWindow levelWindow;
  newNode->GetLevelWindow(levelWindow);
  levelWindow.SetAuto(image, true, true);
  newNode->SetLevelWindow(levelWindow);

  // add new node to data storage
  this->GetDataStorage()->Add(newNode);

  // disable progress bar
  m_Controls.progressBar->setVisible(false);
  m_Controls.ProgressInfo->setVisible(false);
  m_Controls.buttonApplyBeamforming->setText("Apply Beamforming");
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(image->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PAImageProcessing::StartBmodeThread()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataStorage::Pointer storage = this->GetDataStorage();

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(NULL, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      UpdateBFSettings(image);
      std::stringstream message;
      std::string name;
      message << "Performing image processing for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
        m_OldNodeName = name;
      }
      else
        m_OldNodeName = " ";

      message << ".";
      MITK_INFO << message.str();

      m_Controls.buttonApplyBModeFilter->setText("working...");
      DisableControls();

      BmodeThread *thread = new BmodeThread();
      connect(thread, &BmodeThread::result, this, &PAImageProcessing::HandleBmodeResults);
      connect(thread, &BmodeThread::finished, thread, &QObject::deleteLater);

      bool useGPU = m_Controls.UseGPUBmode->isChecked();

      if(m_Controls.BModeMethod->currentText() == "Absolute Filter")
        thread->setConfig(m_UseLogfilter, m_ResampleSpacing, mitk::PhotoacousticImage::BModeMethod::Abs, useGPU);
      else if(m_Controls.BModeMethod->currentText() == "Envelope Detection")
        thread->setConfig(m_UseLogfilter, m_ResampleSpacing, mitk::PhotoacousticImage::BModeMethod::EnvelopeDetection, useGPU);
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Image Processing";
      thread->start();
    }
  }
}

void PAImageProcessing::HandleBmodeResults(mitk::Image::Pointer image)
{
  auto newNode = mitk::DataNode::New();
  newNode->SetData(image);

  // name the new Data node
  std::stringstream newNodeName;
  newNodeName << m_OldNodeName << " ";
  newNodeName << "B-Mode";

  newNode->SetName(newNodeName.str());

  // update level window for the current dynamic range
  mitk::LevelWindow levelWindow;
  newNode->GetLevelWindow(levelWindow);
  auto data = newNode->GetData();
  levelWindow.SetAuto(dynamic_cast<mitk::Image*>(data), true, true);
  newNode->SetLevelWindow(levelWindow);

  // add new node to data storage
  this->GetDataStorage()->Add(newNode);

  // disable progress bar
  m_Controls.progressBar->setVisible(false);
  m_Controls.buttonApplyBModeFilter->setText("Apply B-mode Filter");
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(
    dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PAImageProcessing::StartCropThread()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataStorage::Pointer storage = this->GetDataStorage();

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(NULL, "Template", "Please load and select an image before starting image cropping.");
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      UpdateBFSettings(image);
      std::stringstream message;
      std::string name;
      message << "Performing image cropping for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
        m_OldNodeName = name;
      }
      else
        m_OldNodeName = " ";

      message << ".";
      MITK_INFO << message.str();

      m_Controls.buttonApplyCropFilter->setText("working...");
      DisableControls();

      CropThread *thread = new CropThread();
      connect(thread, &CropThread::result, this, &PAImageProcessing::HandleCropResults);
      connect(thread, &CropThread::finished, thread, &QObject::deleteLater);

      thread->setConfig(m_Controls.CutoffAbove->value(), m_Controls.CutoffBelow->value(), 0, image->GetDimension(2) - 1);
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Image Cropping";
      thread->start();
    }
  }
}

void PAImageProcessing::HandleCropResults(mitk::Image::Pointer image)
{
  auto newNode = mitk::DataNode::New();
  newNode->SetData(image);

  // name the new Data node
  std::stringstream newNodeName;
  newNodeName << m_OldNodeName << " ";
  newNodeName << "Cropped";

  newNode->SetName(newNodeName.str());

  // update level window for the current dynamic range
  mitk::LevelWindow levelWindow;
  newNode->GetLevelWindow(levelWindow);
  auto data = newNode->GetData();
  levelWindow.SetAuto(dynamic_cast<mitk::Image*>(data), true, true);
  newNode->SetLevelWindow(levelWindow);

  // add new node to data storage
  this->GetDataStorage()->Add(newNode);

  m_Controls.buttonApplyCropFilter->setText("Apply Crop Filter");
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(
    dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PAImageProcessing::StartBandpassThread()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataStorage::Pointer storage = this->GetDataStorage();

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(NULL, "Template", "Please load and select an image before starting image cropping.");
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      UpdateBFSettings(image);
      std::stringstream message;
      std::string name;
      message << "Performing Bandpass filter on image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
        m_OldNodeName = name;
      }
      else
        m_OldNodeName = " ";

      message << ".";
      MITK_INFO << message.str();

      m_Controls.buttonApplyBandpass->setText("working...");
      DisableControls();

      BandpassThread *thread = new BandpassThread();
      connect(thread, &BandpassThread::result, this, &PAImageProcessing::HandleBandpassResults);
      connect(thread, &BandpassThread::finished, thread, &QObject::deleteLater);

      float recordTime = image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] / 1000 / m_Controls.BPSpeedOfSound->value();

      // add a safeguard so the program does not chrash when applying a Bandpass that reaches out of the bounds of the image
      float maxFrequency = 1 / (recordTime / image->GetDimension(1)) * image->GetDimension(1) / 2 / 2 / 1000;
      float BPHighPass = 1000000 * m_Controls.BPhigh->value(); // [Hz]
      float BPLowPass = maxFrequency - 1000000 * m_Controls.BPlow->value(); // [Hz]

      if (BPLowPass > maxFrequency && m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too low, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPLowPass < 0 && m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too high, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPHighPass > maxFrequency &&  m_Controls.UseBP->isChecked())
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass too high, disabled it.");
        Msgbox.exec();

        BPHighPass = 0;
      }
      if (BPHighPass > maxFrequency - BFconfig.BPLowPass)
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass higher than LowPass, disabled both.");
        Msgbox.exec();

        BPHighPass = 0;
        BPLowPass = 0;
      }

      thread->setConfig(BPHighPass, BPLowPass, m_Controls.BPFalloff->value(), recordTime);
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Bandpass filter";
      thread->start();
    }
  }
}

void PAImageProcessing::HandleBandpassResults(mitk::Image::Pointer image)
{
  auto newNode = mitk::DataNode::New();
  newNode->SetData(image);

  // name the new Data node
  std::stringstream newNodeName;
  newNodeName << m_OldNodeName << " ";
  newNodeName << "Bandpassed";

  newNode->SetName(newNodeName.str());

  // update level window for the current dynamic range
  mitk::LevelWindow levelWindow;
  newNode->GetLevelWindow(levelWindow);
  auto data = newNode->GetData();
  levelWindow.SetAuto(dynamic_cast<mitk::Image*>(data), true, true);
  newNode->SetLevelWindow(levelWindow);

  // add new node to data storage
  this->GetDataStorage()->Add(newNode);

  m_Controls.buttonApplyBandpass->setText("Apply Bandpass");
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(
    dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void PAImageProcessing::SliceBoundsEnabled()
{
  if (!m_Controls.Partial->isChecked())
  {
    m_Controls.boundLow->setEnabled(false);
    m_Controls.boundHigh->setEnabled(false);
    return;
  }
  else
  {
    m_Controls.boundLow->setEnabled(true);
    m_Controls.boundHigh->setEnabled(true);
  }
}

void PAImageProcessing::UpperSliceBoundChanged()
{
  if(m_Controls.boundLow->value() > m_Controls.boundHigh->value())
  {
    m_Controls.boundLow->setValue(m_Controls.boundHigh->value());
  }
}

void PAImageProcessing::LowerSliceBoundChanged()
{
  if (m_Controls.boundLow->value() > m_Controls.boundHigh->value())
  {
    m_Controls.boundHigh->setValue(m_Controls.boundLow->value());
  }
}

void PAImageProcessing::UpdateProgress(int progress, std::string progressInfo)
{
  if (progress < 100)
    m_Controls.progressBar->setValue(progress);
  else
    m_Controls.progressBar->setValue(100);
  m_Controls.ProgressInfo->setText(progressInfo.c_str());
  qApp->processEvents();
}

void PAImageProcessing::PAMessageBox(std::string message)
{
  if (0 != message.compare("noMessage"))
  {
    QMessageBox msgBox;
    msgBox.setText(message.c_str());
    msgBox.exec();
  }
}

void PAImageProcessing::UpdateImageInfo()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);

    if (image)
    {
      // beamforming configs
      if (m_Controls.UseImageSpacing->isChecked())
      {
        m_Controls.ElementCount->setValue(image->GetDimension(0));
        m_Controls.Pitch->setValue(image->GetGeometry()->GetSpacing()[0]);
      }

      m_Controls.boundLow->setMaximum(image->GetDimension(2) - 1);
      m_Controls.boundHigh->setMaximum(image->GetDimension(2) - 1);

      UpdateBFSettings(image);

      m_Controls.CutoffBeforeBF->setValue(0.000001 / BFconfig.TimeSpacing); // 1us standard offset for our transducer

      std::stringstream frequency;
      float maxFrequency = (1 / BFconfig.TimeSpacing) * image->GetDimension(1) / 2 / 2 / 1000;
      frequency << maxFrequency / 1000000; //[MHz]
      frequency << "MHz";

      m_Controls.BPhigh->setMaximum(maxFrequency / 1000000);
      m_Controls.BPlow->setMaximum(maxFrequency / 1000000);

      frequency << " is the maximal allowed frequency for the selected image.";
      m_Controls.BPhigh->setToolTip(frequency.str().c_str());
      m_Controls.BPlow->setToolTip(frequency.str().c_str());
      m_Controls.BPhigh->setToolTipDuration(5000);
      m_Controls.BPlow->setToolTipDuration(5000);
    }
  }
}

void PAImageProcessing::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      m_Controls.labelWarning->setVisible( false );
      m_Controls.buttonApplyBModeFilter->setEnabled( true );
      m_Controls.labelWarning2->setVisible(false);
      m_Controls.buttonApplyCropFilter->setEnabled(true);
      m_Controls.labelWarning3->setVisible(false);
      m_Controls.buttonApplyBandpass->setEnabled(true);
      m_Controls.labelWarning4->setVisible(false);
      m_Controls.buttonApplyBeamforming->setEnabled(true);
      UpdateImageInfo();
      return;
    }
  }
  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonApplyBModeFilter->setEnabled( false );
  m_Controls.labelWarning2->setVisible(true);
  m_Controls.buttonApplyCropFilter->setEnabled(false);
  m_Controls.labelWarning3->setVisible(true);
  m_Controls.buttonApplyBandpass->setEnabled(false);
  m_Controls.labelWarning4->setVisible(true);
  m_Controls.buttonApplyBeamforming->setEnabled(false);
}

void PAImageProcessing::UseResampling()
{
  if (m_Controls.DoResampling->isChecked())
  {
    m_Controls.ResamplingValue->setEnabled(true);
    m_ResampleSpacing = m_Controls.ResamplingValue->value();
  }
  else
  {
    m_Controls.ResamplingValue->setEnabled(false);
    m_ResampleSpacing = 0;
  }
}

void PAImageProcessing::UseLogfilter()
{
  m_UseLogfilter = m_Controls.Logfilter->isChecked();
}

void PAImageProcessing::SetResampling()
{
  m_ResampleSpacing = m_Controls.ResamplingValue->value();
}

void PAImageProcessing::UpdateBFSettings(mitk::Image::Pointer image)
{
  if ("DAS" == m_Controls.BFAlgorithm->currentText())
    BFconfig.Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
  else if ("DMAS" == m_Controls.BFAlgorithm->currentText())
    BFconfig.Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DMAS;
  else if ("sDMAS" == m_Controls.BFAlgorithm->currentText())
    BFconfig.Algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS;

  if ("Quad. Approx." == m_Controls.DelayCalculation->currentText())
  {
    BFconfig.DelayCalculationMethod = mitk::BeamformingSettings::DelayCalc::QuadApprox;
  }
  else if ("Spherical Wave" == m_Controls.DelayCalculation->currentText())
  {
    BFconfig.DelayCalculationMethod = mitk::BeamformingSettings::DelayCalc::Spherical;
  }

  if ("Von Hann" == m_Controls.Apodization->currentText())
  {
    BFconfig.Apod = mitk::BeamformingSettings::Apodization::Hann;
  }
  else if ("Hamming" == m_Controls.Apodization->currentText())
  {
    BFconfig.Apod = mitk::BeamformingSettings::Apodization::Hamm;
  }
  else if ("Box" == m_Controls.Apodization->currentText())
  {
    BFconfig.Apod = mitk::BeamformingSettings::Apodization::Box;
  }

  BFconfig.Pitch = m_Controls.Pitch->value() / 1000; // [m]
  BFconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  BFconfig.SamplesPerLine = m_Controls.Samples->value();
  BFconfig.ReconstructionLines = m_Controls.Lines->value();
  BFconfig.TransducerElements = m_Controls.ElementCount->value();
  BFconfig.apodizationArraySize = m_Controls.Lines->value();
  BFconfig.Angle = m_Controls.Angle->value(); // [deg]
  BFconfig.UseBP = m_Controls.UseBP->isChecked();
  BFconfig.UseGPU = m_Controls.UseGPUBf->isChecked();
  BFconfig.upperCutoff = m_Controls.CutoffBeforeBF->value();

  if (m_Controls.UseImageSpacing->isChecked())
  {
    BFconfig.RecordTime = image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] / 1000000; // [s]
    BFconfig.TimeSpacing = image->GetGeometry()->GetSpacing()[1] / 1000000;
    MITK_INFO << "Calculated Scan Depth of " << BFconfig.RecordTime * BFconfig.SpeedOfSound * 100 / 2 << "cm";
  }
  else
  {
    BFconfig.RecordTime = 2 * m_Controls.ScanDepth->value() / 1000 / BFconfig.SpeedOfSound; // [s]
    BFconfig.TimeSpacing = BFconfig.RecordTime / image->GetDimension(1);
  }

  if ("US Image" == m_Controls.ImageType->currentText())
  {
    BFconfig.isPhotoacousticImage = false;
  }
  else if ("PA Image" == m_Controls.ImageType->currentText())
  {
    BFconfig.isPhotoacousticImage = true;
  }

  BFconfig.partial = m_Controls.Partial->isChecked();
  BFconfig.CropBounds[0] = m_Controls.boundLow->value();
  BFconfig.CropBounds[1] = m_Controls.boundHigh->value();
}

void PAImageProcessing::EnableControls()
{
  m_Controls.BatchProcessing->setEnabled(true);
  m_Controls.StepBeamforming->setEnabled(true);
  m_Controls.StepBandpass->setEnabled(true);
  m_Controls.StepCropping->setEnabled(true);
  m_Controls.StepBMode->setEnabled(true);

  UpdateSaveBoxes();

  m_Controls.DoResampling->setEnabled(true);
  UseResampling();
  m_Controls.Logfilter->setEnabled(true);
  m_Controls.BModeMethod->setEnabled(true);
  m_Controls.buttonApplyBModeFilter->setEnabled(true);

  m_Controls.CutoffAbove->setEnabled(true);
  m_Controls.CutoffBelow->setEnabled(true);
  m_Controls.CutoffBeforeBF->setEnabled(true);
  m_Controls.buttonApplyCropFilter->setEnabled(true);
  m_Controls.BPSpeedOfSound->setEnabled(true);
  m_Controls.buttonApplyBandpass->setEnabled(true);

  m_Controls.Partial->setEnabled(true);
  m_Controls.boundHigh->setEnabled(true);
  m_Controls.boundLow->setEnabled(true);
  m_Controls.BFAlgorithm->setEnabled(true);
  m_Controls.DelayCalculation->setEnabled(true);
  m_Controls.ImageType->setEnabled(true);
  m_Controls.Apodization->setEnabled(true);
  m_Controls.UseBP->setEnabled(true);

  #ifdef PHOTOACOUSTICS_USE_GPU
    m_Controls.UseGPUBf->setEnabled(true);
    m_Controls.UseGPUBmode->setEnabled(true);
  #endif

  m_Controls.BPhigh->setEnabled(true);
  m_Controls.BPlow->setEnabled(true);
  m_Controls.BPFalloff->setEnabled(true);
  m_Controls.UseImageSpacing->setEnabled(true);
  UseImageSpacing();
  m_Controls.Pitch->setEnabled(true);
  m_Controls.ElementCount->setEnabled(true);
  m_Controls.SpeedOfSound->setEnabled(true);
  m_Controls.Samples->setEnabled(true);
  m_Controls.Lines->setEnabled(true);
  m_Controls.Angle->setEnabled(true);
  m_Controls.buttonApplyBeamforming->setEnabled(true);
}

void PAImageProcessing::DisableControls()
{
  m_Controls.BatchProcessing->setEnabled(false);
  m_Controls.StepBeamforming->setEnabled(false);
  m_Controls.StepBandpass->setEnabled(false);
  m_Controls.StepCropping->setEnabled(false);
  m_Controls.StepBMode->setEnabled(false);
  m_Controls.SaveBeamforming->setEnabled(false);
  m_Controls.SaveBandpass->setEnabled(false);
  m_Controls.SaveCropping->setEnabled(false);
  m_Controls.SaveBMode->setEnabled(false);

  m_Controls.DoResampling->setEnabled(false);
  m_Controls.ResamplingValue->setEnabled(false);
  m_Controls.Logfilter->setEnabled(false);
  m_Controls.BModeMethod->setEnabled(false);
  m_Controls.buttonApplyBModeFilter->setEnabled(false);

  m_Controls.CutoffAbove->setEnabled(false);
  m_Controls.CutoffBelow->setEnabled(false);
  m_Controls.CutoffBeforeBF->setEnabled(false);
  m_Controls.buttonApplyCropFilter->setEnabled(false);
  m_Controls.BPSpeedOfSound->setEnabled(false);
  m_Controls.buttonApplyBandpass->setEnabled(false);

  m_Controls.Partial->setEnabled(false);
  m_Controls.boundHigh->setEnabled(false);
  m_Controls.boundLow->setEnabled(false);
  m_Controls.BFAlgorithm->setEnabled(false);
  m_Controls.DelayCalculation->setEnabled(false);
  m_Controls.ImageType->setEnabled(false);
  m_Controls.Apodization->setEnabled(false);
  m_Controls.UseBP->setEnabled(false);

  #ifdef PHOTOACOUSTICS_USE_GPU
    m_Controls.UseGPUBf->setEnabled(false);
    m_Controls.UseGPUBmode->setEnabled(false);
  #endif

  m_Controls.BPhigh->setEnabled(false);
  m_Controls.BPlow->setEnabled(false);
  m_Controls.BPFalloff->setEnabled(false);
  m_Controls.UseImageSpacing->setEnabled(false);
  m_Controls.ScanDepth->setEnabled(false);
  m_Controls.Pitch->setEnabled(false);
  m_Controls.ElementCount->setEnabled(false);
  m_Controls.SpeedOfSound->setEnabled(false);
  m_Controls.Samples->setEnabled(false);
  m_Controls.Lines->setEnabled(false);
  m_Controls.Angle->setEnabled(false);
  m_Controls.buttonApplyBeamforming->setEnabled(false);
}

void PAImageProcessing::UseImageSpacing()
{
  if (m_Controls.UseImageSpacing->isChecked())
  {
    m_Controls.ScanDepth->setDisabled(true);
  }
  else
  {
    m_Controls.ScanDepth->setEnabled(true);
  }
}

#include <mitkImageReadAccessor.h>

void BeamformingThread::run()
{
  mitk::Image::Pointer resultImage = mitk::Image::New();
  mitk::Image::Pointer resultImageBuffer;
  std::string errorMessage = "";
  std::function<void(int, std::string)> progressHandle = [this](int progress, std::string progressInfo) {
      emit updateProgress(progress, progressInfo);
    };

  resultImageBuffer = m_FilterBank->ApplyBeamforming(m_InputImage, m_BFconfig, errorMessage, progressHandle);
  mitk::ImageReadAccessor copy(resultImageBuffer);

  resultImage->Initialize(resultImageBuffer);
  resultImage->SetSpacing(resultImageBuffer->GetGeometry()->GetSpacing());
  resultImage->SetImportVolume(const_cast<void*>(copy.GetData()), 0, 0, mitk::Image::CopyMemory);

  emit result(resultImage);
  emit message(errorMessage);
}

void BeamformingThread::setConfig(mitk::BeamformingSettings BFconfig)
{
  m_BFconfig = BFconfig;
}

void BeamformingThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}

void BmodeThread::run()
{
  mitk::Image::Pointer resultImage;

  resultImage = m_FilterBank->ApplyBmodeFilter(m_InputImage, m_Method, m_UseGPU, m_UseLogfilter, m_ResampleSpacing);

  emit result(resultImage);
}

void BmodeThread::setConfig(bool useLogfilter, double resampleSpacing, mitk::PhotoacousticImage::BModeMethod method, bool useGPU)
{
  m_UseLogfilter = useLogfilter;
  m_ResampleSpacing = resampleSpacing;
  m_Method = method;
  m_UseGPU = useGPU;
}

void BmodeThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}

void CropThread::run()
{
  mitk::Image::Pointer resultImage;

  resultImage = m_FilterBank->ApplyCropping(m_InputImage, m_CutAbove, m_CutBelow, 0, 0, m_CutSliceFirst, m_CutSliceLast);

  emit result(resultImage);
}

void CropThread::setConfig(unsigned int CutAbove, unsigned int CutBelow, unsigned int CutSliceFirst, unsigned int CutSliceLast)
{
  m_CutAbove = CutAbove;
  m_CutBelow = CutBelow;
  m_CutSliceLast = CutSliceLast;
  m_CutSliceFirst = CutSliceFirst;
}

void CropThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}

void BandpassThread::run()
{
  mitk::Image::Pointer resultImage;

  resultImage = m_FilterBank->BandpassFilter(m_InputImage, m_RecordTime, m_BPHighPass, m_BPLowPass, m_TukeyAlpha);
  emit result(resultImage);
}

void BandpassThread::setConfig(float BPHighPass, float BPLowPass, float TukeyAlpha, float recordTime)
{
  m_BPHighPass = BPHighPass;
  m_BPLowPass = BPLowPass;
  m_TukeyAlpha = TukeyAlpha;
  m_RecordTime = recordTime;
}

void BandpassThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}
