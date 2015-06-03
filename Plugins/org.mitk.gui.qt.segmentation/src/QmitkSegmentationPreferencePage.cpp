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

QmitkSegmentationPreferencePage::QmitkSegmentationPreferencePage()
: m_MainControl(nullptr)
, m_Initializing(false)
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
  m_Initializing = true;
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_SegmentationPreferencesNode = prefService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

  m_MainControl = new QWidget(parent);

  auto  formLayout = new QFormLayout;
  formLayout->setHorizontalSpacing(8);
  formLayout->setVerticalSpacing(24);

  m_SlimViewCheckBox = new QCheckBox("Hide tool button texts and increase icon size", m_MainControl);
  formLayout->addRow("Slim view", m_SlimViewCheckBox);

  auto   displayOptionsLayout = new QVBoxLayout;
  m_RadioOutline = new QRadioButton( "Draw as outline", m_MainControl);
  displayOptionsLayout->addWidget( m_RadioOutline );
  m_RadioOverlay = new QRadioButton( "Draw as transparent overlay", m_MainControl);
  displayOptionsLayout->addWidget( m_RadioOverlay );
  formLayout->addRow( "2D display", displayOptionsLayout );

  m_VolumeRenderingCheckBox = new QCheckBox( "Show as volume rendering", m_MainControl );
  formLayout->addRow( "3D display", m_VolumeRenderingCheckBox );
  connect( m_VolumeRenderingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnVolumeRenderingCheckboxChecked(int)) );

  auto   surfaceLayout = new QFormLayout;
  surfaceLayout->setSpacing(8);

  m_SmoothingCheckBox = new QCheckBox("Use image spacing as smoothing value hint", m_MainControl);
  surfaceLayout->addRow(m_SmoothingCheckBox);
  connect(m_SmoothingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnSmoothingCheckboxChecked(int)));

  m_SmoothingSpinBox = new QDoubleSpinBox(m_MainControl);
  m_SmoothingSpinBox->setMinimum(0.0);
  m_SmoothingSpinBox->setSingleStep(0.5);
  m_SmoothingSpinBox->setValue(1.0);
  m_SmoothingSpinBox->setToolTip("The Smoothing value is used as variance for a gaussian blur.");
  surfaceLayout->addRow("Smoothing value (mm)", m_SmoothingSpinBox);

  m_DecimationSpinBox = new QDoubleSpinBox(m_MainControl);
  m_DecimationSpinBox->setMinimum(0.0);
  m_DecimationSpinBox->setMaximum(0.99);
  m_DecimationSpinBox->setSingleStep(0.1);
  m_DecimationSpinBox->setValue(0.5);
  m_DecimationSpinBox->setToolTip("Valid range is [0, 1). High values increase decimation, especially when very close to 1. A value of 0 disables decimation.");
  surfaceLayout->addRow("Decimation rate", m_DecimationSpinBox);

  m_ClosingSpinBox = new QDoubleSpinBox(m_MainControl);
  m_ClosingSpinBox->setMinimum(0.0);
  m_ClosingSpinBox->setMaximum(1.0);
  m_ClosingSpinBox->setSingleStep(0.1);
  m_ClosingSpinBox->setValue(0.0);
  m_ClosingSpinBox->setToolTip("Valid range is [0, 1]. Higher values increase closing. A value of 0 disables closing.");
  surfaceLayout->addRow("Closing Ratio", m_ClosingSpinBox);

  m_dequeMaxSize = new QSpinBox(m_MainControl);
  m_dequeMaxSize->setMinimum(mitk::MIN_DEQUE_SIZE);
  m_dequeMaxSize->setMaximum(mitk::MAX_DEQUE_SIZE);
  m_dequeMaxSize->setValue(m_SegmentationPreferencesNode->GetInt("queue size", mitk::MAX_DEQUE_SIZE));
  surfaceLayout->addRow("Max queue size", m_dequeMaxSize);

  m_SelectionModeCheckBox = new QCheckBox("Enable auto-selection mode", m_MainControl);
  m_SelectionModeCheckBox->setToolTip("If checked the segmentation plugin ensures that only one segmentation and the according greyvalue image are visible at one time.");
  formLayout->addRow("Data node selection mode",m_SelectionModeCheckBox);

  formLayout->addRow("Smoothed surface creation", surfaceLayout);

  m_MainControl->setLayout(formLayout);
  this->Update();
  m_Initializing = false;
}

QWidget* QmitkSegmentationPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkSegmentationPreferencePage::PerformOk()
{
  m_SegmentationPreferencesNode->PutBool("slim view", m_SlimViewCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("draw outline", m_RadioOutline->isChecked());
  m_SegmentationPreferencesNode->PutBool("volume rendering", m_VolumeRenderingCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutBool("smoothing hint", m_SmoothingCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutDouble("smoothing value", m_SmoothingSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("decimation rate", m_DecimationSpinBox->value());
  m_SegmentationPreferencesNode->PutDouble("closing ratio", m_ClosingSpinBox->value());
  m_SegmentationPreferencesNode->PutBool("auto selection", m_SelectionModeCheckBox->isChecked());
  m_SegmentationPreferencesNode->PutInt("queue size", m_dequeMaxSize->value());
  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{

}

void QmitkSegmentationPreferencePage::Update()
{
  //m_EnableSingleEditing->setChecked(m_SegmentationPreferencesNode->GetBool("Single click property editing", true));

  m_SlimViewCheckBox->setChecked(m_SegmentationPreferencesNode->GetBool("slim view", false));

  if (m_SegmentationPreferencesNode->GetBool("draw outline", true) )
  {
    m_RadioOutline->setChecked( true );
  }
  else
  {
    m_RadioOverlay->setChecked( true );
  }

  m_VolumeRenderingCheckBox->setChecked( m_SegmentationPreferencesNode->GetBool("volume rendering", false) );

  if (m_SegmentationPreferencesNode->GetBool("smoothing hint", true))
  {
    m_SmoothingCheckBox->setChecked(true);
    m_SmoothingSpinBox->setDisabled(true);
  }
  else
  {
      m_SmoothingCheckBox->setChecked(false);
      m_SmoothingSpinBox->setEnabled(true);
  }

  m_SelectionModeCheckBox->setChecked( m_SegmentationPreferencesNode->GetBool("auto selection", true) );

  m_SmoothingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("smoothing value", 1.0));
  m_DecimationSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("decimation rate", 0.5));
  m_ClosingSpinBox->setValue(m_SegmentationPreferencesNode->GetDouble("closing ratio", 0.0));
}

void QmitkSegmentationPreferencePage::OnVolumeRenderingCheckboxChecked(int state)
{
  if (m_Initializing) return;

  if ( state != Qt::Unchecked )
  {
    QMessageBox::information(nullptr,
                             "Memory warning",
                             "Turning on volume rendering of segmentations will make the application more memory intensive (and potentially prone to crashes).\n\n"
                             "If you encounter out-of-memory problems, try turning off volume rendering again.");
  }
}

void QmitkSegmentationPreferencePage::OnSmoothingCheckboxChecked(int state)
{
  if (state != Qt::Unchecked)
    m_SmoothingSpinBox->setDisabled(true);
  else
    m_SmoothingSpinBox->setEnabled(true);
}
