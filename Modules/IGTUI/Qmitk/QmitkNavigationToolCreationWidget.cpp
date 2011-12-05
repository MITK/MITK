/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkNavigationToolCreationWidget.h"

//mitk headers
#include "mitkTrackingTypes.h"
#include <mitkSTLFileReader.h>
#include <mitkSurface.h>

//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>

//poco headers
#include <Poco/Path.h>

// vtk
#include <vtkSphereSource.h>

const std::string QmitkNavigationToolCreationWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationwizardwidget";

QmitkNavigationToolCreationWidget::QmitkNavigationToolCreationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
}


QmitkNavigationToolCreationWidget::~QmitkNavigationToolCreationWidget()
{
}

void QmitkNavigationToolCreationWidget::CreateQtPartControl(QWidget *parent)
  {
    if (!m_Controls)
    {
    // create GUI widgets
    m_Controls = new Ui::QmitkNavigationToolCreationWidgetControls;
    m_Controls->setupUi(parent);
    }
  }

void QmitkNavigationToolCreationWidget::CreateConnections()
  {
    if ( m_Controls )
    {
      connect( (QObject*)(m_Controls->m_cancel), SIGNAL(clicked()), this, SLOT(OnCancel()) );
      connect( (QObject*)(m_Controls->m_finished), SIGNAL(clicked()), this, SLOT(OnFinished()) );
      connect( (QObject*)(m_Controls->m_LoadSurface), SIGNAL(clicked()), this, SLOT(OnLoadSurface()) );
      connect( (QObject*)(m_Controls->m_LoadCalibrationFile), SIGNAL(clicked()), this, SLOT(OnLoadCalibrationFile()) );
    }
  }

void QmitkNavigationToolCreationWidget::Initialize(mitk::DataStorage* dataStorage, std::string supposedIdentifier)
  {
  m_DataStorage = dataStorage;
  m_Controls->m_IdentifierEdit->setText(QString(supposedIdentifier.c_str()));

  //initialize UI components
  m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
  m_Controls->m_SurfaceChooser->SetAutoSelectNewItems(true);
  m_Controls->m_SurfaceChooser->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  //set default data
  m_Controls->m_ToolNameEdit->setText("NewTool");
  m_Controls->m_CalibrationFileName->setText("<not given>");
  m_Controls->m_SerialNumberEdit->setText("<not given>");
  m_Controls->m_Surface_Use_Sphere->setChecked(true);
  m_Controls->m_ToolTypeChooser->setCurrentIndex(0);
  
  }

void QmitkNavigationToolCreationWidget::SetTrackingDeviceType(mitk::TrackingDeviceType type, bool changeable)
  {
  switch(type)
      {
      case mitk::NDIAurora:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);break;
      case mitk::NDIPolaris:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(1);break;
      case mitk::ClaronMicron:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(2);break;
      default:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);
      }
  m_Controls->m_TrackingDeviceTypeChooser->setEnabled(changeable);
  }

mitk::NavigationTool::Pointer QmitkNavigationToolCreationWidget::GetCreatedTool()
{
  return m_CreatedTool;
}

//##################################################################################
//############################## slots                  ############################
//##################################################################################

