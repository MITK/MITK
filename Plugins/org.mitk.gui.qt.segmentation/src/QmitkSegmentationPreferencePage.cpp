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

#include "QmitkSegmentationPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleSpinBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkLimitedLinearUndo.h>

QmitkSegmentationPreferencePage::QmitkSegmentationPreferencePage() :
  m_Initializing(false)
{
}

QmitkSegmentationPreferencePage::~QmitkSegmentationPreferencePage()
{

}

void QmitkSegmentationPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkSegmentationPreferencePage::CreateQtControl(QWidget* parent)
{
  m_MainControl = new QWidget(parent);
  m_Controls.setupUi(m_MainControl);

  m_Initializing = true;
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_SegmentationPreferencesNode = prefService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

  connect(m_Controls.m_VolumeRenderingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnVolumeRenderingCheckboxChecked(int)) );
  connect(m_Controls.m_SmoothingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnSmoothingCheckboxChecked(int)));

  m_Controls.m_SmoothingSpinBox->setMinimum(0.0);
  m_Controls.m_SmoothingSpinBox->setSingleStep(0.5);
  m_Controls.m_SmoothingSpinBox->setValue(1.0);
  m_Controls.m_SmoothingSpinBox->setToolTip("The Smoothing value is used as variance for a gaussian blur.");

  m_Controls.m_DecimationSpinBox->setMinimum(0.0);
  m_Controls.m_DecimationSpinBox->setMaximum(0.99);
  m_Controls.m_DecimationSpinBox->setSingleStep(0.1);
  m_Controls.m_DecimationSpinBox->setValue(0.5);
  m_Controls.m_DecimationSpinBox->setToolTip("Valid range is [0, 1). High values increase decimation, especially when very close to 1. A value of 0 disables decimation.");

  m_Controls.m_ClosingSpinBox->setMinimum(0.0);
  m_Controls.m_ClosingSpinBox->setMaximum(1.0);
  m_Controls.m_ClosingSpinBox->setSingleStep(0.1);
  m_Controls.m_ClosingSpinBox->setValue(0.0);
  m_Controls.m_ClosingSpinBox->setToolTip("Valid range is [0, 1]. Higher values increase closing. A value of 0 disables closing.");

  m_Controls.m_dequeMaxSize->setMinimum(mitk::MIN_DEQUE_SIZE);
  m_Controls.m_dequeMaxSize->setMaximum(mitk::MAX_DEQUE_SIZE);
  int value = m_SegmentationPreferencesNode->GetInt("queue size", mitk::DEF_DEQUE_SIZE);
  m_Controls.m_dequeMaxSize->setValue(value);
  mitk::LimitedLinearUndo::setDequeSize(value);

  m_Controls.m_SelectionModeCheckBox->setToolTip("If checked the segmentation plugin ensures that only one segmentation and the according greyvalue image are visible at one time.");

  this->Update();
  m_Initializing = false;
}

QWidget* QmitkSegmentationPreferencePage::GetQtControl() const
{
  return m_MainControl;
}
bool QmitkSegmentationPreferencePage::PerformOk()
{
  m_SegmentationPreferencesNode->PutBool("slim view", m_Controls.m_SlimViewCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("draw outline", m_Controls.m_RadioOutline->isChecked());
  m_SegmentationPreferencesNode->PutBool("volume rendering", m_Controls.m_VolumeRenderingCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("smoothing hint", m_Controls.m_SmoothingCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutDouble("smoothing value", m_Controls.m_SmoothingSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("decimation rate", m_Controls.m_DecimationSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("closing ratio", m_Controls.m_ClosingSpinBox->value());
  m_SegmentationPreferencesNode->PutBool("auto selection", m_Controls.m_SelectionModeCheckBox->isChecked());

  int value = m_Controls.m_dequeMaxSize->value();
  mitk::LimitedLinearUndo::setDequeSize(value);
  m_SegmentationPreferencesNode->PutInt("queue size", value);

  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{

}

void QmitkSegmentationPreferencePage::Update()
{
  m_Controls.m_SlimViewCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("slim view", false));

  if (m_SegmentationPreferencesNode->GetBool("draw outline", true) )
  {
    m_Controls.m_RadioOutline->setChecked( true );
  }
  else
  {
    m_Controls.m_RadioOverlay->setChecked( true );
  }

  m_Controls.m_VolumeRenderingCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("volume rendering", false));

  if (m_SegmentationPreferencesNode->GetBool("smoothing hint", true))
  {
    m_Controls.m_SmoothingCheckBox->setChecked(true);
    m_Controls.m_SmoothingSpinBox->setDisabled(true);
  }
  else
  {
    m_Controls.m_SmoothingCheckBox->setChecked(false);
    m_Controls.m_SmoothingSpinBox->setEnabled(true);
  }

  m_Controls.m_SelectionModeCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("auto selection", true));

  m_Controls.m_SmoothingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("smoothing value", 1.0));
  m_Controls.m_DecimationSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("decimation rate", 0.2));
  m_Controls.m_ClosingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("closing ratio", 0.0));
}

void QmitkSegmentationPreferencePage::OnVolumeRenderingCheckboxChecked(int state)
{
  if (m_Initializing) return;

  if ( state != Qt::Unchecked )
  {
    QMessageBox::information(nullptr,
                             tr("Memory warning"),
                             tr("Turning on volume rendering of segmentations will make the application more memory intensive (and potentially prone to crashes).\n\n"
                             "If you encounter out-of-memory problems, try turning off volume rendering again."));
  }
}

void QmitkSegmentationPreferencePage::OnSmoothingCheckboxChecked(int state)
{
  if (state != Qt::Unchecked)
  {
    m_Controls.m_SmoothingSpinBox->setDisabled(true);
  }
  else
  {
    m_Controls.m_SmoothingSpinBox->setEnabled(true);
  }
}
