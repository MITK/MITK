/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <qfiledialog.h>

// MITK
#include <mitkNavigationToolStorageDeserializer.h>



const std::string QmitkMITKIGTTrackingToolboxView::VIEW_ID = "org.mitk.views.mitkigttrackingtoolbox";

QmitkMITKIGTTrackingToolboxView::QmitkMITKIGTTrackingToolboxView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
	
}

QmitkMITKIGTTrackingToolboxView::~QmitkMITKIGTTrackingToolboxView()
{
}


void QmitkMITKIGTTrackingToolboxView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMITKIGTTrackingToolboxViewControls;
    m_Controls->setupUi( parent );
 
    connect( m_Controls->m_LoadTools, SIGNAL(clicked()), this, SLOT(OnLoadTools()) );
  }
}


void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkMITKIGTTrackingToolboxView::OnLoadTools()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(NULL,tr("Open Toolfile"), "/", tr("All Files (*.*)")); //later perhaps: tr("Toolfile (*.tfl)"
  if (filename.isNull()) return;
  
  //initialize tool storage
  m_toolStorage = mitk::NavigationToolStorage::New();
  
  //read tool storage from disk
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  m_toolStorage = myDeserializer->Deserialize(filename.toStdString());
  if (m_toolStorage.IsNull()) 
	{
	MessageBox(myDeserializer->GetErrorMessage());
	m_toolStorage = NULL;
	return;
    }

  //update label
  QString toolLabel = QString("Loaded Tools: ") + QString::number(m_toolStorage->GetToolCount()) + " Tools from " + filename;
  m_Controls->m_toolLabel->setText(toolLabel);
}

void QmitkMITKIGTTrackingToolboxView::OnStartTracking()
{}

void QmitkMITKIGTTrackingToolboxView::OnStopTracking()
{}

void QmitkMITKIGTTrackingToolboxView::MessageBox(std::string s)
  {
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
  }



