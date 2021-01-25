/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

const std::string QmitkNavigationToolCreationWidget::VIEW_ID = "org.mitk.views.navigationtoolcreationwizardwidget";

QmitkNavigationToolCreationWidget::QmitkNavigationToolCreationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
  m_ToolToBeEdited = mitk::NavigationTool::New();
  m_FinalTool = mitk::NavigationTool::New();
  m_ToolTransformationWidget = new QmitkInteractiveTransformationWidget();
  m_Controls = nullptr;

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

  OnSurfaceUseToggled();
}

QmitkNavigationToolCreationWidget::~QmitkNavigationToolCreationWidget()
{
  m_Controls->m_CalibrationLandmarksList->SetPointSetNode(nullptr);
  m_Controls->m_RegistrationLandmarksList->SetPointSetNode(nullptr);
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
    connect((QObject*)(m_Controls->m_TrackingDeviceTypeChooser), SIGNAL(currentIndexChanged(int)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_ToolNameEdit), SIGNAL(textChanged(const QString)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_ToolTypeChooser), SIGNAL(currentIndexChanged(int)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_IdentifierEdit), SIGNAL(textChanged(const QString)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_SerialNumberEdit), SIGNAL(textChanged(const QString)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisX), SIGNAL(valueChanged(int)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisY), SIGNAL(valueChanged(int)), this, SLOT(GetValuesFromGuiElements()));
    connect((QObject*)(m_Controls->m_ToolAxisZ), SIGNAL(valueChanged(int)), this, SLOT(GetValuesFromGuiElements()));

    //Buttons
    connect((QObject*)(m_Controls->m_LoadCalibrationFile), SIGNAL(clicked()), this, SLOT(OnLoadCalibrationFile()));
    connect(m_Controls->m_Surface_Use_Other, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceUseToggled()));
    connect(m_Controls->m_Surface_Load_File, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceUseToggled()));
    connect((QObject*)(m_Controls->m_LoadSurface), SIGNAL(clicked()), this, SLOT(OnLoadSurface()));
    connect((QObject*)(m_Controls->m_EditToolTip), SIGNAL(clicked()), this, SLOT(OnEditToolTip()));

    connect((QObject*)(m_ToolTransformationWidget), SIGNAL(EditToolTipFinished(mitk::AffineTransform3D::Pointer)), this,
      SLOT(OnEditToolTipFinished(mitk::AffineTransform3D::Pointer)));

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

  m_ToolToBeEdited->GetDataNode()->SetName(supposedName);

  this->SetDefaultData(m_ToolToBeEdited);
}

void QmitkNavigationToolCreationWidget::ShowToolPreview(std::string _name)
{
  m_DataStorage->Add(m_ToolToBeEdited->GetDataNode());
  m_ToolToBeEdited->GetDataNode()->SetName(_name);

  //change color to blue
  m_ToolToBeEdited->GetDataNode()->SetProperty("color", mitk::ColorProperty::New(0, 0, 1));

  //Global Reinit to show new tool
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
}

void QmitkNavigationToolCreationWidget::SetDefaultData(mitk::NavigationTool::Pointer DefaultTool)
{
  //Set Members. This can either be the new initialized tool from call of Initialize() or a tool which already exists in the toolStorage
  m_ToolToBeEdited = DefaultTool->Clone();

  //Set all gui variables
  SetGuiElements();
}

