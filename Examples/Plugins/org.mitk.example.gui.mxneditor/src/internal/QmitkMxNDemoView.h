/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNDemoView_h
#define QmitkMxNDemoView_h

#include <QmitkAbstractView.h>
#include <QmitkMxNMultiWidget.h>

#include <berryIPartListener.h>

#include <QMetaObject>
#include <QPointer>

#include <map>

class QCheckBox;
class QLabel;
class QPushButton;

/**
 * \brief Drives the plugin-owned MxN demo editor from code.
 *
 * The editor's declaration sets its starting locks and layout; this view shows
 * the programmatic path next to it: it opens the editor, changes its locks and
 * applies another layout. It holds the editor's multi widget only as a guarded
 * pointer and looks it up again on part events, because every reopened editor
 * builds a new widget.
 */
class QmitkMxNDemoView : public QmitkAbstractView, public berry::IPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  /** \brief The editor declared in this plugin's plugin.xml. */
  static const QString EDITOR_ID;

  ~QmitkMxNDemoView() override;

  berry::IPartListener::Events::Types GetPartEventTypes() const override;
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override;

protected:

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:

  void OpenDemoDisplay();
  void SwitchLayout();

  /** \brief Point the view at the demo editor's current multi widget, or at
   *         none if the editor is not open. */
  void SetMultiWidget(QmitkMxNMultiWidget* multiWidget);
  QmitkMxNMultiWidget* FindMultiWidget() const;

  void ShowLocks(QmitkMxNLayoutLocks locks);
  void ApplyLocksFromBoxes();

  QPointer<QmitkMxNMultiWidget> m_MultiWidget;
  QMetaObject::Connection m_LocksConnection;

  QPushButton* m_OpenButton = nullptr;
  QPushButton* m_SwitchButton = nullptr;
  std::map<QmitkMxNLayoutLock, QCheckBox*> m_LockBoxes;
  QLabel* m_Status = nullptr;

  bool m_ShowsAltLayout = false;
};

#endif
