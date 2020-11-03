/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkXnatCreateObjectDialog.h"

// XNAT
#include <QmitkXnatExperimentWidget.h>
#include <QmitkXnatProjectWidget.h>
#include <QmitkXnatSubjectWidget.h>

// CTK XNAT Core
//#include <ctkXnatProject.h>
#include <ctkXnatExperiment.h>
#include <ctkXnatSubject.h>

// Qt
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

// Testing
#include <iostream>
#include <cstdio>

QmitkXnatCreateObjectDialog::QmitkXnatCreateObjectDialog(SpecificType type, QWidget *parent)
  : QDialog(parent), m_Type(type)
{
  switch (m_Type)
  {
    // case PROJECT:
    //  m_Object = new ctkXnatProject();
    //  m_Widget = new QmitkXnatProjectWidget(dynamic_cast<ctkXnatProject*>(m_Object));
    //  break;
    case SUBJECT:
      m_Object = new ctkXnatSubject();
      m_Widget = new QmitkXnatSubjectWidget(QmitkXnatSubjectWidget::Mode::CREATE, parent);
      dynamic_cast<QmitkXnatSubjectWidget *>(m_Widget)->SetSubject(dynamic_cast<ctkXnatSubject *>(m_Object));
      break;
    case EXPERIMENT:
      m_Object = new ctkXnatExperiment();
      m_Widget = new QmitkXnatExperimentWidget(QmitkXnatExperimentWidget::Mode::CREATE, parent);
      dynamic_cast<QmitkXnatExperimentWidget *>(m_Widget)->SetExperiment(dynamic_cast<ctkXnatExperiment *>(m_Object));
      break;
    default:
      break;
  }

  QList<QLineEdit *> children = m_Widget->findChildren<QLineEdit *>();
  foreach (QLineEdit *child, children)
  {
    child->setReadOnly(false);
  }

  QGridLayout *grid = dynamic_cast<QGridLayout *>(m_Widget->layout());

  QPushButton *btnOk = new QPushButton("Create");
  QPushButton *btnCancel = new QPushButton("Cancel");

  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addWidget(btnOk);
  hLayout->addWidget(btnCancel);

  connect(btnOk, SIGNAL(clicked()), this, SLOT(OnAcceptClicked()));
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(OnCancelClicked()));

  grid->addLayout(hLayout, grid->rowCount() + 1, 1);

  // Fill the dialog
  setLayout(grid);
}

QmitkXnatCreateObjectDialog::~QmitkXnatCreateObjectDialog()
{
  delete m_Widget;
}

ctkXnatObject *QmitkXnatCreateObjectDialog::GetXnatObject()
{
  return m_Object;
}

void QmitkXnatCreateObjectDialog::OnAcceptClicked()
{
  // ctkXnatProject* project;
  ctkXnatSubject *subject;
  ctkXnatExperiment *experiment;

  switch (m_Type)
  {
    // case PROJECT:
    //  project = dynamic_cast<QmitkXnatProjectWidget*>(m_Widget)->GetProject();

    //  // Check mandatory field
    //  if (project->property("label").isEmpty())
    //  {
    //    QMessageBox msgBox;
    //    msgBox.setIcon(QMessageBox::Warning);
    //    msgBox.setText("Please fill in an ID to create a new project!");
    //    msgBox.exec();
    //    return;
    //  }

    //  m_Object = project;
    //  break;

    case SUBJECT:
      subject = dynamic_cast<QmitkXnatSubjectWidget *>(m_Widget)->GetSubject();

      // Check mandatory field
      if (subject->property("label").isEmpty())
      {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Please fill in an ID to create a new subject!");
        msgBox.exec();
        return;
      }

      m_Object = subject;
      break;

    case EXPERIMENT:
      experiment = dynamic_cast<QmitkXnatExperimentWidget *>(m_Widget)->GetExperiment();

      // Check mandatory field
      if (experiment->property("label").isEmpty())
      {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Please fill in an ID to create a new experiment!");
        msgBox.exec();
        return;
      }
      else if (experiment->property("xsiType").isEmpty() || experiment->property("xsiType") == "xnat:experimentData")
      {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Please fill in an XSI Type to create a new experiment!");
        msgBox.exec();
        return;
      }

      m_Object = experiment;
      break;

    default:
      break;
  }
  accept();
}

void QmitkXnatCreateObjectDialog::OnCancelClicked()
{
  reject();
}
