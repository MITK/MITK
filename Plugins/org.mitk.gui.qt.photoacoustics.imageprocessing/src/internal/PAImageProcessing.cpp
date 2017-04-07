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

//mitk image
#include <mitkImage.h>
#include "mitkPhotoacousticImage.h"
#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkPhotoacousticBeamformingDMASFilter.h"

//other
#include <thread>
#include <functional>

const std::string PAImageProcessing::VIEW_ID = "org.mitk.views.paimageprocessing";

PAImageProcessing::PAImageProcessing() : m_ResampleSpacing(0), m_UseLogfilter(false)
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
  connect(m_Controls.UseImageSpacing, SIGNAL(clicked()), this, SLOT(UseImageSpacing()));
  connect(m_Controls.ScanDepth, SIGNAL(valueChanged(double)), this, SLOT(UpdateFrequency()));
  connect(m_Controls.SpeedOfSound, SIGNAL(valueChanged(double)), this, SLOT(UpdateFrequency()));
  connect(m_Controls.Samples, SIGNAL(valueChanged(double)), this, SLOT(UpdateFrequency()));
  connect(m_Controls.UseImageSpacing, SIGNAL(clicked), this, SLOT(UpdateFrequency()));
  connect(m_Controls.UseBP, SIGNAL(clicked()), this, SLOT(UseBandpass()));

  m_Controls.DoResampling->setChecked(false);
  m_Controls.ResamplingValue->setEnabled(false);
  m_Controls.progressBar->setMinimum(0);
  m_Controls.progressBar->setMaximum(100);
  m_Controls.progressBar->setVisible(false);
  m_Controls.UseImageSpacing->setToolTip("Image spacing of y-Axis must be in us, x-Axis in mm.");
  m_Controls.UseImageSpacing->setToolTipDuration(5000);
  m_Controls.ProgressInfo->setVisible(false);

  UseImageSpacing();
  UseBandpass();
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
      connect(thread, &BeamformingThread::finished, thread, &QObject::deleteLater);

      thread->setBeamformingAlgorithm(m_CurrentBeamformingAlgorithm);
      thread->setConfigs(DMASconfig, DASconfig);
      thread->setInputImage(image);

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

  if (m_CurrentBeamformingAlgorithm == BeamformingAlgorithms::DAS)
    newNodeName << "DAS bf, ";
  else if (m_CurrentBeamformingAlgorithm == BeamformingAlgorithms::DMAS)
    newNodeName << "DMAS bf, ";

  if (DASconfig.DelayCalculationMethod == mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Linear)
    newNodeName << "l. delay";
  if (DASconfig.DelayCalculationMethod == mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::QuadApprox)
    newNodeName << "q. delay";
  if (DASconfig.DelayCalculationMethod == mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Spherical)
    newNodeName << "s. delay";

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
  m_Controls.ProgressInfo->setVisible(false);
  m_Controls.buttonApplyBeamforming->setText("Apply Beamforming");
  EnableControls();

  // update rendering
  mitk::RenderingManager::GetInstance()->InitializeViews(
    dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
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

      thread->setConfig(m_UseLogfilter, m_ResampleSpacing);
      thread->setInputImage(image);

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

void PAImageProcessing::UpdateProgress(int progress, std::string progressInfo)
{
  if (progress < 100)
    m_Controls.progressBar->setValue(progress);
  else
    m_Controls.progressBar->setValue(100);
  m_Controls.ProgressInfo->setText(progressInfo.c_str());
  qApp->processEvents();
}

void PAImageProcessing::UpdateFrequency()
{
  DMASconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  DMASconfig.SamplesPerLine = m_Controls.Samples->value();

  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected
    if (!m_Controls.UseImageSpacing->isChecked())
      UpdateRecordTime(mitk::Image::New());
    return;
  }

  mitk::BaseData* data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image* image = dynamic_cast<mitk::Image*>(data);
    if(m_Controls.UseImageSpacing->isChecked())
    {
      m_Controls.ElementCount->setValue(image->GetDimension(0));
      m_Controls.Pitch->setValue(image->GetGeometry()->GetSpacing()[0]);
      MITK_INFO << image->GetGeometry()->GetSpacing()[0];
      MITK_INFO << (double)image->GetGeometry()->GetSpacing()[0];
    }
    if (image)
      UpdateRecordTime(image);
  }

  std::stringstream frequency;
  frequency << 1 / (DMASconfig.RecordTime / DMASconfig.SamplesPerLine) * DMASconfig.SamplesPerLine / 2 / 2 / 1000 / 1000000; //[MHz]
  frequency << "MHz";

  frequency << " is the maximal allowed frequency for the selected image.";
  m_Controls.BPhigh->setToolTip(frequency.str().c_str());
  m_Controls.BPlow->setToolTip(frequency.str().c_str());
  m_Controls.BPhigh->setToolTipDuration(5000);
  m_Controls.BPlow->setToolTipDuration(5000);
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
      UpdateFrequency();
      return;
    }
  }
  m_Controls.labelWarning->setVisible( true );
  m_Controls.buttonApplyBModeFilter->setEnabled( false );
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
    m_CurrentBeamformingAlgorithm = BeamformingAlgorithms::DAS;
  else if ("DMAS" == m_Controls.BFAlgorithm->currentText())
    m_CurrentBeamformingAlgorithm = BeamformingAlgorithms::DMAS;

  if ("Plane Wave" == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Linear;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::Linear;
  }
  else if ("Quad. Approx." == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::QuadApprox;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::QuadApprox;
  }
  else if ("Spherical Wave" == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Spherical;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::Spherical;
  }

  if ("Von Hann" == m_Controls.Apodization->currentText())
  {
    DASconfig.Apod = mitk::BeamformingDASFilter::beamformingSettings::Apodization::Hann;
    DMASconfig.Apod = mitk::BeamformingDMASFilter::beamformingSettings::Apodization::Hann;
  }
  else if ("Hamming" == m_Controls.Apodization->currentText())
  {
    DASconfig.Apod = mitk::BeamformingDASFilter::beamformingSettings::Apodization::Hamm;
    DMASconfig.Apod = mitk::BeamformingDMASFilter::beamformingSettings::Apodization::Hamm;
  }
  else if ("Box" == m_Controls.Apodization->currentText())
  {
    DASconfig.Apod = mitk::BeamformingDASFilter::beamformingSettings::Apodization::Box;
    DMASconfig.Apod = mitk::BeamformingDMASFilter::beamformingSettings::Apodization::Box;
  }

  DASconfig.Pitch = m_Controls.Pitch->value() / 1000; // [m]
  DASconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  DASconfig.SamplesPerLine = m_Controls.Samples->value();
  DASconfig.ReconstructionLines = m_Controls.Lines->value();
  DASconfig.TransducerElements = m_Controls.ElementCount->value();
  DASconfig.Angle = m_Controls.Angle->value();
  DASconfig.BPHighPass = 1000000 * m_Controls.BPhigh->value();
  DASconfig.BPLowPass = 1000000 * (1 / (DMASconfig.RecordTime / DMASconfig.SamplesPerLine) * DMASconfig.SamplesPerLine / 2 / 2 / 1000 / 1000000 - m_Controls.BPlow->value());
  DASconfig.BPFalloff = m_Controls.BPFalloff->value();
  DASconfig.UseBP = m_Controls.UseBP->isChecked();

  DMASconfig.Pitch = m_Controls.Pitch->value() / 1000; // [m]
  DMASconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  DMASconfig.SamplesPerLine = m_Controls.Samples->value();
  DMASconfig.ReconstructionLines = m_Controls.Lines->value();
  DMASconfig.TransducerElements = m_Controls.ElementCount->value();
  DMASconfig.Angle = m_Controls.Angle->value();
  DMASconfig.BPHighPass = 1000000 * m_Controls.BPhigh->value();
  DMASconfig.BPLowPass = 1000000 * (1 / (DMASconfig.RecordTime / DMASconfig.SamplesPerLine) * DMASconfig.SamplesPerLine / 2 / 2 / 1000 /1000000 - m_Controls.BPlow->value());
  DMASconfig.BPFalloff = m_Controls.BPFalloff->value();
  DMASconfig.UseBP = m_Controls.UseBP->isChecked();

  UpdateRecordTime(image);

  // add a safeguard so the program does not chrash when applying a Bandpass that reaches out of the bounds of the image

  double maxFrequency = 1 / (DMASconfig.RecordTime / DMASconfig.SamplesPerLine) * DMASconfig.SamplesPerLine / 2 / 2 / 1000; // [Hz]

  if (DMASconfig.BPLowPass > maxFrequency && m_Controls.UseBP->isChecked())
  {
    QMessageBox Msgbox;
    Msgbox.setText("LowPass too low, disabled it.");
    Msgbox.exec();

    DMASconfig.BPLowPass = 0;
    DASconfig.BPLowPass = 0;
  }
  if (DMASconfig.BPLowPass < 0 && m_Controls.UseBP->isChecked())
  {
    QMessageBox Msgbox;
    Msgbox.setText("LowPass too high, disabled it.");
    Msgbox.exec();

    DMASconfig.BPLowPass = 0;
    DASconfig.BPLowPass = 0;
  }
  if (DMASconfig.BPHighPass > maxFrequency &&  m_Controls.UseBP->isChecked())
  {
    QMessageBox Msgbox;
    Msgbox.setText("HighPass too high, disabled it.");
    Msgbox.exec();

    DMASconfig.BPHighPass = 0;
    DASconfig.BPHighPass = 0;
  }
  if (DMASconfig.BPHighPass > DMASconfig.BPLowPass)
  {
    QMessageBox Msgbox;
    Msgbox.setText("HighPass higher than LowPass, disabled both.");
    Msgbox.exec();

    DMASconfig.BPHighPass = 0;
    DASconfig.BPHighPass = 0;
    DMASconfig.BPLowPass = 0;
    DASconfig.BPLowPass = 0;
  }
}

