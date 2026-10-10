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
#include <QmitkMxNMultiWidget.h>

// c++
#include <memory>

class MXNMULTIWIDGETEDITOR_EXPORT QmitkMxNMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor
{
  Q_OBJECT

public:

  /** \brief ID of the stock MxN editor declaration. Other declarations of this
   *         class use their own IDs; nothing in the editor compares against
   *         this one. */
  static const QString EDITOR_ID;

  QmitkMxNMultiWidgetEditor();
  ~QmitkMxNMultiWidgetEditor() override;

  /**
  * \brief Reads the declaration's parameters before the part is created.
  *
  *   - 'widgetName': default the declaration's editor ID.
  *   - 'layoutLock': see 'QmitkMxNParseLayoutLocks'; default none.
  *   - 'initialLayout': a layout JSON file embedded in the declaring plugin,
  *     path relative to it; default the widget's 1x1 default.
  *
  *   Locks and layout are starting values only; code can change both later
  *   through the multi widget.
  *
  *   Never throws: an unknown parameter name, an invalid 'widgetName', an
  *   invalid 'layoutLock', or an 'initialLayout' resource that is missing or
  *   not valid JSON is logged and recorded. The editor then shows an error
  *   panel naming the declaring plugin, the parameter and the value instead of
  *   the MxN display, because the Qt workbench shows nothing for a part that
  *   fails to create.
  */
  void SetInitializationData(const berry::IConfigurationElement::Pointer& config,
                             const QString& propertyName,
                             const berry::Object::Pointer& data) override;

  QmitkLevelWindowWidget* GetLevelWindowWidget() const override { return nullptr; }

  /**
  * @brief Overridden from berry::IPartListener
  */
  berry::IPartListener::Events::Types GetPartEventTypes() const override;
  /**
  * @brief Overridden from berry::IPartListener
  */
  void PartOpened(const berry::IWorkbenchPartReference::Pointer& partRef) override;

  void OnLayoutSet(int row, int column) override;
  /**
  * @brief Syncs both tool bars with the scheme the multi widget has actually
  *        applied, wherever the change came from.
  */
  void OnInteractionSchemeApplied(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

  /** \brief The interaction scheme currently in effect for this editor. The
   *         layout editor hosts the toggle for it and reads this to show the
   *         live state. */
  mitk::InteractionSchemeSwitcher::InteractionScheme GetInteractionScheme() const;

  /** \brief Whether the declaration, or applying its initial layout, failed.
   *         Such an editor has no multi widget and shows an error panel
   *         instead. */
  bool HasConfigurationErrors() const;

private Q_SLOTS:

  void OnLayoutChanged();

  /** \brief Toggle (a cell's sync barcode) or show ("Open layout editor" in a
   *         cell's context menu) the dockable MxN layout editor view. */
  void OnLayoutEditorRequested(QmitkMxNMultiWidget::LayoutEditorRequest request);

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

  /** \brief Logs a configuration error and records it for the error panel. */
  void RecordConfigurationError(const QString& parameter, const QString& problem);

  void ShowConfigurationErrors(QWidget* parent);

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif
