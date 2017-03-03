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

//mitk image
#include <mitkImage.h>
#include "mitkPhotoacousticImage.h"
#include "Algorithms\mitkPhotoacousticBeamformingDASFilter.h"
#include "Algorithms\mitkPhotoacousticBeamformingDMASFilter.h"

const std::string PAImageProcessing::VIEW_ID = "org.mitk.views.paimageprocessing";

PAImageProcessing::PAImageProcessing() : m_ResampleSpacing(0), m_UseLogfilter(false)
{

}

void PAImageProcessing::SetFocus()
{
  m_Controls.buttonApplyBModeFilter->setFocus();
}

void PAImageProcessing::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.buttonApplyBModeFilter, SIGNAL(clicked()), this, SLOT(ApplyBModeFilter()) );
  connect(m_Controls.DoResampling, SIGNAL(clicked()), this, SLOT(UseResampling()));
  connect(m_Controls.Logfilter, SIGNAL(clicked()), this, SLOT(UseLogfilter()));
  connect(m_Controls.ResamplingValue, SIGNAL(valueChanged(double)), this, SLOT(SetResampling()));
  connect(m_Controls.buttonApplyBeamforming, SIGNAL(clicked()), this, SLOT(ApplyBeamforming()));

  m_Controls.DoResampling->setChecked(false);
  m_Controls.ResamplingValue->setEnabled(false);

  UpdateBFSettings();
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

void PAImageProcessing::ApplyBModeFilter()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

  mitk::DataNode::Pointer node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information( NULL, "Template", "Please load and select an image before starting image processing.");
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
      }
      message << ".";
      MITK_INFO << message.str();
      mitk::PhotoacousticImage::Pointer filterbank = mitk::PhotoacousticImage::New();
      node->SetData(filterbank->ApplyBmodeFilter(image, m_UseLogfilter, m_ResampleSpacing));

      // update level window for the current dynamic range
      mitk::LevelWindow levelWindow;
      node->GetLevelWindow(levelWindow);
      data = node->GetData();
      levelWindow.SetAuto(dynamic_cast<mitk::Image*>(data),true,true);
      node->SetLevelWindow(levelWindow);

      // update rendering
      mitk::RenderingManager::GetInstance()->InitializeViews(
        dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void PAImageProcessing::ApplyBeamforming()
{
  UpdateBFSettings();

  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;

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
      message << "Performing beamforming for image ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();
      mitk::PhotoacousticImage::Pointer filterbank = mitk::PhotoacousticImage::New();

      if(m_CurrentBeamformingAlgorithm == BeamformingAlgorithms::DAS)
        node->SetData(filterbank->ApplyBeamformingDAS(image, DASconfig));
      else if(m_CurrentBeamformingAlgorithm == BeamformingAlgorithms::DMAS)
        node->SetData(filterbank->ApplyBeamformingDMAS(image, DMASconfig));

      // update level window for the current dynamic range
      mitk::LevelWindow levelWindow;
      node->GetLevelWindow(levelWindow);
      data = node->GetData();
      levelWindow.SetAuto(dynamic_cast<mitk::Image*>(data), true, true);
      node->SetLevelWindow(levelWindow);

      // update rendering
      mitk::RenderingManager::GetInstance()->InitializeViews(
        dynamic_cast<mitk::Image*>(data)->GetGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void PAImageProcessing::UpdateBFSettings()
{
  if ("Delay and Sum" == m_Controls.BFAlgorithm->currentText())
    m_CurrentBeamformingAlgorithm = BeamformingAlgorithms::DAS;
  else if ("Delay, Multiply and Sum" == m_Controls.BFAlgorithm->currentText())
    m_CurrentBeamformingAlgorithm = BeamformingAlgorithms::DMAS;

  if ("Linear Waves" == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Linear;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::Linear;
  }
  else if ("Quadratic Approximation to Spherical Waves" == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::QuadApprox;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::QuadApprox;
  }
  else if ("Spherical Waves" == m_Controls.DelayCalculation->currentText())
  {
    DASconfig.DelayCalculationMethod = mitk::BeamformingDASFilter::beamformingSettings::DelayCalc::Spherical;
    DMASconfig.DelayCalculationMethod = mitk::BeamformingDMASFilter::beamformingSettings::DelayCalc::Spherical;
  }

  DASconfig.Pitch = m_Controls.Pitch->value() / 1000; // [m]
  DASconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  DASconfig.SamplesPerLine = m_Controls.Samples->value();
  DASconfig.ReconstructionLines = m_Controls.Lines->value();
  DASconfig.RecordTime = m_Controls.ScanDepth->value() / 1000 / DASconfig.SpeedOfSound * 2; // [s]
  DASconfig.TransducerElements = m_Controls.ElementCount->value();
  DASconfig.Angle = m_Controls.Angle->value();

  DMASconfig.Pitch = m_Controls.Pitch->value() / 1000; // [m]
  DMASconfig.SpeedOfSound = m_Controls.SpeedOfSound->value(); // [m/s]
  DMASconfig.SamplesPerLine = m_Controls.Samples->value();
  DMASconfig.ReconstructionLines = m_Controls.Lines->value();
  DMASconfig.RecordTime = m_Controls.ScanDepth->value() / 1000 / DMASconfig.SpeedOfSound * 2; // [s]
  DMASconfig.TransducerElements = m_Controls.ElementCount->value();
  DMASconfig.Angle = m_Controls.Angle->value();
}