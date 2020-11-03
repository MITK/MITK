/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSTDMULTIWIDGETEDITOR_H
#define QMITKSTDMULTIWIDGETEDITOR_H

// mitk gui qt common plugin
#include <QmitkAbstractMultiWidgetEditor.h>

#include <mitkILinkedRenderWindowPart.h>

#include <org_mitk_gui_qt_stdmultiwidgeteditor_Export.h>

// c++
#include <memory>

class QmitkStdMultiWidget;
class QmitkStdMultiWidgetEditorPrivate;

/**
 * @brief
 */
class ORG_MITK_GUI_QT_STDMULTIWIDGETEDITOR QmitkStdMultiWidgetEditor final : public QmitkAbstractMultiWidgetEditor,
                                                                             public mitk::ILinkedRenderWindowPart
{
  Q_OBJECT

public:

  static const QString EDITOR_ID;

  QmitkStdMultiWidgetEditor();
  virtual ~QmitkStdMultiWidgetEditor() override;

  virtual QmitkLevelWindowWidget* GetLevelWindowWidget() const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart
  */
  virtual void EnableSlicingPlanes(bool enable) override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart
  */
  virtual bool IsSlicingPlanesEnabled() const override;
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

  void OnInteractionSchemeChanged(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

  void ShowLevelWindowWidget(bool show);

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void CreateQtPartControl(QWidget* parent) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;
  /**
  * @brief InitializePreferences Internal helper method to set default preferences.
  * This method is used to show the current preferences in the first call of
  * the preference page (the GUI).
  *
  * @param preferences berry preferences.
  */
  void InitializePreferences(berry::IBerryPreferences *preferences);
  /**
   * @brief GetPreferenceDecorations Getter to fill internal members with values of preferences.
   * @param preferences The berry preferences.
   *
   * If a preference is set, the value will overwrite the current value. If it does not exist,
   * the value will not change.
   */
  void GetPreferenceDecorations(const berry::IBerryPreferences *preferences);
  /**
   * @brief GetColorForWidget helper method to convert a saved color string to mitk::Color.
   * @param hexColor color in hex format (#12356) where each digit is in the form (0-F).
   * @return the color in mitk format.
   */
  mitk::Color HexColorToMitkColor(const QString& hexColor);
  /**
   * @brief MitkColorToHex Convert an mitk::Color to hex string.
   * @param color mitk format.
   * @return String in hex (#RRGGBB).
   */
  QString MitkColorToHex(const mitk::Color& color);

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif // QMITKSTDMULTIWIDGETEDITOR_H
