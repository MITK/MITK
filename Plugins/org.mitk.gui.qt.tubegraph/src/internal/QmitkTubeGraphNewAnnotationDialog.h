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

#ifndef _QMITKTUBEGRAPHNEWANNOTATIONDIALOG_H_INCLUDED
#define _QMITKTUBEGRAPHNEWANNOTATIONDIALOG_H_INCLUDED

#include <qdialog.h>

class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QLabel;

class QmitkTubeGraphNewAnnotationDialog : public QDialog
{
  Q_OBJECT

public:

  QmitkTubeGraphNewAnnotationDialog(QWidget* parent = nullptr);
  virtual ~QmitkTubeGraphNewAnnotationDialog();

  QString GetAnnotationName();
  QString GetAnnotationDescription();

protected slots:

void OnAddingAnnotation();

protected:

   QVBoxLayout* layout;
   QHBoxLayout* buttonLayout;

   QPushButton* okButton;
   QPushButton* cancleButton;

   QLabel* annotationNameLabel;
   QLineEdit* annotationNameLineEdit;

   QLabel* annotationDescriptionLabel;
   QLineEdit* annotationDescriptionLineEdit;

   QString m_NewAnnotationName;
   QString m_NewAnnotationDescription;
};

#endif
