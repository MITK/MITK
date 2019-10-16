/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKMXNMULTIWIDGETEDITOR_H
#define QMITKMXNMULTIWIDGETEDITOR_H

#include <QmitkAbstractRenderEditor.h>
#include <mitkILinkedRenderWindowPart.h>

#include <org_mitk_gui_qt_mxnmultiwidgeteditor_Export.h>

// berry
#include <berryIPartListener.h>

#include <mitkInteractionSchemeSwitcher.h>

#include <memory>

class QmitkMxNMultiWidget;

class MXNMULTIWIDGETEDITOR_EXPORT QmitkMxNMultiWidgetEditor final : public QmitkAbstractRenderEditor, public berry::IPartListener
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkMxNMultiWidgetEditor)

  static const QString EDITOR_ID;

  QmitkMxNMultiWidgetEditor();
  ~QmitkMxNMultiWidgetEditor() override;

  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows() const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  bool IsDecorationEnabled(const QString& decoration) const override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor : IRenderWindowPart
  */
  QStringList GetDecorations() const override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  berry::IPartListener::Events::Types GetPartEventTypes() const override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  void PartClosed(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  /**
  * @brief Return the current MxN multi widget of this editor.
  */
  QmitkMxNMultiWidget* GetMxNMultiWidget();

private Q_SLOTS:

  void OnLayoutSet(int row, int column);
  void OnSynchronize(bool synchronized);
  void OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

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
  void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;

  class Impl;
  const std::unique_ptr<Impl> m_Impl;
};

#endif // QMITKMXNMULTIWIDGETEDITOR_H
