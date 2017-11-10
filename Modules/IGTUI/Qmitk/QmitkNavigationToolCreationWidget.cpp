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
#include <mitkIOUtil.h>
#include <QmitkIGTCommonHelper.h>
#include <QDialogButtonBox>

//poco headers
#include <Poco/Path.h>

// vtk
#include <vtkSphereSource.h>
#include <vtkConeSource.h>

const std::string QmitkNavigationToolCreationWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationwizardwidget";

QmitkNavigationToolCreationWidget::QmitkNavigationToolCreationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_ToolToBeEdited = mitk::NavigationTool::New();
  m_FinalTool = mitk::NavigationTool::New();
  m_ToolTransformationWidget = new QmitkInteractiveTransformationWidget();

  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();

  this->InitializeUIToolLandmarkLists();
  Initialize(nullptr, "");

  //Default values, which are not stored in tool
  m_Controls->m_CalibrationFileName->setText("none");
  m_Controls->m_Surface_Use_Sphere->setChecked(true);

  m_Controls->m_CalibrationLandmarksList->EnableEditButton(false);
  m_Controls->m_RegistrationLandmarksList->EnableEditButton(false);

  RefreshTrackingDeviceCollection();
}

QmitkNavigationToolCreationWidget::~QmitkNavigationToolCreationWidget()
{
  m_Controls->m_CalibrationLandmarksList->SetPointSetNode(NULL);
  m_Controls->m_RegistrationLandmarksList->SetPointSetNode(NULL);
  delete m_ToolTransformationWidget;
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
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_TrackingDeviceTypeChooser), SIGNAL(currentIndexChanged(int index)), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_ToolNameEdit), SIGNAL(textChanged(const QString &text)), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_ToolTypeChooser), SIGNAL(currentIndexChanged(int index)), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_IdentifierEdit), SIGNAL(textChanged(const QString &text)), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_SerialNumberEdit), SIGNAL(textChanged(const QString &text)), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisX), SIGNAL(valueChanged()), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisY), SIGNAL(valueChanged()), this, SLOT(GetGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisZ), SIGNAL(valueChanged()), this, SLOT(GetGuiElements()));

    //Buttons
    connect((QObject*)(m_Controls->m_LoadCalibrationFile), SIGNAL(clicked()), this, SLOT(OnLoadCalibrationFile()));
    connect(m_Controls->m_Surface_Use_Other, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceUseOtherToggled()));
    connect((QObject*)(m_Controls->m_LoadSurface), SIGNAL(clicked()), this, SLOT(OnLoadSurface()));
    connect((QObject*)(m_Controls->m_EditToolTip), SIGNAL(clicked()), this, SLOT(OnEditToolTip()));

    connect((QObject*)(m_ToolTransformationWidget), SIGNAL(EditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip)), this, 
      SLOT(OnEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip)));

    connect((QObject*)(m_Controls->m_cancel), SIGNAL(clicked()), this, SLOT(OnCancel()));
    connect((QObject*)(m_Controls->m_finished), SIGNAL(clicked()), this, SLOT(OnFinished()));
  }
}

void QmitkNavigationToolCreationWidget::Initialize(mitk::DataStorage* dataStorage, const std::string& supposedIdentifier, const std::string& supposedName)
{
  m_DataStorage = dataStorage;
  //initialize UI components
  m_Controls->m_SurfaceChooser->SetDataStorage(m_DataStorage);
  m_Controls->m_SurfaceChooser->SetAutoSelectNewItems(true);
  m_Controls->m_SurfaceChooser->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  //Create new tool, which should be edited/created
  m_ToolToBeEdited = nullptr;//Reset
  m_ToolToBeEdited = mitk::NavigationTool::New();//Reinitialize
  m_ToolToBeEdited->SetIdentifier(supposedIdentifier);

  //Create the default cone surface. Can be changed later on...
  //create DataNode...
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(supposedName);
  m_ToolToBeEdited->SetDataNode(newNode);//ToDo: Data storage ok?

  SetConeAsToolSurface();

  this->SetDefaultData(m_ToolToBeEdited);
}

