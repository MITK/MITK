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

#include <QmitkAbstractView.h>
#include <ui_QmitkPropertyTreeView.h>

class QmitkPropertyItemModel;
class QmitkPropertyItemSortFilterProxyModel;

namespace mitk
{
  class PropertyAliases;
  class PropertyDescriptions;
}

class QmitkPropertyTreeView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  berryObjectMacro(QmitkPropertyTreeView)

  QmitkPropertyTreeView();
  ~QmitkPropertyTreeView();

  void SetFocus();

protected:
  void CreateQtPartControl(QWidget* parent);

private:
  QString GetPropertyNameOrAlias(const QModelIndex& index);
  void OnPreferencesChanged(const berry::IBerryPreferences* preferences);
  void OnPropertyNameChanged(const itk::EventObject& event);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes);

private slots:
  void OnCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnFilterTextChanged(const QString& filter);
  void OnModelReset();

private:
  unsigned long m_PropertyNameChangedTag;
  mitk::PropertyAliases* m_PropertyAliases;
  mitk::PropertyDescriptions* m_PropertyDescriptions;
  bool m_ShowGenuineNames;
  Ui::QmitkPropertyTreeView m_Controls;
  QmitkPropertyItemSortFilterProxyModel* m_ProxyModel;
  QmitkPropertyItemModel* m_Model;
};

#endif
