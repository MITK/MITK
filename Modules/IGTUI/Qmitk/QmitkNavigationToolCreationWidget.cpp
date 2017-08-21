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

#include "QmitkNavigationToolCreationWidget.h"

//mitk headers
#include <mitkTrackingTypes.h>
#include <mitkSurface.h>
#include <mitkNavigationData.h>
#include <mitkRenderingManager.h>
#include "mitkTrackingDeviceTypeCollection.h"

//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <QDialog>
#include <mitkIOUtil.h>
#include <QmitkIGTCommonHelper.h>

//poco headers
#include <Poco/Path.h>

// vtk
#include <vtkSphereSource.h>
#include <vtkConeSource.h>

const std::string QmitkNavigationToolCreationWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationwizardwidget";

QmitkNavigationToolCreationWidget::QmitkNavigationToolCreationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
  m_AdvancedWidget = new QmitkNavigationToolCreationAdvancedWidget(this);
  m_AdvancedWidget->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
  m_AdvancedWidget->setWindowTitle("Tool Creation Advanced Options");
  m_AdvancedWidget->setModal(false);
  CreateQtPartControl(this);
  CreateConnections();

RefreshTrackingDeviceCollection();
}

QmitkNavigationToolCreationWidget::~QmitkNavigationToolCreationWidget()
{
  m_Controls->m_CalibrationLandmarksList->SetPointSetNode(nullptr);
  m_Controls->m_RegistrationLandmarksList->SetPointSetNode(nullptr);
  delete m_AdvancedWidget;
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
    connect( (QObject*)(m_Controls->m_ShowAdvancedOptionsPB), SIGNAL(toggled(bool)), this, SLOT(OnShowAdvancedOptions(bool)) );
    connect( (QObject*)(m_AdvancedWidget), SIGNAL(DialogCloseRequested()), this, SLOT(OnProcessDialogCloseRequest()) );
    connect( (QObject*)(m_AdvancedWidget), SIGNAL(RetrieveDataForManualToolTipManipulation()), this, SLOT(OnRetrieveDataForManualTooltipManipulation()) );

    connect( m_Controls->m_Surface_Use_Other, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceUseOtherToggled(bool)));
  }
}

void QmitkNavigationToolCreationWidget::Initialize(mitk::DataStorage* dataStorage, const std::string& supposedIdentifier, const std::string& supposedName)
{
  m_DataStorage = dataStorage;

  //initialize UI components
  m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
  m_Controls->m_SurfaceChooser->SetAutoSelectNewItems(true);
  m_Controls->m_SurfaceChooser->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  //set default data
  m_Controls->m_ToolNameEdit->setText(supposedName.c_str());
  m_Controls->m_CalibrationFileName->setText("none");
  m_Controls->m_Surface_Use_Sphere->setChecked(true);
  m_AdvancedWidget->SetDataStorage(m_DataStorage);
  m_Controls->m_IdentifierEdit->setText(supposedIdentifier.c_str());
  this->InitializeUIToolLandmarkLists();
  m_Controls->m_CalibrationLandmarksList->EnableEditButton(false);
  m_Controls->m_RegistrationLandmarksList->EnableEditButton(false);
}

void QmitkNavigationToolCreationWidget::SetTrackingDeviceType(mitk::TrackingDeviceType type, bool)
{
  int index = m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(type));

  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(index);
  }
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
  if(m_Controls->m_Surface_Use_Sphere->isChecked())
  {
    //create small sphere and use it as surface
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkConeSource *vtkData = vtkConeSource::New();
    vtkData->SetAngle(5.0);
    vtkData->SetResolution(50);
    vtkData->SetHeight(6.0f);
    vtkData->SetRadius(2.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    vtkData->Delete();
    newNode->SetData(mySphere);
  }
  else
  {
    newNode->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());
  }
  newNode->SetName(m_Controls->m_ToolNameEdit->text().toLatin1());

  m_CreatedTool->SetDataNode(newNode);

  //fill NavigationTool object
  m_CreatedTool->SetCalibrationFile(m_Controls->m_CalibrationFileName->text().toLatin1().data());
  m_CreatedTool->SetIdentifier(m_Controls->m_IdentifierEdit->text().toLatin1().data());
  m_CreatedTool->SetSerialNumber(m_Controls->m_SerialNumberEdit->text().toLatin1().data());

//Tracking Device
m_CreatedTool->SetTrackingDeviceType(m_Controls->m_TrackingDeviceTypeChooser->currentText().toStdString());

  //ToolType
  if (m_Controls->m_ToolTypeChooser->currentText()=="Instrument") m_CreatedTool->SetType(mitk::NavigationTool::Instrument);
  else if (m_Controls->m_ToolTypeChooser->currentText()=="Fiducial") m_CreatedTool->SetType(mitk::NavigationTool::Fiducial);
  else if (m_Controls->m_ToolTypeChooser->currentText()=="Skinmarker") m_CreatedTool->SetType(mitk::NavigationTool::Skinmarker);
  else m_CreatedTool->SetType(mitk::NavigationTool::Unknown);

  //Tool Tip
  mitk::NavigationData::Pointer tempND = mitk::NavigationData::New(m_AdvancedWidget->GetManipulatedToolTip());
  m_CreatedTool->SetToolTipOrientation(tempND->GetOrientation());
  m_CreatedTool->SetToolTipPosition(tempND->GetPosition());

  //Tool Landmarks
  mitk::PointSet::Pointer toolCalLandmarks, toolRegLandmarks;
  GetUIToolLandmarksLists(toolCalLandmarks,toolRegLandmarks);
  m_CreatedTool->SetToolCalibrationLandmarks(toolCalLandmarks);
  m_CreatedTool->SetToolRegistrationLandmarks(toolRegLandmarks);

  //Tool Axis
  mitk::Point3D toolAxis;
  toolAxis.SetElement(0, (m_Controls->m_ToolAxisX->value()));
  toolAxis.SetElement(1, (m_Controls->m_ToolAxisY->value()));
  toolAxis.SetElement(2, (m_Controls->m_ToolAxisZ->value()));
  m_CreatedTool->SetToolAxis(toolAxis);

  emit NavigationToolFinished();
}

