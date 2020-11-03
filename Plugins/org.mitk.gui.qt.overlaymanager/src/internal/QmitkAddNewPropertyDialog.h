/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
                                     mitk::BaseRenderer::Pointer renderer = nullptr,
                                     QWidget *parent = nullptr);
  ~QmitkAddNewPropertyDialog() override;

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
