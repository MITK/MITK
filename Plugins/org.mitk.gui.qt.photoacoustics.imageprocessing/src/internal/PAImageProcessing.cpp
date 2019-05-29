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
#include "mitkPhotoacousticFilterService.h"
#include "mitkCastToFloatImageFilter.h"
#include "mitkBeamformingFilter.h"

//other
#include <thread>
#include <functional>
#include <mitkIOUtil.h>

#define GPU_BATCH_SIZE 32

const std::string PAImageProcessing::VIEW_ID = "org.mitk.views.paimageprocessing";

PAImageProcessing::PAImageProcessing() : m_ResampleSpacing(0), m_UseLogfilter(false), m_FilterBank(mitk::PhotoacousticFilterService::New())
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
  connect(m_Controls.UseSignalDelay, SIGNAL(clicked()), this, SLOT(UseSignalDelay()));
  connect(m_Controls.IsBFImage, SIGNAL(clicked()), this, SLOT(UpdateImageInfo()));
  UpdateSaveBoxes();
  UseSignalDelay();
  m_Controls.DoResampling->setChecked(false);
  m_Controls.ResamplingValue->setEnabled(false);
  m_Controls.progressBar->setMinimum(0);
  m_Controls.progressBar->setMaximum(100);
  m_Controls.progressBar->setVisible(false);
  m_Controls.UseImageSpacing->setToolTip("Image spacing of y-Axis must be in us, x-Axis in mm.");
  m_Controls.UseImageSpacing->setToolTipDuration(5000);
  m_Controls.ProgressInfo->setVisible(false);
  m_Controls.UseGPUBmode->hide();

#ifndef PHOTOACOUSTICS_USE_GPU
  m_Controls.UseGPUBf->setEnabled(false);
  m_Controls.UseGPUBf->setChecked(false);
  m_Controls.UseGPUBmode->setEnabled(false);
  m_Controls.UseGPUBmode->setChecked(false);
#endif

  UseImageSpacing();
}

