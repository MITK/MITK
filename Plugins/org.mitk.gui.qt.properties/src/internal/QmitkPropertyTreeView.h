/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPropertyTreeView_h
#define QmitkPropertyTreeView_h

#include <ui_QmitkPropertyTreeView.h>

// mitk core module
#include <mitkCoreServices.h>
#include <mitkDataNode.h>

// mitk gui common plugin
#include <mitkIRenderWindowPartListener.h>

// mitk gui qt common plugin
#include <QmitkAbstractView.h>
#include "QmitkSelectionServiceConnector.h"

class QmitkPropertyItemDelegate;
class QmitkPropertyItemModel;
class QmitkPropertyItemSortFilterProxyModel;

namespace mitk
{
  class IPropertyAliases;
  class IPropertyDescriptions;
  class IPropertyPersistence;
}

class QmitkPropertyTreeView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  berryObjectMacro(QmitkPropertyTreeView);

  QmitkPropertyTreeView();
  ~QmitkPropertyTreeView() override;

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart*) override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

private:

  void SetAsSelectionListener(bool checked);

  QString GetPropertyNameOrAlias(const QModelIndex& index);

  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void HideAllIcons();

private Q_SLOTS:

  void OnCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnPropertyListChanged(int index);
  void OnAddNewProperty();
  void OnFilterTextChanged(const QString& filter);
  void OnModelReset();

private:

  std::string m_SelectionClassName;
  mitk::CoreServicePointer<mitk::IPropertyAliases> m_PropertyAliases;
  mitk::CoreServicePointer<mitk::IPropertyDescriptions> m_PropertyDescriptions;
  mitk::CoreServicePointer<mitk::IPropertyPersistence> m_PropertyPersistence;
  Ui::QmitkPropertyTreeView m_Controls;
  QmitkPropertyItemSortFilterProxyModel* m_ProxyModel;
  QmitkPropertyItemModel* m_Model;
  QmitkPropertyItemDelegate* m_Delegate;
  mitk::DataNode::Pointer m_SelectedNode;
  mitk::BaseRenderer* m_Renderer;

  std::unique_ptr<QmitkSelectionServiceConnector> m_SelectionServiceConnector;
};

#endif