void QmitkNavigationToolCreationWidget::SetGuiElements()
{
  //Block signals, so that we don't call SetGuiElements again. This is undone at the end of this function!
  m_Controls->m_TrackingDeviceTypeChooser->blockSignals(true);
  m_Controls->m_ToolNameEdit->blockSignals(true);
  m_Controls->m_ToolTypeChooser->blockSignals(true);
  m_Controls->m_IdentifierEdit->blockSignals(true);
  m_Controls->m_SerialNumberEdit->blockSignals(true);
  m_Controls->m_ToolAxisX->blockSignals(true);
  m_Controls->m_ToolAxisY->blockSignals(true);
  m_Controls->m_ToolAxisZ->blockSignals(true);

  //DeviceType
  int index = m_Controls->m_TrackingDeviceTypeChooser->findText(QString::fromStdString(m_ToolToBeEdited->GetTrackingDeviceType()));
  if (index >= 0)
  {
    m_Controls->m_TrackingDeviceTypeChooser->setCurrentIndex(index);
  }

  m_Controls->m_ToolNameEdit->setText(QString(m_ToolToBeEdited->GetToolName().c_str()));
  m_Controls->m_CalibrationFileName->setText(QString(m_ToolToBeEdited->GetCalibrationFile().c_str()));

  FillUIToolLandmarkLists(m_ToolToBeEdited->GetToolControlPoints(), m_ToolToBeEdited->GetToolLandmarks());

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

  QString _label = "(" +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[0], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[1], 'f', 1) + ", " +
    QString::number(m_ToolToBeEdited->GetToolTipPosition()[2], 'f', 1) + "), quat: [" +
    QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[0], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[1], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[2], 'f', 2) + ", " +
    QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[3], 'f', 2) + "]";
  m_Controls->m_ToolTipLabel->setText(_label);

  //Undo block signals. Don't remove it, if signals are still blocked at the beginning of this function!
  m_Controls->m_TrackingDeviceTypeChooser->blockSignals(false);
  m_Controls->m_ToolNameEdit->blockSignals(false);
  m_Controls->m_ToolTypeChooser->blockSignals(false);
  m_Controls->m_IdentifierEdit->blockSignals(false);
  m_Controls->m_SerialNumberEdit->blockSignals(false);
  m_Controls->m_ToolAxisX->blockSignals(false);
  m_Controls->m_ToolAxisY->blockSignals(false);
  m_Controls->m_ToolAxisZ->blockSignals(false);
}

void QmitkNavigationToolCreationWidget::OnSurfaceUseToggled()
{
  if (m_Controls->m_Surface_Use_Sphere->isChecked()) m_ToolToBeEdited->SetDefaultSurface();
  m_Controls->m_SurfaceChooser->setEnabled(m_Controls->m_Surface_Use_Other->isChecked());
  m_Controls->m_LoadSurface->setEnabled(m_Controls->m_Surface_Load_File->isChecked());

  //Global Reinit to show tool surface preview
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
}

void QmitkNavigationToolCreationWidget::OnLoadSurface()
{
  std::string filename = QFileDialog::getOpenFileName(nullptr, tr("Open Surface"), QmitkIGTCommonHelper::GetLastFileLoadPath(), tr("STL (*.stl)")).toLatin1().data();
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
  mitk::Surface::Pointer surface;
  try
  {
    surface = mitk::IOUtil::Load<mitk::Surface>(filename.c_str());
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception occured: " << e.what();
    return;
  }

  m_ToolToBeEdited->GetDataNode()->SetData(surface);

  //Global Reinit to show tool surface or preview
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
}

