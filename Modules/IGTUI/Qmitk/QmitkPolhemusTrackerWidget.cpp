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

#include "QmitkPolhemusTrackerWidget.h"

#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>
#include <QSettings>

#include <QmitkToolSelectionWidget.h>

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

  SetAdvancedSettingsVisible(false);
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

void QmitkPolhemusTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_hemisphereTracking), SIGNAL(clicked()), this, SLOT(on_m_hemisphereTracking_clicked()));
    connect((QObject*)(m_Controls->m_ToggleHemisphere), SIGNAL(clicked()), this, SLOT(on_m_ToggleHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_SetHemisphere), SIGNAL(clicked()), this, SLOT(on_m_SetHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_GetHemisphere), SIGNAL(clicked()), this, SLOT(on_m_GetHemisphere_clicked()));
    connect((QObject*)(m_Controls->m_AdjustHemisphere), SIGNAL(clicked()), this, SLOT(on_m_AdjustHemisphere_clicked()));
  }
}

mitk::TrackingDevice::Pointer QmitkPolhemusTrackerWidget::ConstructTrackingDevice()
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

  //If you set a hemisphere vector which is unequal (0|0|0), this means, that there is no hemisphere tracing any more
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
    mitk::Vector3D _hemisphere;
    mitk::FillVector3D(_hemisphere, 1, 0, 0);

    //Was HemiTracking on before?
    bool _hemiTrack = m_TrackingDevice->GetHemisphereTrackingEnabled(_tool);

    //Set Hemisphere to (1|0|0) where user placed the tool
    m_TrackingDevice->SetHemisphere(_tool, _hemisphere);

    //If HemiTrack was on, switch it on again by setting (0|0|0). Don't use general fuction SetHemisphereTrackingEnabled, as we might only adapt a single tool.
    if (_hemiTrack)
    {
      mitk::FillVector3D(_hemisphere, 0, 0, 0);
      m_TrackingDevice->SetHemisphere(_tool, _hemisphere);
    }
    MITK_INFO << "Adjusting Hemisphere for tool " << m_Controls->m_ToolSelection->currentText().toStdString();
  }
  else if (msgBox.clickedButton() == cancelButton) {
    // abort
    MITK_INFO << "Cancel 'Adjust hemisphere'. No harm done...";
  }
}

void QmitkPolhemusTrackerWidget::OnConnected()
{
  SetAdvancedSettingsVisible(true);

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

void QmitkPolhemusTrackerWidget::OnDisconnected()
{
  SetAdvancedSettingsVisible(false);
}

void QmitkPolhemusTrackerWidget::SetAdvancedSettingsVisible(bool _enable)
{
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