void PAImageProcessing::UpdateRecordTime(mitk::Image::Pointer image)
{
  if (m_Controls.UseImageSpacing->isChecked())
  {
    DASconfig.RecordTime = image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] / 1000000; // [s]
    DMASconfig.RecordTime = image->GetDimension(1)*image->GetGeometry()->GetSpacing()[1] / 1000000; // [s]
    MITK_INFO << "Calculated Scan Depth of " << DASconfig.RecordTime * DASconfig.SpeedOfSound * 100 << "cm";
  }
  else
  {
    DASconfig.RecordTime = m_Controls.ScanDepth->value() / 1000 / DASconfig.SpeedOfSound; // [s]
    DMASconfig.RecordTime = m_Controls.ScanDepth->value() / 1000 / DMASconfig.SpeedOfSound; // [s]
  }

  if ("US Image" == m_Controls.ImageType->currentText())
  {
    if (m_Controls.UseImageSpacing->isChecked())
    {
      DASconfig.RecordTime = DASconfig.RecordTime / 2; // [s]
      DMASconfig.RecordTime = DMASconfig.RecordTime / 2; // [s]
    }

    DASconfig.Photoacoustic = false;
    DMASconfig.Photoacoustic = false;
  }
  else if ("PA Image" == m_Controls.ImageType->currentText())
  {
    DASconfig.Photoacoustic = true;
    DMASconfig.Photoacoustic = true;
  }
}

