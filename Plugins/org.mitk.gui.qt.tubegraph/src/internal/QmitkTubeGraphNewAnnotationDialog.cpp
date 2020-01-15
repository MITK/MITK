/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTubeGraphNewAnnotationDialog.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlistwidget.h>


QmitkTubeGraphNewAnnotationDialog::QmitkTubeGraphNewAnnotationDialog(QWidget* parent)
:QDialog(parent)
{
  QDialog::setFixedSize(200, 200);

  layout = new QVBoxLayout(this);
  layout->setMargin(5);
  layout->setSpacing(5);

  annotationNameLabel = new QLabel("Enter the name of the annotation!", this);
  layout->addWidget(annotationNameLabel);

  annotationNameLineEdit = new QLineEdit(this);
  layout->addWidget(annotationNameLineEdit);

  annotationDescriptionLabel = new QLabel("Enter a description!", this);
  layout->addWidget(annotationDescriptionLabel);

  annotationDescriptionLineEdit = new QLineEdit(this);
  layout->addWidget(annotationDescriptionLineEdit);

  buttonLayout = new QHBoxLayout();

  okButton = new QPushButton("Ok", this);
  buttonLayout->addWidget(okButton, 0, Qt::AlignRight);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnAddingAnnotation()) );

  cancleButton = new QPushButton("Cancle", this);
  buttonLayout->addWidget(cancleButton,  0, Qt::AlignRight);
  connect( cancleButton, SIGNAL(clicked()), this, SLOT(reject()) );

  layout->addLayout(buttonLayout);

  annotationNameLineEdit->setFocus();
}

void QmitkTubeGraphNewAnnotationDialog::OnAddingAnnotation()
{
  if (annotationNameLineEdit->text().isEmpty())
  {
    annotationNameLineEdit->setStyleSheet("border: 1px solid red");
    return;
  }

  m_NewAnnotationName = annotationNameLineEdit->text();
  m_NewAnnotationDescription = annotationDescriptionLineEdit->text();

  this->accept();
}

QmitkTubeGraphNewAnnotationDialog::~QmitkTubeGraphNewAnnotationDialog()
{
  delete layout;

  delete okButton;
  delete cancleButton;
   // delete buttonLayout;

  delete annotationNameLabel;
  delete annotationNameLineEdit;

  delete annotationDescriptionLabel;
  delete annotationDescriptionLineEdit;
}

QString QmitkTubeGraphNewAnnotationDialog::GetAnnotationName()
{
  return m_NewAnnotationName;
}

QString QmitkTubeGraphNewAnnotationDialog::GetAnnotationDescription()
{
  return m_NewAnnotationDescription;
}
