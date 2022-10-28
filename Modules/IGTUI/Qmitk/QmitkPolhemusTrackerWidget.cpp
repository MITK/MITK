/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPolhemusTrackerWidget.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>
#include <QSettings>

#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include "vtkRenderer.h"
#include "vtkCamera.h"

const std::string QmitkPolhemusTrackerWidget::VIEW_ID = "org.mitk.views.PolhemusTrackerWidget";

QmitkPolhemusTrackerWidget::QmitkPolhemusTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}
void QmitkPolhemusTrackerWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);

  SetAdvancedSettingsEnabled(false);
  on_m_AdvancedSettings_clicked(); //hide advanced settings on setup
}

QmitkPolhemusTrackerWidget::~QmitkPolhemusTrackerWidget()
{
  delete m_Controls;
}

void QmitkPolhemusTrackerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkPolhemusTrackerWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkPolhemusTrackerWidget::OnToolStorageChanged()
{
    this->m_TrackingDevice = nullptr;
    MITK_DEBUG<<"Resetting Polhemus Tracking Device, because tool storage changed.";
}

void QmitkPolhemusTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_hemisphereTracking), SIGNAL(clicked()), this, SLOT(on_m_hemisphereTracking_clicked()));
    connect((QObject*)(m_Controls->m_ToggleHemisphere), SIGNAL(clicked()), this, SLOT(on_m_ToggleHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_SetHemisphere), SIGNAL(clicked()), this, SLOT(on_m_SetHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_GetHemisphere), SIGNAL(clicked()), this, SLOT(on_m_GetHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_AdjustHemisphere), SIGNAL(clicked()), this, SLOT(on_m_AdjustHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_AdvancedSettings), SIGNAL(clicked()), this, SLOT(on_m_AdvancedSettings_clicked()));
    connect((QObject*)(m_Controls->m_ToggleToolTipCalibration), SIGNAL(clicked()), this, SLOT(on_m_ToggleToolTipCalibration_clicked()));
  }
}

mitk::TrackingDevice::Pointer QmitkPolhemusTrackerWidget::GetTrackingDevice()
{
  if (m_TrackingDevice.IsNull())
  {
    m_TrackingDevice = mitk::PolhemusTrackingDevice::New();
    m_TrackingDevice->SetHemisphereTrackingEnabled(m_Controls->m_hemisphereTracking->isChecked());
  }

  return static_cast<mitk::TrackingDevice::Pointer>(m_TrackingDevice);
}

QmitkPolhemusTrackerWidget* QmitkPolhemusTrackerWidget::Clone(QWidget* parent) const
{
  QmitkPolhemusTrackerWidget* clonedWidget = new QmitkPolhemusTrackerWidget(parent);
  clonedWidget->Initialize();
  return clonedWidget;
}

void QmitkPolhemusTrackerWidget::on_m_hemisphereTracking_clicked()
{
  m_TrackingDevice->SetHemisphereTrackingEnabled(m_Controls->m_hemisphereTracking->isChecked());
}

void QmitkPolhemusTrackerWidget::on_m_ToggleHemisphere_clicked()
{
  // Index 0 == All Tools == -1 for Polhemus interface; Index 2 == Tool 2 == 1 for Polhemus; etc...
  m_TrackingDevice->ToggleHemisphere(GetSelectedToolIndex());

  MITK_INFO << "Toggle Hemisphere for tool " << m_Controls->m_ToolSelection->currentText().toStdString();
}

void QmitkPolhemusTrackerWidget::on_m_SetHemisphere_clicked()
{
  mitk::Vector3D _hemisphere;
  mitk::FillVector3D(_hemisphere, m_Controls->m_Hemisphere_X->value(), m_Controls->m_Hemisphere_Y->value(), m_Controls->m_Hemisphere_Z->value());
  m_TrackingDevice->SetHemisphere(GetSelectedToolIndex(), _hemisphere);

  //If you set a hemisphere vector which is unequal (0|0|0), this means, that there is no hemisphere tracking any more
  //disable the checkbox in case it was on before, so that it can be reactivated...
  if (_hemisphere.GetNorm() != 0)
    m_Controls->m_hemisphereTracking->setChecked(false);

  MITK_INFO << "Hemisphere set for tool " << m_Controls->m_ToolSelection->currentText().toStdString();
}