void QmitkNavigationToolCreationWidget::OnLoadCalibrationFile()
{
  QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Calibration File"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.*");
  QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(fileName);
  m_Controls->m_CalibrationFileName->setText(fileName);
  m_ToolToBeEdited->SetCalibrationFile(fileName.toStdString());
}
void QmitkNavigationToolCreationWidget::GetValuesFromGuiElements()
{
  //Tracking Device
  m_ToolToBeEdited->SetTrackingDeviceType(m_Controls->m_TrackingDeviceTypeChooser->currentText().toStdString());
  //m_ToolToBeEdited->GetDataNode()->SetName(m_Controls->m_ToolNameEdit->text().toStdString());

  //Tool Landmarks
  mitk::PointSet::Pointer toolCalLandmarks, toolRegLandmarks;
  GetUIToolLandmarksLists(toolCalLandmarks, toolRegLandmarks);
  m_ToolToBeEdited->SetToolControlPoints(toolCalLandmarks);
  m_ToolToBeEdited->SetToolLandmarks(toolRegLandmarks);

  //Advanced
  if (m_Controls->m_ToolTypeChooser->currentText() == "Instrument") m_ToolToBeEdited->SetType(mitk::NavigationTool::Instrument);
  else if (m_Controls->m_ToolTypeChooser->currentText() == "Fiducial") m_ToolToBeEdited->SetType(mitk::NavigationTool::Fiducial);
  else if (m_Controls->m_ToolTypeChooser->currentText() == "Skinmarker") m_ToolToBeEdited->SetType(mitk::NavigationTool::Skinmarker);
  else m_FinalTool->SetType(mitk::NavigationTool::Unknown);

  m_ToolToBeEdited->SetIdentifier(m_Controls->m_IdentifierEdit->text().toLatin1().data());
  m_ToolToBeEdited->SetSerialNumber(m_Controls->m_SerialNumberEdit->text().toLatin1().data());

  ////Tool Axis
  //mitk::Point3D toolAxis;
  //toolAxis.SetElement(0, (m_Controls->m_ToolAxisX->value()));
  //toolAxis.SetElement(1, (m_Controls->m_ToolAxisY->value()));
  //toolAxis.SetElement(2, (m_Controls->m_ToolAxisZ->value()));
  //m_ToolToBeEdited->SetToolAxis(toolAxis);
}

mitk::NavigationTool::Pointer QmitkNavigationToolCreationWidget::GetCreatedTool()
{
  return m_FinalTool;
}

void QmitkNavigationToolCreationWidget::OnFinished()
{
  if (m_Controls->m_Surface_Use_Other->isChecked())
    m_ToolToBeEdited->GetDataNode()->SetData(m_Controls->m_SurfaceChooser->GetSelectedNode()->GetData());

  //here we create a new tool
  m_FinalTool = m_ToolToBeEdited->Clone();
  //Set the correct name of data node, cause the m_ToolToBeEdited was called "Tool preview"
  m_FinalTool->GetDataNode()->SetName(m_Controls->m_ToolNameEdit->text().toStdString());

  emit NavigationToolFinished();
}

void QmitkNavigationToolCreationWidget::OnCancel()
{
  Initialize(nullptr, "");//Reset everything to a fresh tool, like it was done in the constructor
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
  m_ToolTransformationWidget->SetToolToEdit(m_ToolToBeEdited);
  m_ToolTransformationWidget->SetDefaultRotation(m_ToolToBeEdited->GetToolAxisOrientation());
  m_ToolTransformationWidget->SetDefaultOffset(m_ToolToBeEdited->GetToolTipPosition());

  m_ToolTransformationWidget->open();
}

void QmitkNavigationToolCreationWidget::OnEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip)
{
  //if user pressed cancle, nullptr is returned. Do nothing. Else, set values.
  if (toolTip)
  {
    m_ToolToBeEdited->SetToolTipPosition(mitk::Point3D(toolTip->GetOffset()));
    mitk::NavigationData::Pointer tempND = mitk::NavigationData::New(toolTip);//Convert to Navigation data for simple transversion to quaternion
    m_ToolToBeEdited->SetToolAxisOrientation(tempND->GetOrientation());

    //Update Label
    QString _label = "(" +
      QString::number(m_ToolToBeEdited->GetToolTipPosition()[0], 'f', 1) + ", " +
      QString::number(m_ToolToBeEdited->GetToolTipPosition()[1], 'f', 1) + ", " +
      QString::number(m_ToolToBeEdited->GetToolTipPosition()[2], 'f', 1) + "), quat: [" +
      QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[0], 'f', 2) + ", " +
      QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[1], 'f', 2) + ", " +
      QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[2], 'f', 2) + ", " +
      QString::number(m_ToolToBeEdited->GetToolAxisOrientation()[3], 'f', 2) + "]";
    m_Controls->m_ToolTipLabel->setText(_label);
  }
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