void PAImageProcessing::UseSignalDelay()
{
  if (m_Controls.UseSignalDelay->isChecked())
  {
    m_Controls.SignalDelay->setEnabled(true);
  }
  else
  {
    m_Controls.SignalDelay->setEnabled(false);
  }
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

  std::set<char> delims{ '/' };

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
    std::string imageName = split.at(split.size() - 1);

    // remove ".nrrd"
    imageName = imageName.substr(0, imageName.size() - 5);

    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(filename.toStdString().c_str());
    auto BFconfig = CreateBeamformingSettings(image);

    // Beamforming
    if (doSteps[0])
    {
      if (m_Controls.UseSignalDelay->isChecked())
      {
        float signalDelay = m_Controls.SignalDelay->value();
        if (signalDelay != 0)
        {
          int cropPixels = std::round(signalDelay / BFconfig->GetTimeSpacing() / 1000000);
          MITK_INFO << cropPixels;
          int errCode = 0;
          image = m_FilterBank->ApplyCropping(image, cropPixels, 0, 0, 0, 0, 0, &errCode);

          if (errCode == -1)
          {
            QMessageBox Msgbox;
            Msgbox.setText("It has been attempted to cut off more pixels than the image contains. Aborting batch processing.");
            Msgbox.exec();
            m_Controls.progressBar->setVisible(false);
            EnableControls();
            return;
          }

          BFconfig = mitk::BeamformingSettings::New(BFconfig->GetPitchInMeters(), BFconfig->GetSpeedOfSound(),
            BFconfig->GetTimeSpacing(), BFconfig->GetAngle(), BFconfig->GetIsPhotoacousticImage(), BFconfig->GetSamplesPerLine(),
            BFconfig->GetReconstructionLines(), image->GetDimensions(), BFconfig->GetReconstructionDepth(),
            BFconfig->GetUseGPU(), BFconfig->GetGPUBatchSize(), BFconfig->GetDelayCalculationMethod(), BFconfig->GetApod(),
            BFconfig->GetApodizationArraySize(), BFconfig->GetAlgorithm());
        }
      }

      std::function<void(int, std::string)> progressHandle = [this](int progress, std::string progressInfo) {
        this->UpdateProgress(progress, progressInfo);
      };
      m_Controls.progressBar->setValue(100);

      image = m_FilterBank->ApplyBeamforming(image, BFconfig, progressHandle);

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

      int errCode = 0;
      image = m_FilterBank->ApplyCropping(image, m_Controls.CutoffAbove->value(), m_Controls.CutoffBelow->value(), 0, 0, 0, 0, &errCode);

      if (errCode == -1)
      {
        QMessageBox Msgbox;
        Msgbox.setText("It has been attempted to cut off more pixels than the image contains. Aborting batch processing.");
        Msgbox.exec();
        m_Controls.progressBar->setVisible(false);
        EnableControls();
        return;
      }

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

      if (BPLowPass > maxFrequency)
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too low, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPLowPass < 0)
      {
        QMessageBox Msgbox;
        Msgbox.setText("LowPass too high, disabled it.");
        Msgbox.exec();

        BPLowPass = 0;
      }
      if (BPHighPass > maxFrequency)
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass too high, disabled it.");
        Msgbox.exec();

        BPHighPass = 0;
      }
      if (BPHighPass > maxFrequency - BPLowPass)
      {
        QMessageBox Msgbox;
        Msgbox.setText("HighPass higher than LowPass, disabled both.");
        Msgbox.exec();

        BPHighPass = 0;
        BPLowPass = 0;
      }

      image = m_FilterBank->ApplyBandpassFilter(image, BPHighPass, BPLowPass,
        m_Controls.BPFalloffHigh->value(),
        m_Controls.BPFalloffLow->value(),
        BFconfig->GetTimeSpacing(),
        BFconfig->GetSpeedOfSound(),
        m_Controls.IsBFImage->isChecked());

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

      if (m_Controls.BModeMethod->currentText() == "Absolute Filter")
        image = m_FilterBank->ApplyBmodeFilter(image, mitk::PhotoacousticFilterService::BModeMethod::Abs, m_UseLogfilter);
      else if (m_Controls.BModeMethod->currentText() == "Envelope Detection")
        image = m_FilterBank->ApplyBmodeFilter(image, mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection, m_UseLogfilter);

      if (m_ResampleSpacing != 0)
      {
        double desiredSpacing[2]{ image->GetGeometry()->GetSpacing()[0], m_ResampleSpacing };

        image = m_FilterBank->ApplyResampling(image, desiredSpacing);
      }

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
      auto BFconfig = CreateBeamformingSettings(image);
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
      DisableControls();

      BeamformingThread *thread = new BeamformingThread();
      connect(thread, &BeamformingThread::result, this, &PAImageProcessing::HandleResults);
      connect(thread, &BeamformingThread::updateProgress, this, &PAImageProcessing::UpdateProgress);
      connect(thread, &BeamformingThread::finished, thread, &QObject::deleteLater);

      thread->setConfig(BFconfig);

      if (m_Controls.UseSignalDelay->isChecked())
        thread->setSignalDelay(m_Controls.SignalDelay->value());

      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Beamforming";
      thread->start();
    }
  }
}

