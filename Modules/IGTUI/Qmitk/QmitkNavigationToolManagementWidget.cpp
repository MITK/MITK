/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: QmitkTrackingDeviceWidget.cpp $
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkNavigationToolManagementWidget.h"

#include "mitkTrackingTypes.h"

#include <qfiledialog.h>

const std::string QmitkNavigationToolManagementWidget::VIEW_ID = "org.mitk.views.navigationtoolmanagementwidget";

QmitkNavigationToolManagementWidget::QmitkNavigationToolManagementWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
}


QmitkNavigationToolManagementWidget::~QmitkNavigationToolManagementWidget()
{
}

void QmitkNavigationToolManagementWidget::CreateQtPartControl(QWidget *parent)
  {
    if (!m_Controls)
    {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationToolManagementWidgetControls;
    m_Controls->setupUi(parent);
    }
  }

void QmitkNavigationToolManagementWidget::CreateConnections()
  {
    if ( m_Controls )
    {
      //connect( (QObject*)(m_Controls->m_step4_cancel), SIGNAL(clicked()), this, SLOT(AbortStep4()) );
      connect( (QObject*)(m_Controls->m_AddTool), SIGNAL(clicked()), this, SLOT(OnAddTool()) );
      connect( (QObject*)(m_Controls->m_AddToolCancel), SIGNAL(clicked()), this, SLOT(OnAddToolCancel()) );
      connect( (QObject*)(m_Controls->m_AddToolSave), SIGNAL(clicked()), this, SLOT(OnAddToolSave()) );
      connect( (QObject*)(m_Controls->m_LoadSurface), SIGNAL(clicked()), this, SLOT(OnLoadSurface()) );
      connect( (QObject*)(m_Controls->m_LoadCalibrationFile), SIGNAL(clicked()), this, SLOT(OnLoadCalibrationFile()) );

      
    }
  }

void QmitkNavigationToolManagementWidget::OnAddTool()
  {
    //reset input fields
    m_Controls->m_ToolNameEdit->setText("");
    m_Controls->m_IdentifierEdit->setText("");
    m_Controls->m_SerialNumberEdit->setText("");
    m_Controls->m_CalibrationFileName->setText("");
   
    //initialize UI components
    m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
    m_Controls->m_MainWidgets->setCurrentIndex(1);
    
  }

void QmitkNavigationToolManagementWidget::OnAddToolSave()
  {
    mitk::NavigationTool::Pointer newTool = mitk::NavigationTool::New();
    
    //create DataTreeNode...
    mitk::DataTreeNode::Pointer newNode = mitk::DataTreeNode::New();
    newNode->SetName(m_Controls->m_ToolNameEdit->text().toLatin1());
    //TODO: Surface Laden und als Data an die Node hängen!
    m_DataStorage->Add(newNode);

    //fill NavigationTool object
    newTool->SetCalibrationFile(std::string(m_Controls->m_CalibrationFileName->text().toLatin1()));

    //this->m_NavigationTools.push_back();
  }

void QmitkNavigationToolManagementWidget::OnAddToolCancel()
  {
    m_Controls->m_MainWidgets->setCurrentIndex(0);
  }

void QmitkNavigationToolManagementWidget::OnLoadSurface()
  {
  }

void QmitkNavigationToolManagementWidget::OnLoadCalibrationFile()
  {
    m_Controls->m_CalibrationFileName->setText(QFileDialog::getOpenFileName(NULL,tr("Open Image"), "/", "*.*"));
  }

void QmitkNavigationToolManagementWidget::Initialize(mitk::DataStorage* dataStorage)
  {
  m_DataStorage = dataStorage;
  }