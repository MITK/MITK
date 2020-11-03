/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatExperimentWidget.h"

#include <ctkXnatExperiment.h>

#include <QCompleter>

QmitkXnatExperimentWidget::QmitkXnatExperimentWidget(QWidget *parent) : QWidget(parent)
{
  m_Mode = INFO;
  Init();
}

QmitkXnatExperimentWidget::QmitkXnatExperimentWidget(Mode mode, QWidget *parent) : QWidget(parent), m_Mode(mode)
{
  Init();
}

QmitkXnatExperimentWidget::~QmitkXnatExperimentWidget()
{
}

void QmitkXnatExperimentWidget::Init()
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  if (m_Mode == Mode::INFO)
  {
    // make not needed row invisible
    m_Controls.labelLabel->setText("ID:");
    m_Controls.mandatoryLabel->setVisible(false);
  }
  if (m_Mode == Mode::CREATE)
  {
    // change modality to xsiType
    m_Controls.modalityLabel->setText("Type*:");

    // create list of all experiment session types
    QStringList xsiTypeList;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_CR_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_CT_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_DX_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_DX3DCRANIOFACIAL_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_ECG_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_EEG_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_EPS_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_ES_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_ESV_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_GM_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_GMV_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_HD_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_IO_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_MEG_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_MG_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_MR_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_OP_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_OPT_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_OTHER_DICOM_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_PET_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_RF_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_RT_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_SM_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_US_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_XA_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_XA3D_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_XC_SESSION;
    xsiTypeList << ctkXnatDefaultSchemaTypes::XSI_XCV_SESSION;

    // create completer for experiment sessions
    QCompleter *completer = new QCompleter(xsiTypeList, this);
    m_Controls.modalityLineEdit->setCompleter(completer);
    m_Controls.modalityLineEdit->setPlaceholderText("Start with \"xnat::\"");
  }
}

void QmitkXnatExperimentWidget::SetExperiment(ctkXnatExperiment *experiment)
{
  m_Experiment = experiment;

  // Set the UI labels
  if (m_Experiment->parent())
  {
    m_Controls.breadcrumbLabel->setText("Project:" + m_Experiment->parent()->parent()->property("name") +
                                        " > Subject:" + m_Experiment->parent()->property("label"));
  }
  m_Controls.labelLineEdit->setText(m_Experiment->property("label"));
  m_Controls.modalityLineEdit->setText(m_Experiment->property("modality"));
  m_Controls.dateLineEdit->setText(m_Experiment->property("date"));
  m_Controls.timeLineEdit->setText(m_Experiment->property("time"));
  m_Controls.scannerLineEdit->setText(m_Experiment->property("scanner"));
}

ctkXnatExperiment *QmitkXnatExperimentWidget::GetExperiment() const
{
  if (m_Mode == Mode::CREATE)
  {
    if (!m_Controls.labelLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("label", m_Controls.labelLineEdit->text());
    }
    if (!m_Controls.modalityLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("xsiType", m_Controls.modalityLineEdit->text());
    }
    if (!m_Controls.dateLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("date", m_Controls.dateLineEdit->text());
    }
    if (!m_Controls.timeLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("time", m_Controls.dateLineEdit->text());
    }
    if (!m_Controls.scannerLineEdit->text().isEmpty())
    {
      m_Experiment->setProperty("scanner", m_Controls.scannerLineEdit->text());
    }
  }

  return m_Experiment;
}