void PAImageProcessing::HandleResults(mitk::Image::Pointer image, std::string nameExtension)
{
  if (image == nullptr)
  {
    QMessageBox Msgbox;
    Msgbox.setText("An error has occurred during processing; please see the console output.");
    Msgbox.exec();

    // disable progress bar
    m_Controls.progressBar->setVisible(false);
    m_Controls.ProgressInfo->setVisible(false);
    EnableControls();

    return;
  }
  MITK_INFO << "Handling results...";
  auto newNode = mitk::DataNode::New();
  newNode->SetData(image);

  newNode->SetName(m_OldNodeName + nameExtension);

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
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(image->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  MITK_INFO << "Handling results...[Done]";
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

      DisableControls();

      BmodeThread *thread = new BmodeThread();
      connect(thread, &BmodeThread::result, this, &PAImageProcessing::HandleResults);
      connect(thread, &BmodeThread::finished, thread, &QObject::deleteLater);

      bool useGPU = m_Controls.UseGPUBmode->isChecked();

      if (m_Controls.BModeMethod->currentText() == "Absolute Filter")
        thread->setConfig(m_UseLogfilter, m_ResampleSpacing, mitk::PhotoacousticFilterService::BModeMethod::Abs, useGPU);
      else if (m_Controls.BModeMethod->currentText() == "Envelope Detection")
        thread->setConfig(m_UseLogfilter, m_ResampleSpacing, mitk::PhotoacousticFilterService::BModeMethod::EnvelopeDetection, useGPU);
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Image Processing";
      thread->start();
    }
  }
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

      DisableControls();

      CropThread *thread = new CropThread();
      connect(thread, &CropThread::result, this, &PAImageProcessing::HandleResults);
      connect(thread, &CropThread::finished, thread, &QObject::deleteLater);

      if(m_Controls.Partial->isChecked())
        thread->setConfig(m_Controls.CutoffAbove->value(), m_Controls.CutoffBelow->value(), m_Controls.boundLow->value(), m_Controls.boundHigh->value());
      else
        thread->setConfig(m_Controls.CutoffAbove->value(), m_Controls.CutoffBelow->value(), 0, image->GetDimension(2) - 1);

      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Image Cropping";
      thread->start();
    }
  }
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
    QMessageBox::information(NULL, "Template", "Please load and select an image before applying a bandpass filter.");
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if (image)
    {
      auto config = CreateBeamformingSettings(image);
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

      DisableControls();

      BandpassThread *thread = new BandpassThread();
      connect(thread, &BandpassThread::result, this, &PAImageProcessing::HandleResults);
      connect(thread, &BandpassThread::finished, thread, &QObject::deleteLater);

      float BPHighPass = 1000000.0f * m_Controls.BPhigh->value(); // [Now in Hz]
      float BPLowPass = 1000000.0f * m_Controls.BPlow->value(); // [Now in Hz]

      thread->setConfig(BPHighPass, BPLowPass, m_Controls.BPFalloffLow->value(), m_Controls.BPFalloffHigh->value(), 
        config->GetTimeSpacing(), config->GetSpeedOfSound(), m_Controls.IsBFImage->isChecked());
      thread->setInputImage(image);
      thread->setFilterBank(m_FilterBank);

      MITK_INFO << "Started new thread for Bandpass filter";
      thread->start();
    }
  }
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
  if (m_Controls.boundLow->value() > m_Controls.boundHigh->value())
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

      float speedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
      std::stringstream frequency;
      float timeSpacing;
      if (m_Controls.UseImageSpacing->isChecked())
      {
        timeSpacing = image->GetGeometry()->GetSpacing()[1] / 1000000.0f;
        MITK_INFO << "Calculated Scan Depth of " << (image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] /
          1000000) * speedOfSound * 100 / 2 << "cm";
      }
      else
      {
        timeSpacing = (2 * m_Controls.ScanDepth->value() / 1000 / speedOfSound) / image->GetDimension(1);
      }
      float maxFrequency = (1 / timeSpacing) / 2;
      if(m_Controls.IsBFImage->isChecked())
        maxFrequency = ( 1 / (image->GetGeometry()->GetSpacing()[1] / 1e3 / speedOfSound)) / 2;
      frequency << maxFrequency / 1e6; //[MHz]
      frequency << "MHz";

      m_Controls.BPhigh->setMaximum(maxFrequency / 1e6);
      m_Controls.BPlow->setMaximum(maxFrequency / 1e6);

      frequency << " is the maximal allowed frequency for the selected image.";
      m_Controls.BPhigh->setToolTip(frequency.str().c_str());
      m_Controls.BPlow->setToolTip(frequency.str().c_str());
      m_Controls.BPhigh->setToolTipDuration(5000);
      m_Controls.BPlow->setToolTipDuration(5000);
    }
  }
}

