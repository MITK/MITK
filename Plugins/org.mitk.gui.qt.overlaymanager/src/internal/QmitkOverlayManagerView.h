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

#include "mitkAnnotation.h"
#include "mitkILifecycleAwarePart.h"
#include "ui_QmitkOverlayManagerViewControls.h"
#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>

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
  \brief QmitkOverlayManagerView allows to view all overlays added to the mitk::OverlayManager. Properties of these
  overlays
  can be modified.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkOverlayManagerView : public QmitkAbstractView,
                                public mitk::IRenderWindowPartListener,
                                public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  typedef std::map<std::string, mitk::Annotation::Pointer> OverlayMapType;
  static const std::string VIEW_ID;
  QmitkOverlayManagerView();
  ~QmitkOverlayManagerView();

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *) override;

  Ui::QmitkOverlayManagerViewControls m_Controls;
  void OnFocusChanged(itk::Object *, const itk::EventObject &event);

private slots:
  void OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);
  void OnPropertyListChanged(int index);
  void OnAddNewProperty();
  void OnActivateOverlayList();
  void OnOverlaySelectionChanged(QListWidgetItem *current, QListWidgetItem *);

  void OnDelete();
  void OnAddOverlay();

private:
  mitk::Annotation::Pointer CreateTextOverlay2D();
  mitk::Annotation::Pointer CreateTextOverlay3D();
  mitk::Annotation::Pointer CreateLabelOverlay();
  mitk::Annotation::Pointer CreateColorBarOverlay();
  mitk::Annotation::Pointer CreateScaleLegendOverlay();
  mitk::Annotation::Pointer CreateLogoOverlay();

  QString GetPropertyNameOrAlias(const QModelIndex &index);
  void OnPropertyNameChanged(const itk::EventObject &event);
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes) override;

  void InitializeAddOverlayMenu();

  /** \see berry::IPartListener::PartActivated */
  virtual void Activated();

  /** \see berry::IPartListener::PartDeactivated */
  virtual void Deactivated();

  /** \see berry::IPartListener::PartVisible */
  virtual void Visible();

  /** \see berry::IPartListener::PartHidden */
  virtual void Hidden();

  QWidget *m_Parent;
  unsigned long m_PropertyNameChangedTag;
  unsigned long m_OverlayManagerObserverTag;
  std::string m_SelectionClassName;
  mitk::IPropertyAliases *m_PropertyAliases;
  mitk::IPropertyDescriptions *m_PropertyDescriptions;
  mitk::IPropertyPersistence *m_PropertyPersistence;
  QSortFilterProxyModel *m_ProxyModel;
  QmitkPropertyItemModel *m_Model;
  QmitkPropertyItemDelegate *m_Delegate;
  mitk::Annotation::Pointer m_SelectedOverlay;
  mitk::BaseRenderer *m_Renderer;
  QMenu *m_AddOverlayMenu;
  unsigned long m_RenderWindowFocusObserverTag;
  OverlayMapType m_OverlayMap;
};

#endif // QmitkOverlayManagerView_h