void QmitkNavigationToolCreationWidget::SetConeAsToolSurface()
{
  //create small cone and use it as surface
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
  m_ToolToBeEdited->GetDataNode()->SetData(mySphere);
}

void QmitkNavigationToolCreationWidget::SetDefaultData(mitk::NavigationTool::Pointer DefaultTool)
{
  //Set Members. This can either be the new initialized tool from call of Initialize() or a tool which already exists in the toolStorage
  //TODO m_ToolToBeEdited = DefaultTool->Clone();//Todo implement clone function!!!!

  //Set all gui variables
  SetGuiElements();
}

void QmitkNavigationToolCreationWidget::SetGuiElements()
{
  //Block signals, so that we don't call SetGuiElements again. This is undone at the end o this function!
  this->blockSignals(true);

  //DeviceType
  int index = m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(m_ToolToBeEdited->GetTrackingDeviceType()));
  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(index);
  }

  m_Controls->m_ToolNameEdit->setText(QString(m_ToolToBeEdited->GetDataNode()->GetName().c_str()));
  m_Controls->m_CalibrationFileName->setText(QString(m_ToolToBeEdited->GetCalibrationFile().c_str()));


  m_Controls->m_SurfaceChooser->SetSelectedNode(m_ToolToBeEdited->GetDataNode());
  FillUIToolLandmarkLists(m_ToolToBeEdited->GetToolCalibrationLandmarks(), m_ToolToBeEdited->GetToolRegistrationLandmarks());

  switch (m_ToolToBeEdited->GetType())
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

  m_Controls->m_IdentifierEdit->setText(QString(m_ToolToBeEdited->GetIdentifier().c_str()));
  m_Controls->m_SerialNumberEdit->setText(QString(m_ToolToBeEdited->GetSerialNumber().c_str()));

  m_Controls->m_ToolAxisX->setValue(m_ToolToBeEdited->GetToolAxis()[0]);
  m_Controls->m_ToolAxisY->setValue(m_ToolToBeEdited->GetToolAxis()[1]);
  m_Controls->m_ToolAxisZ->setValue(m_ToolToBeEdited->GetToolAxis()[2]);
  QString _label = "(" +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[0], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[1], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[2], 'f', 1) + "), quat: [" +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[0], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[1], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[2], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[3], 'f', 2) + "]";
  m_Controls->m_ToolTipLabel->setText(_label);

  //Undo block signals. Don't remove it, if signals are still blocked at the beginning of this function!
  this->blockSignals(false);
}

void QmitkNavigationToolCreationWidget::OnSurfaceUseOtherToggled()
{
  m_Controls->m_LoadSurface->setEnabled(m_Controls->m_Surface_Use_Other->isChecked());
  if (m_Controls->m_Surface_Use_Sphere->isChecked())
    SetConeAsToolSurface();
}

void QmitkNavigationToolCreationWidget::OnLoadSurface()
{
  std::string filename = QFileDialog::getOpenFileName(NULL, tr("Open Surface"), QmitkIGTCommonHelper::GetLastFileLoadPath(), tr("STL (*.stl)")).toLatin1().data();
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
  try
  {
    mitk::IOUtil::Load(filename.c_str(), *m_DataStorage);
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception occured: " << e.what();
  }
  //Todo: ist der neue Knoten automatisch der erste oder wie bekomme ich denß
  m_ToolToBeEdited->GetDataNode()->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());
}

