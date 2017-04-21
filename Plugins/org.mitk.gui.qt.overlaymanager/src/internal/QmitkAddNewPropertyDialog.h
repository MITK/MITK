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

#ifndef QmitkAddNewPropertyDialog_h
#define QmitkAddNewPropertyDialog_h

#include <QDialog>
#include <mitkAnnotation.h>
#include <mitkBaseProperty.h>
#include <mitkBaseRenderer.h>
#include <ui_QmitkAddNewPropertyDialog.h>

class QmitkAddNewPropertyDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkAddNewPropertyDialog(mitk::Annotation::Pointer overlay,
                                     mitk::BaseRenderer::Pointer renderer = NULL,
                                     QWidget *parent = NULL);
  ~QmitkAddNewPropertyDialog();

private slots:
  void AddNewProperty();
  void ShowAdequateValueWidget(const QString &type);
  bool ValidateValue();

private:
  mitk::BaseProperty::Pointer CreateProperty() const;
  void Initialize();

  Ui::QmitkAddNewPropertyDialog m_Controls;
  mitk::Annotation::Pointer m_Overlay;
  mitk::BaseRenderer::Pointer m_Renderer;
};

#endif