void QmitkPolhemusTrackerWidget::on_m_GetHemisphere_clicked()
{
  mitk::Vector3D _hemisphere = m_TrackingDevice->GetHemisphere(GetSelectedToolIndex());
  m_Controls->m_Hemisphere_X->setValue(_hemisphere[0]);
  m_Controls->m_Hemisphere_Y->setValue(_hemisphere[1]);
  m_Controls->m_Hemisphere_Z->setValue(_hemisphere[2]);

  QString label;

  if (m_TrackingDevice->GetHemisphereTrackingEnabled(GetSelectedToolIndex()))
  {
    label = "HemisphereTracking is ON for tool ";
    label.append(m_Controls->m_ToolSelection->currentText());
  }
  else if (GetSelectedToolIndex() == -1)
  {
    label = "HemisphereTracking is OFF for at least one tool.";
  }
  else
  {
    label = "HemisphereTracking is OFF for tool ";
    label.append(m_Controls->m_ToolSelection->currentText());
  }

  m_Controls->m_StatusLabelHemisphereTracking->setText(label);

  MITK_INFO << "Updated SpinBox for Hemisphere of tool " << m_Controls->m_ToolSelection->currentText().toStdString();
}

void QmitkPolhemusTrackerWidget::on_m_AdjustHemisphere_clicked()
{
  int _tool = GetSelectedToolIndex();
  QMessageBox msgBox;
  QString _text;
  if (_tool == -1)
  {
    _text.append("Adjusting hemisphere for all tools.");
    msgBox.setText(_text);
    _text.clear();
    _text = tr("Please make sure, that the entire tools (including tool tip AND sensor) are placed in the positive x hemisphere. Press 'Adjust hemisphere' if you are ready.");
    msgBox.setInformativeText(_text);
  }
  else
  {
    _text.append("Adjusting hemisphere for tool '");
    _text.append(m_Controls->m_ToolSelection->currentText());
    _text.append(tr("' at port %2.").arg(_tool));
    msgBox.setText(_text);
    _text.clear();
    _text = tr("Please make sure, that the entire tool (including tool tip AND sensor) is placed in the positive x hemisphere. Press 'Adjust hemisphere' if you are ready.");
    msgBox.setInformativeText(_text);
  }

  QPushButton *adjustButton = msgBox.addButton(tr("Adjust hemisphere"), QMessageBox::ActionRole);
  QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
  msgBox.exec();
  if (msgBox.clickedButton() == adjustButton) {
    // adjust
    m_TrackingDevice->AdjustHemisphere(_tool);
    MITK_INFO << "Adjusting Hemisphere for tool " << m_Controls->m_ToolSelection->currentText().toStdString();
  }
  else if (msgBox.clickedButton() == cancelButton) {
    // abort
    MITK_INFO << "Cancel 'Adjust hemisphere'. No harm done...";
  }
}

void QmitkPolhemusTrackerWidget::on_m_ToggleToolTipCalibration_clicked()
{
  if (m_Controls->m_ToolSelection->currentIndex() != 0)
  {
    mitk::PolhemusTool* _tool = dynamic_cast<mitk::PolhemusTool*> (this->m_TrackingDevice->GetToolByName(m_Controls->m_ToolSelection->currentText().toStdString()));
    mitk::Point3D tip = _tool->GetToolTipPosition().GetVectorFromOrigin()*(-1.);
    mitk::Quaternion quat = _tool->GetToolAxisOrientation().inverse();
    _tool->SetToolTipPosition(tip, quat);
  }
  else
  {
    for (int i = 0; i < m_TrackingDevice->GetToolCount(); ++i)
    {
      mitk::PolhemusTool* _tool = dynamic_cast<mitk::PolhemusTool*> (this->m_TrackingDevice->GetTool(i));
      mitk::Point3D tip = _tool->GetToolTipPosition().GetVectorFromOrigin()*(-1.);
      mitk::Quaternion quat = _tool->GetToolAxisOrientation().inverse();
      _tool->SetToolTipPosition(tip, quat);
    }
  }
}

