/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkViewNavigatorView_h
#define QmitkViewNavigatorView_h

#include <berryIPartListener.h>
#include <QmitkAbstractView.h>

class QmitkViewModel;
class QmitkViewProxyModel;

namespace berry
{
  struct IPartService;
  struct IWorkbenchPage;
}

namespace Ui
{
  class QmitkViewNavigatorView;
}

class QmitkViewNavigatorView : public QmitkAbstractView, public berry::IPartListener
{
  Q_OBJECT

public:
  QmitkViewNavigatorView();
  ~QmitkViewNavigatorView() override;

private:
  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

  Events::Types GetPartEventTypes() const override;
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  berry::IWorkbenchPage* GetActivePage() const;
  berry::IPartService* GetPartService() const;

  void OnFilterTextChanged(const QString& filter);
  void OnItemDoubleClicked(const QModelIndex& index);

  Ui::QmitkViewNavigatorView* m_Ui;
  QmitkViewModel* m_Model;
  QmitkViewProxyModel* m_ProxyModel;
};

#endif