void PAImageProcessing::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
  const QList<mitk::DataNode::Pointer>& nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach(mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()))
    {
      m_Controls.labelWarning->setVisible(false);
      m_Controls.buttonApplyBModeFilter->setEnabled(true);
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
  m_Controls.labelWarning->setVisible(true);
  m_Controls.buttonApplyBModeFilter->setEnabled(false);
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

mitk::BeamformingSettings::Pointer PAImageProcessing::CreateBeamformingSettings(mitk::Image::Pointer image)
{
  mitk::BeamformingSettings::BeamformingAlgorithm algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
  if ("DAS" == m_Controls.BFAlgorithm->currentText())
    algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DAS;
  else if ("DMAS" == m_Controls.BFAlgorithm->currentText())
    algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::DMAS;
  else if ("sDMAS" == m_Controls.BFAlgorithm->currentText())
    algorithm = mitk::BeamformingSettings::BeamformingAlgorithm::sDMAS;

  mitk::BeamformingSettings::DelayCalc delay = mitk::BeamformingSettings::DelayCalc::Spherical;

  mitk::BeamformingSettings::Apodization apod = mitk::BeamformingSettings::Apodization::Box;
  if ("Von Hann" == m_Controls.Apodization->currentText())
  {
    apod = mitk::BeamformingSettings::Apodization::Hann;
  }
  else if ("Hamming" == m_Controls.Apodization->currentText())
  {
    apod = mitk::BeamformingSettings::Apodization::Hamm;
  }
  else if ("Box" == m_Controls.Apodization->currentText())
  {
    apod = mitk::BeamformingSettings::Apodization::Box;
  }

  float pitchInMeters = m_Controls.Pitch->value() / 1000; // [m]
  float speedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  unsigned int samplesPerLine = m_Controls.Samples->value();
  unsigned int reconstructionLines = m_Controls.Lines->value();
  unsigned int apodizatonArraySize = m_Controls.Lines->value();
  float angle = m_Controls.Angle->value(); // [deg]
  bool useGPU = m_Controls.UseGPUBf->isChecked();

  float timeSpacing;
  if (m_Controls.UseImageSpacing->isChecked())
  {
    timeSpacing = image->GetGeometry()->GetSpacing()[1] / 1000000.0f;
    MITK_INFO << "Calculated Scan Depth of " << (image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] /
      1000000) * speedOfSound * 100 / 2 << "cm";
  }
  else
  {
    timeSpacing = (2 * m_Controls.ScanDepth->value() / 1000 / speedOfSound) / image->GetDimension(1);
  }

  bool isPAImage = true;
  if ("US Image" == m_Controls.ImageType->currentText())
  {
    isPAImage = false;
  }
  else if ("PA Image" == m_Controls.ImageType->currentText())
  {
    isPAImage = true;
  }

  float reconstructionDepth = m_Controls.ReconstructionDepth->value() / 1000.f; // [m]

  return mitk::BeamformingSettings::New(pitchInMeters,
    speedOfSound, timeSpacing, angle, isPAImage, samplesPerLine, reconstructionLines,
    image->GetDimensions(), reconstructionDepth, useGPU, GPU_BATCH_SIZE, delay, apod,
    apodizatonArraySize, algorithm);
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
  m_Controls.buttonApplyCropFilter->setEnabled(true);
  m_Controls.BPSpeedOfSound->setEnabled(true);
  m_Controls.buttonApplyBandpass->setEnabled(true);

  m_Controls.Partial->setEnabled(true);
  m_Controls.boundHigh->setEnabled(true);
  m_Controls.boundLow->setEnabled(true);
  m_Controls.BFAlgorithm->setEnabled(true);
  m_Controls.ReconstructionDepth->setEnabled(true);
  m_Controls.ImageType->setEnabled(true);
  m_Controls.Apodization->setEnabled(true);

#ifdef PHOTOACOUSTICS_USE_GPU
  m_Controls.UseGPUBf->setEnabled(true);
  m_Controls.UseGPUBmode->setEnabled(true);
#endif

  m_Controls.BPhigh->setEnabled(true);
  m_Controls.BPlow->setEnabled(true);
  m_Controls.BPFalloffLow->setEnabled(true);
  m_Controls.BPFalloffHigh->setEnabled(true);
  m_Controls.UseImageSpacing->setEnabled(true);
  UseImageSpacing();
  m_Controls.Pitch->setEnabled(true);
  m_Controls.ElementCount->setEnabled(true);
  m_Controls.SpeedOfSound->setEnabled(true);
  m_Controls.Samples->setEnabled(true);
  m_Controls.Lines->setEnabled(true);
  m_Controls.Angle->setEnabled(true);
  m_Controls.buttonApplyBeamforming->setEnabled(true);
  m_Controls.UseSignalDelay->setEnabled(true);
  m_Controls.SignalDelay->setEnabled(true);
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
  m_Controls.buttonApplyCropFilter->setEnabled(false);
  m_Controls.BPSpeedOfSound->setEnabled(false);
  m_Controls.buttonApplyBandpass->setEnabled(false);

  m_Controls.Partial->setEnabled(false);
  m_Controls.boundHigh->setEnabled(false);
  m_Controls.boundLow->setEnabled(false);
  m_Controls.BFAlgorithm->setEnabled(false);
  m_Controls.ReconstructionDepth->setEnabled(false);
  m_Controls.ImageType->setEnabled(false);
  m_Controls.Apodization->setEnabled(false);

#ifdef PHOTOACOUSTICS_USE_GPU
  m_Controls.UseGPUBf->setEnabled(false);
  m_Controls.UseGPUBmode->setEnabled(false);
#endif

  m_Controls.BPhigh->setEnabled(false);
  m_Controls.BPlow->setEnabled(false);
  m_Controls.BPFalloffLow->setEnabled(false);
  m_Controls.BPFalloffHigh->setEnabled(false);
  m_Controls.UseImageSpacing->setEnabled(false);
  m_Controls.ScanDepth->setEnabled(false);
  m_Controls.Pitch->setEnabled(false);
  m_Controls.ElementCount->setEnabled(false);
  m_Controls.SpeedOfSound->setEnabled(false);
  m_Controls.Samples->setEnabled(false);
  m_Controls.Lines->setEnabled(false);
  m_Controls.Angle->setEnabled(false);
  m_Controls.buttonApplyBeamforming->setEnabled(false);
  m_Controls.UseSignalDelay->setEnabled(false);
  m_Controls.SignalDelay->setEnabled(false);
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
  if (m_SignalDelay != 0)
  {
    int cropPixels = std::round(m_SignalDelay / m_BFconfig->GetTimeSpacing() / 1000000);
    MITK_INFO << cropPixels;
    int errCode = 0;
    m_InputImage = m_FilterBank->ApplyCropping(m_InputImage, cropPixels, 0, 0, 0, 0, 0, &errCode);

    m_BFconfig = mitk::BeamformingSettings::New(m_BFconfig->GetPitchInMeters(), m_BFconfig->GetSpeedOfSound(),
      m_BFconfig->GetTimeSpacing(), m_BFconfig->GetAngle(), m_BFconfig->GetIsPhotoacousticImage(), m_BFconfig->GetSamplesPerLine(),
      m_BFconfig->GetReconstructionLines(), m_InputImage->GetDimensions(), m_BFconfig->GetReconstructionDepth(),
      m_BFconfig->GetUseGPU(), m_BFconfig->GetGPUBatchSize(), m_BFconfig->GetDelayCalculationMethod(), m_BFconfig->GetApod(),
      m_BFconfig->GetApodizationArraySize(), m_BFconfig->GetAlgorithm());
  }

  mitk::Image::Pointer resultImage;
  std::function<void(int, std::string)> progressHandle = [this](int progress, std::string progressInfo) {
    emit updateProgress(progress, progressInfo);
  };
  resultImage = m_FilterBank->ApplyBeamforming(m_InputImage, m_BFconfig, progressHandle);
  emit result(resultImage, "_bf");
}

