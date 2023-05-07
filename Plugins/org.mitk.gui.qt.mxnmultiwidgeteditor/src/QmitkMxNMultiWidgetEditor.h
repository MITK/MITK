/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNMultiWidgetEditor_h
#define QmitkMxNMultiWidgetEditor_h

#include <org_mitk_gui_qt_mxnmultiwidgeteditor_Export.h>

#include <QmitkAbstractMultiWidgetEditor.h>

// c++
#include <memory>

class QmitkMxNMultiWidget;

class MXNMULTIWIDGETEDITOR_EXPORT QmitkMxNMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor
{
  Q_OBJECT

public:

  static const QString EDITOR_ID;

  QmitkMxNMultiWidgetEditor();
  ~QmitkMxNMultiWidgetEditor() override;

  virtual QmitkLevelWindowWidget* GetLevelWindowWidget() const override { return nullptr; }

  /**
* @brief Overridden from berry::IPartListener
*/
  virtual berry::IPartListener::Events::Types GetPartEventTypes() const override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  virtual void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  virtual void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  virtual void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  virtual void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  void OnLayoutSet(int row, int column) override;
  void OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme) override;

private Q_SLOTS:

  void OnLayoutChanged();

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void CreateQtPartControl(QWidget* parent) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  void OnPreferencesChanged(const mitk::IPreferences* preferences) override;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif
