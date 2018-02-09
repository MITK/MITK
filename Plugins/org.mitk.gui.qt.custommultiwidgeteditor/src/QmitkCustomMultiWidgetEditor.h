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

#ifndef QMITKCUSTOMMULTIWIDGETEDITOR_H
#define QMITKCUSTOMMULTIWIDGETEDITOR_H

#include <QmitkAbstractRenderEditor.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkILinkedRenderWindowPart.h>

// custom multi widget editor
#include <org_mitk_gui_qt_custommultiwidgeteditor_Export.h>
// qt widgets module
#include <QmitkCustomMultiWidget.h>

#include <memory>

class QmitkCustomMultiWidget;
class QmitkMouseModeSwitcher;

class CUSTOMMULTIWIDGETEDITOR_EXPORT QmitkCustomMultiWidgetEditor final : public QmitkAbstractRenderEditor, public mitk::ILifecycleAwarePart, public mitk::ILinkedRenderWindowPart
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkCustomMultiWidgetEditor)

  static const QString EDITOR_ID;

  QmitkCustomMultiWidgetEditor();
  ~QmitkCustomMultiWidgetEditor();

  /**
  * @brief Overridden from mitk::ILifecycleAwarePart
  */
  virtual void Activated() override;
  /**
  * @brief Overridden from mitk::ILifecycleAwarePart
  */
  virtual void Deactivated() override;
  /**
  * @brief Overridden from mitk::ILifecycleAwarePart
  */
  virtual void Visible() override;
  /**
  * @brief Overridden from mitk::ILifecycleAwarePart
  */
  virtual void Hidden() override;

  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows() const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual bool IsDecorationEnabled(const QString& decoration) const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart : IRenderWindowPart
  */
  virtual QStringList GetDecorations() const override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart
  */
  virtual void EnableSlicingPlanes(bool enable) override;
  /**
  * @brief Overridden from mitk::ILinkedRenderWindowPart
  */
  virtual bool IsSlicingPlanesEnabled() const override;

  QmitkCustomMultiWidget* GetCustomMultiWidget();

private:
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void SetFocus() override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* preferences) override;
  /**
  * @brief Overridden from QmitkAbstractRenderEditor
  */
  virtual void CreateQtPartControl(QWidget* parent) override;

  /**
  * @brief
  *
  *
  */
  void RequestActivateMenuWidget(bool on);
  /**
  * @brief Set the default preferences. This method is used to show the current preferences
  *     in the first call of the preference page (the GUI).
  *
  * @param preferences  The berry preferences.
  */
  void InitializePreferences(berry::IBerryPreferences* preferences);
  /**
   * @brief FillMembersWithCurrentDecorations Helper method to fill internal members with
   * current values of the std multi widget.
   */
  void FillMembersWithCurrentDecorations();
  /**
   * @brief GetPreferenceDecorations Getter to fill internal members with values of preferences.
   * @param preferences The berry preferences.
   *
   * If a preference is set, the value will overwrite the current value. If it does not exist,
   * the value will not change.
   */
  void GetPreferenceDecorations(const berry::IBerryPreferences* preferences);
  /**
   * @brief Convert a hex color string to mitk::Color.
   *
   * @param   widgetColorInHex    The color in hex format (#RRGGBB) where each digit is in the form (0-F).
   * @return                      The color in mitk format.
   */
  mitk::Color HexColorToMitkColor(const QString& hexColor);
  /**
   * @brief Convert an mitk::Color to a hex color string.
   *
   * @param   color   The color in mitk format.
   * @return          The color as string in hex (#RRGGBB).
   */
  QString MitkColorToHexColor(const mitk::Color& mitkColor);

  QmitkCustomMultiWidget* m_CustomMultiWidget;
  QmitkMouseModeSwitcher* m_MouseModeSwitcher;

};

#endif // QMITKCUSTOMMULTIWIDGETEDITOR_H
