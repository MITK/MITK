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

#include <mitkBaseRenderer.h>
#include <mitkBaseProperty.h>
#include <mitkDataNode.h>
#include <QDialog>
#include <ui_QmitkAddNewPropertyDialog.h>

class QmitkAddNewPropertyDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkAddNewPropertyDialog(mitk::BaseData::Pointer baseData, QWidget* parent = NULL);
  explicit QmitkAddNewPropertyDialog(mitk::DataNode::Pointer dataNode, mitk::BaseRenderer::Pointer renderer = NULL, QWidget* parent = NULL);
  ~QmitkAddNewPropertyDialog();

private slots:
  void AddNewProperty();
  void ShowAdequateValueWidget(const QString& type);
  bool ValidateValue();

private:
  mitk::BaseProperty::Pointer CreateProperty() const;
  void Initialize();

  Ui::QmitkAddNewPropertyDialog m_Controls;
  mitk::DataNode::Pointer m_DataNode;
  mitk::BaseData::Pointer m_BaseData;
  mitk::BaseRenderer::Pointer m_Renderer;
};

#endif
