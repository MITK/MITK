/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTubeGraphNewAnnotationDialog_h
#define QmitkTubeGraphNewAnnotationDialog_h

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
  ~QmitkTubeGraphNewAnnotationDialog() override;

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