void QmitkNavigationToolCreationWidget::OnFinished()
  {
    //here we create a new tool
    m_CreatedTool = mitk::NavigationTool::New();

    //create DataNode...
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetName(m_Controls->m_ToolNameEdit->text().toLatin1());
    if(m_Controls->m_Surface_Use_Sphere->isChecked())
      {
      //create small sphere and use it as surface
      mitk::Surface::Pointer mySphere = mitk::Surface::New();
      vtkSphereSource *vtkData = vtkSphereSource::New();
      vtkData->SetRadius(3.0f);
      vtkData->SetCenter(0.0, 0.0, 0.0);
      vtkData->Update();
      mySphere->SetVtkPolyData(vtkData->GetOutput());
      vtkData->Delete();
      newNode->SetData(mySphere);
      }
    else {newNode->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());}

    m_CreatedTool->SetDataNode(newNode);
    
    //fill NavigationTool object
    m_CreatedTool->SetCalibrationFile(m_Controls->m_CalibrationFileName->text().toAscii().data());
    m_CreatedTool->SetIdentifier(m_Controls->m_IdentifierEdit->text().toAscii().data());
    m_CreatedTool->SetSerialNumber(m_Controls->m_SerialNumberEdit->text().toAscii().data());

    //Tracking Device
    if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="NDI Aurora") m_CreatedTool->SetTrackingDeviceType(mitk::NDIAurora);
    else if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="NDI Polaris") m_CreatedTool->SetTrackingDeviceType(mitk::NDIPolaris);
    else if (m_Controls->m_TrackingDeviceTypeChooser->currentText()=="Claron Technology Micron Tracker") m_CreatedTool->SetTrackingDeviceType(mitk::ClaronMicron);
    else m_CreatedTool->SetTrackingDeviceType(mitk::TrackingSystemNotSpecified);
    
    //ToolType
    if (m_Controls->m_ToolTypeChooser->currentText()=="Instrument") m_CreatedTool->SetType(mitk::NavigationTool::Instrument);
    else if (m_Controls->m_ToolTypeChooser->currentText()=="Fiducial") m_CreatedTool->SetType(mitk::NavigationTool::Fiducial);
    else if (m_Controls->m_ToolTypeChooser->currentText()=="Skinmarker") m_CreatedTool->SetType(mitk::NavigationTool::Skinmarker);
    else m_CreatedTool->SetType(mitk::NavigationTool::Unknown);

    emit NavigationToolFinished();
  }

void QmitkNavigationToolCreationWidget::OnCancel()
  {
    m_CreatedTool = NULL;
    emit Canceled();
  }

void QmitkNavigationToolCreationWidget::OnLoadSurface()
  {
    std::string filename = QFileDialog::getOpenFileName(NULL,tr("Open Surface"), "/", "*.stl").toLatin1().data();
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
      {
      stlReader->SetFileName( filename.c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      }

    if ( stlReader->GetOutput() == NULL );
    else
      {
      mitk::DataNode::Pointer newNode = mitk::DataNode::New();
      newNode->SetName(filename);
      newNode->SetData(stlReader->GetOutput());
      m_DataStorage->Add(newNode);
      }
  }

void QmitkNavigationToolCreationWidget::OnLoadCalibrationFile()
  {
    m_Controls->m_CalibrationFileName->setText(QFileDialog::getOpenFileName(NULL,tr("Open Calibration File"), "/", "*.*"));
  }

void QmitkNavigationToolCreationWidget::SetDefaultData(mitk::NavigationTool::Pointer DefaultTool)
  {
    m_Controls->m_ToolNameEdit->setText(QString(DefaultTool->GetDataNode()->GetName().c_str()));
    m_Controls->m_IdentifierEdit->setText(QString(DefaultTool->GetIdentifier().c_str()));
    m_Controls->m_SerialNumberEdit->setText(QString(DefaultTool->GetSerialNumber().c_str()));
    switch(DefaultTool->GetTrackingDeviceType())
      {
      case mitk::NDIAurora:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);break;
      case mitk::NDIPolaris:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(1);break;
      case mitk::ClaronMicron:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(2);break;
      default:
              m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(0);
      }
    m_Controls->m_CalibrationFileName->setText(QString(DefaultTool->GetCalibrationFile().c_str()));
    m_Controls->m_Surface_Use_Other->setChecked(true);
    switch(DefaultTool->GetType())
      {
      case mitk::NavigationTool::Instrument:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(0); break;
      case mitk::NavigationTool::Fiducial:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(1); break;
      case mitk::NavigationTool::Skinmarker:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(2); break;
      case mitk::NavigationTool::Unknown:
        m_Controls->m_ToolTypeChooser->setCurrentIndex(3); break;
      }

    m_Controls->m_SurfaceChooser->SetSelectedNode(DefaultTool->GetDataNode());

  }



//##################################################################################
//############################## internal help methods #############################
//##################################################################################
void QmitkNavigationToolCreationWidget::MessageBox(std::string s)
  {
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
}