void QmitkPolhemusTrackerWidget::OnConnected(bool _success)
{
  if (!_success)
  {
    this->m_TrackingDevice = nullptr;
    return;
  }

  SetAdvancedSettingsEnabled(true);

  if (m_TrackingDevice->GetToolCount() != m_Controls->m_ToolSelection->count())
  {
    m_Controls->m_ToolSelection->clear();

    m_Controls->m_ToolSelection->addItem("All Tools");

    for (int i = 0; i < m_TrackingDevice->GetToolCount(); ++i)
    {
      m_Controls->m_ToolSelection->addItem(m_TrackingDevice->GetTool(i)->GetToolName());
    }
  }
}

void QmitkPolhemusTrackerWidget::OnStartTracking(bool _success)
{
  if (!_success)
    return;

  auto allRenderWindows = mitk::BaseRenderer::GetAll3DRenderWindows();
  for (auto mapit = allRenderWindows.begin(); mapit != allRenderWindows.end(); ++mapit)
  {
    // rotate 3D render windows, so that the view matches the sensor. Positive x == right, y == front, z == down;
    mapit->second->GetCameraController()->SetViewToPosterior();
    mapit->second->GetVtkRenderer()->GetActiveCamera()->SetViewUp(0, 0, -1);
  }
}

void QmitkPolhemusTrackerWidget::OnDisconnected(bool _success)
{
  if (!_success)
    return;
  SetAdvancedSettingsEnabled(false);
}

void QmitkPolhemusTrackerWidget::SetAdvancedSettingsEnabled(bool _enable)
{
  m_Controls->m_ToolSelection->setEnabled(_enable);
  m_Controls->label_toolsToChange->setEnabled(_enable);
  m_Controls->label_UpdateOnRequest->setEnabled(_enable);
  m_Controls->m_GetHemisphere->setEnabled(_enable);
  m_Controls->m_Hemisphere_X->setEnabled(_enable);
  m_Controls->m_Hemisphere_Y->setEnabled(_enable);
  m_Controls->m_Hemisphere_Z->setEnabled(_enable);
  m_Controls->m_SetHemisphere->setEnabled(_enable);
  m_Controls->m_ToggleHemisphere->setEnabled(_enable);
  m_Controls->m_AdjustHemisphere->setEnabled(_enable);
  m_Controls->m_ToggleToolTipCalibration->setEnabled(_enable);
}

void QmitkPolhemusTrackerWidget::on_m_AdvancedSettings_clicked()
{
  bool _enable = m_Controls->m_AdvancedSettings->isChecked();
  m_Controls->m_ToolSelection->setVisible(_enable);
  m_Controls->label_toolsToChange->setVisible(_enable);
  m_Controls->label_UpdateOnRequest->setVisible(_enable);
  m_Controls->m_GetHemisphere->setVisible(_enable);
  m_Controls->m_Hemisphere_X->setVisible(_enable);
  m_Controls->m_Hemisphere_Y->setVisible(_enable);
  m_Controls->m_Hemisphere_Z->setVisible(_enable);
  m_Controls->m_SetHemisphere->setVisible(_enable);
  m_Controls->m_ToggleHemisphere->setVisible(_enable);
  m_Controls->m_AdjustHemisphere->setVisible(_enable);
  m_Controls->m_ToggleToolTipCalibration->setVisible(_enable);
  m_Controls->m_StatusLabelHemisphereTracking->setVisible(_enable);
}

int QmitkPolhemusTrackerWidget::GetSelectedToolIndex()
{
  // Index 0 == All Tools == -1 for Polhemus interface; Index 1 == Tool 1 == 1 for Polhemus Interface; etc...
  int _index = m_Controls->m_ToolSelection->currentIndex() - 1;
  if (_index != -1)
  {
    //we need to find the internal Polhemus index for this tool. This is stored in the identifier of a navigation tool or as Port in PolhemusTool.
    mitk::PolhemusTool* _tool = dynamic_cast<mitk::PolhemusTool*>(m_TrackingDevice->GetToolByName(m_Controls->m_ToolSelection->currentText().toStdString()));
    _index = _tool->GetToolPort();
  }
  return _index;
}
