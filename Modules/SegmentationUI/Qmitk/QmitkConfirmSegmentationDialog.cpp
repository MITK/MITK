/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkConfirmSegmentationDialog.h"
#include "ui_QmitkConfirmSegmentationDialog.h"

QmitkConfirmSegmentationDialog::QmitkConfirmSegmentationDialog(QWidget *parent)
  : QDialog(parent), m_Controls(new Ui::QmitkConfirmSegmentationDialog)
{
  m_Controls->setupUi(this);

  connect(m_Controls->m_btnOverwriteSegmentation, SIGNAL(clicked()), this, SLOT(OnOverwriteExistingSegmentation()));
  connect(m_Controls->m_btnNewSegmentation, SIGNAL(clicked()), this, SLOT(OnCreateNewSegmentation()));
  connect(m_Controls->m_btnCancel, SIGNAL(clicked()), this, SLOT(OnCancelSegmentation()));
}

QmitkConfirmSegmentationDialog::~QmitkConfirmSegmentationDialog()
{
  delete m_Controls;
}

void QmitkConfirmSegmentationDialog::OnOverwriteExistingSegmentation()
{
  this->done(OVERWRITE_SEGMENTATION);
}

void QmitkConfirmSegmentationDialog::OnCreateNewSegmentation()
{
  this->done(CREATE_NEW_SEGMENTATION);
}

void QmitkConfirmSegmentationDialog::OnCancelSegmentation()
{
  this->done(CANCEL_SEGMENTATION);
}

void QmitkConfirmSegmentationDialog::SetSegmentationName(QString name)
{
  QString text("Do you really want to overwrite " + name + "?");
  m_Controls->m_lblDialogText->setText(text);
}
