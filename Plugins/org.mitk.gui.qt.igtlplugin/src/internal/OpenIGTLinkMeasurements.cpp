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
//
#include "OpenIGTLinkMeasurements.h"


const std::string OpenIGTLinkMeasurements::VIEW_ID = "org.mitk.views.OpenIGTLinkMeasurements";

OpenIGTLinkMeasurements::~OpenIGTLinkMeasurements()
{
}

void OpenIGTLinkMeasurements::CreateQtPartControl( QWidget *parent )
{
  //setup measurements
  this->m_Measurements = mitk::IGTLMeasurements::GetInstance();

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect(m_Controls.exportButton, SIGNAL(clicked()),
    this, SLOT(ExportButtonClicked()));
  connect(m_Controls.startButton, SIGNAL(clicked()),
    this, SLOT(StartStopButtonClicked()));
}

void OpenIGTLinkMeasurements::StartStopButtonClicked()
{
  if (this->m_Controls.startButton->text().contains("Start"))
  {
    //reset the measurements
    this->m_Measurements->Reset();
    //start the measurements
    this->m_Measurements->SetStarted(true);
    //adapt the gui
    this->m_Controls.startButton->setText("Stop");
    this->m_Controls.exportButton->setEnabled(false);
  }
  else
  {
    this->m_Measurements->SetStarted(false);
    this->m_Controls.startButton->setText("Start");
    this->m_Controls.exportButton->setEnabled(true);
  }
}

void OpenIGTLinkMeasurements::ExportButtonClicked()
{
   if (!m_Measurements->ExportData("MeasurementOutput.txt"))
      MITK_ERROR("OpenIGTLinkMeasurements") << "export did not work!!!";
}