void QmitkNavigationToolCreationWidget::OnLoadCalibrationFile()
{
  QString fileName = QFileDialog::getOpenFileName(NULL, tr("Open Calibration File"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.*");
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(fileName);
  m_Controls->m_CalibrationFileName->setText(fileName);
  m_ToolToBeEdited->SetCalibrationFile(fileName.toStdString());
}
void QmitkNavigationToolCreationWidget::GetValuesFromGuiElements()
{

  //Tracking Device
  m_ToolToBeEdited->SetTrackingDeviceType(m_Controls->m_TrackingDeviceTypeChooser->currentText().toStdString());
  m_ToolToBeEdited->GetDataNode()->SetName(m_Controls->m_ToolNameEdit->text().toStdString());

  //Tool Landmarks
  mitk::PointSet::Pointer toolCalLandmarks, toolRegLandmarks;
  GetUIToolLandmarksLists(toolCalLandmarks, toolRegLandmarks);
  m_ToolToBeEdited->SetToolCalibrationLandmarks(toolCalLandmarks);
  m_ToolToBeEdited->SetToolRegistrationLandmarks(toolRegLandmarks);

  //Advanced
  if (m_Controls->m_ToolTypeChooser->currentText() == "Instrument") m_ToolToBeEdited->SetType(mitk::NavigationTool::Instrument);
  else if (m_Controls->m_ToolTypeChooser->currentText() == "Fiducial") m_ToolToBeEdited->SetType(mitk::NavigationTool::Fiducial);
  else if (m_Controls->m_ToolTypeChooser->currentText() == "Skinmarker") m_ToolToBeEdited->SetType(mitk::NavigationTool::Skinmarker);
  else m_FinalTool->SetType(mitk::NavigationTool::Unknown);

  m_ToolToBeEdited->SetIdentifier(m_Controls->m_IdentifierEdit->text().toLatin1().data());
  m_ToolToBeEdited->SetSerialNumber(m_Controls->m_SerialNumberEdit->text().toLatin1().data());

  //Tool Axis
  mitk::Point3D toolAxis;
  toolAxis.SetElement(0, (m_Controls->m_ToolAxisX->value()));
  toolAxis.SetElement(1, (m_Controls->m_ToolAxisY->value()));
  toolAxis.SetElement(2, (m_Controls->m_ToolAxisZ->value()));
  m_ToolToBeEdited->SetToolAxis(toolAxis);
}

mitk::NavigationTool::Pointer QmitkNavigationToolCreationWidget::GetCreatedTool()
{
  return m_FinalTool;
}


void QmitkNavigationToolCreationWidget::OnFinished()
{
  //here we create a new tool
  m_FinalTool = m_ToolToBeEdited->Clone();


  emit NavigationToolFinished();
}

void QmitkNavigationToolCreationWidget::OnCancel()
{
  Initialize(nullptr,"");//Reset everything to a fresh tool, like it was done in the constructor
  emit Canceled();
}


void QmitkNavigationToolCreationWidget::SetTrackingDeviceType(mitk::TrackingDeviceType type, bool changeable /*= true*/)
{
  //Adapt Gui
  int index = m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(type));

  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(index);
  }

  m_Controls->m_TrackingDeviceTypeChooser->setEditable(changeable);

  //Set data to member
  m_ToolToBeEdited->SetTrackingDeviceType(type);
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

void QmitkNavigationToolCreationWidget::OnEditToolTip()
{
  m_ToolTransformationWidget->SetGeometryPointer(m_ToolToBeEdited->GetDataNode()->GetData()->GetGeometry());
  m_ToolTransformationWidget->SetValues(m_ToolToBeEdited->GetToolTipTransform());



  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_ToolTransformationWidget);
  m_ToolEditDialog.setLayout(mainLayout);
  m_ToolEditDialog.setWindowTitle("Edit Tool Tip and Tool Orientation");
  m_ToolEditDialog.open();
}

void QmitkNavigationToolCreationWidget::OnEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip)
{
  m_ToolEditDialog.close();
  //This function is called, when the toolTipEdit view is closed.
  m_ToolToBeEdited->SetToolTipPosition(toolTip->GetOffset());
  mitk::NavigationData::Pointer tempND = mitk::NavigationData::New(toolTip);//Convert to Navigation data for simple transversion to quaternion
  m_ToolToBeEdited->SetToolTipOrientation(tempND->GetOrientation());

  //Update Label
  QString _label = "(" +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[0], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[1], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[2], 'f', 1) + "), quat: [" +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[0], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[1], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[2], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipOrientation()[3], 'f', 2) + "]";
  m_Controls->m_ToolTipLabel->setText(_label);
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
  FillUIToolLandmarkLists(mitk::PointSet::New(), mitk::PointSet::New());
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