void BeamformingThread::setConfig(mitk::BeamformingSettings::Pointer BFconfig)
{
  m_BFconfig = BFconfig;
}

void BeamformingThread::setSignalDelay(float delay)
{
  m_SignalDelay = delay;
}

void BeamformingThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}

void BmodeThread::run()
{
  mitk::Image::Pointer resultImage = m_FilterBank->ApplyBmodeFilter(m_InputImage,
    m_Method, m_UseLogfilter);

  if (m_ResampleSpacing != 0)
  {
    double desiredSpacing[2]{ m_InputImage->GetGeometry()->GetSpacing()[0], m_ResampleSpacing };
    resultImage = m_FilterBank->ApplyResampling(resultImage, desiredSpacing);
  }
  emit result(resultImage, "_bmode");
}

void BmodeThread::setConfig(bool useLogfilter, double resampleSpacing, mitk::PhotoacousticFilterService::BModeMethod method, bool useGPU)
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


  int errCode = 0;

  resultImage = m_FilterBank->ApplyCropping(m_InputImage, m_CutAbove, m_CutBelow, 0, 0, m_CutSliceFirst, (m_InputImage->GetDimension(2) - 1)  - m_CutSliceLast, &errCode);
  if (errCode == -1)
  {
    emit result(nullptr, "_cropped");
    return;
  }
  emit result(resultImage, "_cropped");
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
  mitk::Image::Pointer resultImage = m_FilterBank->ApplyBandpassFilter(m_InputImage, m_BPHighPass, m_BPLowPass, m_TukeyAlphaHighPass, m_TukeyAlphaLowPass, m_TimeSpacing, m_SpeedOfSound, m_IsBFImage);
  emit result(resultImage, "_bandpassed");
}

void BandpassThread::setConfig(float BPHighPass, float BPLowPass, float TukeyAlphaHighPass, float TukeyAlphaLowPass, float TimeSpacing, float SpeedOfSound, bool IsBFImage)
{
  m_BPHighPass = BPHighPass;
  m_BPLowPass = BPLowPass;
  m_TukeyAlphaHighPass = TukeyAlphaHighPass;
  m_TukeyAlphaLowPass = TukeyAlphaLowPass;
  m_TimeSpacing = TimeSpacing;
  m_SpeedOfSound = SpeedOfSound;
  m_IsBFImage = IsBFImage;
}

void BandpassThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}