void QmitkNavigationToolCreationWidget::OnCancel()
{
  m_CreatedTool = nullptr;

  emit Canceled();
}

void QmitkNavigationToolCreationWidget::OnLoadSurface()
{
  std::string filename = QFileDialog::getOpenFileName(nullptr,tr("Open Surface"), QmitkIGTCommonHelper::GetLastFileLoadPath(), tr("STL (*.stl)")).toLatin1().data();
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
  try
  {
    mitk::IOUtil::Load(filename.c_str(), *m_DataStorage);
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception occured: " << e.what();
  }
}

void QmitkNavigationToolCreationWidget::OnLoadCalibrationFile()
{
  QString fileName = QFileDialog::getOpenFileName(nullptr,tr("Open Calibration File"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.*");
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(fileName);
  m_Controls->m_CalibrationFileName->setText(fileName);
}

void QmitkNavigationToolCreationWidget::SetDefaultData(mitk::NavigationTool::Pointer DefaultTool)
{
m_Controls->m_ToolNameEdit->setText(QString(DefaultTool->GetDataNode()->GetName().c_str()));
m_Controls->m_IdentifierEdit->setText(QString(DefaultTool->GetIdentifier().c_str()));
m_Controls->m_SerialNumberEdit->setText(QString(DefaultTool->GetSerialNumber().c_str()));
m_AdvancedWidget->SetDefaultTooltip( DefaultTool->GetToolTipTransform() );
int index = m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(DefaultTool->GetTrackingDeviceType()));

if (index >= 0)
{
  m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(index);
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
  FillUIToolLandmarkLists(DefaultTool->GetToolCalibrationLandmarks(),DefaultTool->GetToolRegistrationLandmarks());
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

void QmitkNavigationToolCreationWidget::OnShowAdvancedOptions(bool state)
{
  if(state)
  {
    m_AdvancedWidget->show();
    m_AdvancedWidget->SetDefaultTooltip(m_AdvancedWidget->GetManipulatedToolTip()); //use the last one, if there is one
    m_AdvancedWidget->ReInitialize();

    // reinit the views with the new nodes
    mitk::DataStorage::SetOfObjects::ConstPointer rs = m_DataStorage->GetAll();
    mitk::TimeGeometry::Pointer bounds = m_DataStorage->ComputeBoundingGeometry3D(rs, "visible");    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  }
  else
  {
    m_AdvancedWidget->hide();
  }
}

void QmitkNavigationToolCreationWidget::OnProcessDialogCloseRequest()
{
  m_AdvancedWidget->hide();
  m_Controls->m_ShowAdvancedOptionsPB->setChecked(false);
}

void QmitkNavigationToolCreationWidget::OnRetrieveDataForManualTooltipManipulation()
{
  if(m_Controls->m_Surface_Use_Sphere->isChecked())
  {
    m_AdvancedWidget->SetToolTipSurface(true);
  }
  else
  {
    m_AdvancedWidget->SetToolTipSurface(false,
                                        dynamic_cast<mitk::DataNode*>(m_Controls->m_SurfaceChooser->GetSelectedNode().GetPointer()));
  }
}

void QmitkNavigationToolCreationWidget::OnSurfaceUseOtherToggled(bool checked)
{
  m_Controls->m_LoadSurface->setEnabled(checked);
}

void QmitkNavigationToolCreationWidget::FillUIToolLandmarkLists(mitk::PointSet::Pointer calLandmarks, mitk::PointSet::Pointer regLandmarks)
{
  m_calLandmarkNode->SetData(calLandmarks);
  m_regLandmarkNode->SetData(regLandmarks);
  m_Controls->m_CalibrationLandmarksList->SetPointSetNode(m_calLandmarkNode);
  m_Controls->m_RegistrationLandmarksList->SetPointSetNode(m_regLandmarkNode);
}

void QmitkNavigationToolCreationWidget::GetUIToolLandmarksLists(mitk::PointSet::Pointer& calLandmarks, mitk::PointSet::Pointer& regLandmarks)
{
  calLandmarks = dynamic_cast<mitk::PointSet*>(m_calLandmarkNode->GetData());
  regLandmarks = dynamic_cast<mitk::PointSet*>(m_regLandmarkNode->GetData());
}

void QmitkNavigationToolCreationWidget::InitializeUIToolLandmarkLists()
{
  m_calLandmarkNode = mitk::DataNode::New();
  m_regLandmarkNode = mitk::DataNode::New();
  FillUIToolLandmarkLists(mitk::PointSet::New(),mitk::PointSet::New());
}

void QmitkNavigationToolCreationWidget::RefreshTrackingDeviceCollection()
{
  us::ModuleContext* context = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();
  if (refs.empty())
  {
    MITK_WARN << "No tracking device service found!";
    return;
  }
  mitk::TrackingDeviceTypeCollection* _DeviceTypeCollection = context->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

  for (auto name : _DeviceTypeCollection->GetTrackingDeviceTypeNames())
  {
    //if the device is not included yet, add name to comboBox and widget to stackedWidget
    if (m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(name)) == -1)
    {
      m_Controls->m_TrackingDeviceTypeChooser->addItem(QString::fromStdString(name));
    }
  }
}
