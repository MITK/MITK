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


#ifndef QmitkOverlayManagerView_h
#define QmitkOverlayManagerView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkOverlayManagerViewControls.h"

class QmitkPropertyItemDelegate;
class QmitkPropertyItemModel;
class QSortFilterProxyModel;

namespace mitk
{
class IPropertyAliases;
class IPropertyDescriptions;
class IPropertyPersistence;
}

/**
  \brief QmitkOverlayManagerView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkOverlayManagerView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;
  QmitkOverlayManagerView();
  ~QmitkOverlayManagerView();

protected:

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  Ui::QmitkOverlayManagerViewControls m_Controls;

private slots:
  void OnCurrentRowChanged(const QModelIndex& current, const QModelIndex& previous);
  void OnPropertyListChanged(int index);
  void OnAddNewProperty();

private:
  QString GetPropertyNameOrAlias(const QModelIndex& index);
  void OnPropertyNameChanged(const itk::EventObject& event);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  QWidget* m_Parent;
  unsigned long m_PropertyNameChangedTag;
  std::string m_SelectionClassName;
  mitk::IPropertyAliases* m_PropertyAliases;
  mitk::IPropertyDescriptions* m_PropertyDescriptions;
  mitk::IPropertyPersistence* m_PropertyPersistence;
  QSortFilterProxyModel* m_ProxyModel;
  QmitkPropertyItemModel* m_Model;
  QmitkPropertyItemDelegate* m_Delegate;
  mitk::DataNode::Pointer m_SelectedNode;
  mitk::BaseRenderer* m_Renderer;

};

#endif // QmitkOverlayManagerView_h