void PAImageProcessing::EnableControls()
{
  m_Controls.DoResampling->setEnabled(true);
  UseResampling();
  m_Controls.Logfilter->setEnabled(true);
  m_Controls.buttonApplyBModeFilter->setEnabled(true);
  m_Controls.Cutoff->setEnabled(true);
  m_Controls.BFAlgorithm->setEnabled(true);
  m_Controls.DelayCalculation->setEnabled(true);
  m_Controls.ImageType->setEnabled(true);
  m_Controls.Apodization->setEnabled(true);
  m_Controls.UseBP->setEnabled(true);
  UseBandpass();
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
  m_Controls.DoResampling->setEnabled(false);
  m_Controls.ResamplingValue->setEnabled(false);
  m_Controls.Logfilter->setEnabled(false);
  m_Controls.buttonApplyBModeFilter->setEnabled(false);
  m_Controls.Cutoff->setEnabled(false);
  m_Controls.BFAlgorithm->setEnabled(false);
  m_Controls.DelayCalculation->setEnabled(false);
  m_Controls.ImageType->setEnabled(false);
  m_Controls.Apodization->setEnabled(false);
  m_Controls.UseBP->setEnabled(false);
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
  UpdateFrequency();
}

void PAImageProcessing::UseBandpass()
{
  if (m_Controls.UseBP->isChecked())
  {
    m_Controls.BPhigh->setEnabled(true);
    m_Controls.BPlow->setEnabled(true);
    m_Controls.BPFalloff->setEnabled(true);
  }
  else
  {
    m_Controls.BPhigh->setDisabled(true);
    m_Controls.BPlow->setDisabled(true);
    m_Controls.BPFalloff->setDisabled(true);
  }
  UpdateFrequency();
}

void BeamformingThread::run()
{
  mitk::Image::Pointer resultImage;
  mitk::PhotoacousticImage::Pointer filterbank = mitk::PhotoacousticImage::New();
  std::function<void(int, std::string)> progressHandle = [this](int progress, std::string progressInfo) {
      emit updateProgress(progress, progressInfo);
    };

  if (m_CurrentBeamformingAlgorithm == PAImageProcessing::BeamformingAlgorithms::DAS)
    resultImage = filterbank->ApplyBeamformingDAS(m_InputImage, m_DASconfig, m_Cutoff, progressHandle);
  else if (m_CurrentBeamformingAlgorithm == PAImageProcessing::BeamformingAlgorithms::DMAS)
    resultImage = filterbank->ApplyBeamformingDMAS(m_InputImage, m_DMASconfig, m_Cutoff, progressHandle);

   emit result(resultImage);
}

void BeamformingThread::setConfigs(mitk::BeamformingDMASFilter::beamformingSettings DMASconfig, mitk::BeamformingDASFilter::beamformingSettings DASconfig)
{
  m_DMASconfig = DMASconfig;
  m_DASconfig = DASconfig;
}

void BeamformingThread::setBeamformingAlgorithm(PAImageProcessing::BeamformingAlgorithms beamformingAlgorithm)
{
  m_CurrentBeamformingAlgorithm = beamformingAlgorithm;
}

void BeamformingThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}

void BeamformingThread::setCutoff(int cutoff)
{
  m_Cutoff = cutoff;
}

void BmodeThread::run()
{
  mitk::Image::Pointer resultImage;
  mitk::PhotoacousticImage::Pointer filterbank = mitk::PhotoacousticImage::New();

  resultImage = filterbank->ApplyBmodeFilter(m_InputImage, m_UseLogfilter, m_ResampleSpacing);

  emit result(resultImage);
}

void BmodeThread::setConfig(bool useLogfilter, double resampleSpacing)
{
  m_UseLogfilter = useLogfilter;
  m_ResampleSpacing = resampleSpacing;
}

void BmodeThread::setInputImage(mitk::Image::Pointer image)
{
  m_InputImage = image;
}