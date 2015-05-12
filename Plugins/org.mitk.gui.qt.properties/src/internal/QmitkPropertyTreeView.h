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

#ifndef QmitkPropertyTreeView_h
#define QmitkPropertyTreeView_h

#include <mitkDataNode.h>
#include <mitkIRenderWindowPartListener.h>
#include <QmitkAbstractView.h>
#include <ui_QmitkPropertyTreeView.h>

class QmitkPropertyItemDelegate;
class QmitkPropertyItemModel;
class QmitkPropertyItemSortFilterProxyModel;

namespace mitk
{
  class IPropertyAliases;
  class IPropertyDescriptions;
}

class QmitkPropertyTreeView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  berryObjectMacro(QmitkPropertyTreeView);

  QmitkPropertyTreeView();
  ~QmitkPropertyTreeView();

  void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart*) override;

protected:
  void CreateQtPartControl(QWidget* parent) override;

private:
  QString GetPropertyNameOrAlias(const QModelIndex& index);
  void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;
  void OnPropertyNameChanged(const itk::EventObject& event);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

private slots:
  void OnCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnPropertyListChanged(int index);
  void OnAddNewProperty();
  void OnFilterTextChanged(const QString& filter);
  void OnModelReset();

private:
  QWidget* m_Parent;
  unsigned long m_PropertyNameChangedTag;
  std::string m_SelectionClassName;
  mitk::IPropertyAliases* m_PropertyAliases;
  mitk::IPropertyDescriptions* m_PropertyDescriptions;
  bool m_ShowAliasesInDescription;
  bool m_DeveloperMode;
  Ui::QmitkPropertyTreeView m_Controls;
  QmitkPropertyItemSortFilterProxyModel* m_ProxyModel;
  QmitkPropertyItemModel* m_Model;
  QmitkPropertyItemDelegate* m_Delegate;
  mitk::DataNode::Pointer m_SelectedNode;
  mitk::BaseRenderer* m_Renderer;
};

#endif
