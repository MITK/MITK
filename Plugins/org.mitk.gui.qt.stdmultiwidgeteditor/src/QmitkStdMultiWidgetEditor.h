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

#ifndef QmitkStdMultiWidgetEditor_h
#define QmitkStdMultiWidgetEditor_h

#include <QmitkAbstractRenderEditor.h>

#include <mitkILinkedRenderWindowPart.h>

#include <org_mitk_gui_qt_stdmultiwidgeteditor_Export.h>

class QmitkStdMultiWidget;
class QmitkMouseModeSwitcher;
class QmitkStdMultiWidgetEditorPrivate;

/**
 * \ingroup org_mitk_gui_qt_stdmultiwidgeteditor
 */
class ORG_MITK_GUI_QT_STDMULTIWIDGETEDITOR QmitkStdMultiWidgetEditor
    : public QmitkAbstractRenderEditor, public mitk::ILinkedRenderWindowPart
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkStdMultiWidgetEditor)

  static const QString EDITOR_ID;

  QmitkStdMultiWidgetEditor();
  ~QmitkStdMultiWidgetEditor();

  QmitkStdMultiWidget* GetStdMultiWidget();

  /// \brief If on=true will request the QmitkStdMultiWidget set the Menu widget to
  /// whatever was the last known enabled state, and if on=false will turn the Menu widget off.
  void RequestActivateMenuWidget(bool on);

  // -------------------  mitk::IRenderWindowPart  ----------------------

  /**
   * \see mitk::IRenderWindowPart::GetActiveQmitkRenderWindow()
   */
  QmitkRenderWindow* GetActiveQmitkRenderWindow() const override;

  /**
   * \see mitk::IRenderWindowPart::GetQmitkRenderWindows()
   */
  QHash<QString,QmitkRenderWindow*> GetQmitkRenderWindows() const override;

  /**
   * \see mitk::IRenderWindowPart::GetQmitkRenderWindow(QString)
   */
  QmitkRenderWindow* GetQmitkRenderWindow(const QString& id) const override;

  /**
   * \see mitk::IRenderWindowPart::GetSelectionPosition()
   */
  mitk::Point3D GetSelectedPosition(const QString& id = QString()) const override;

  /**
   * \see mitk::IRenderWindowPart::SetSelectedPosition()
   */
  void SetSelectedPosition(const mitk::Point3D& pos, const QString& id = QString()) override;

  /**
   * \see mitk::IRenderWindowPart::EnableDecorations()
   */
  void EnableDecorations(bool enable, const QStringList& decorations = QStringList()) override;

  /**
   * \see mitk::IRenderWindowPart::IsDecorationEnabled()
   */
  bool IsDecorationEnabled(const QString& decoration) const override;

  /**
   * \see mitk::IRenderWindowPart::GetDecorations()
   */
  QStringList GetDecorations() const override;

  virtual void SetWindowPresetBeforeLoading();
  virtual void SetWindowPresetAfterLoading();

  // -------------------  mitk::ILinkedRenderWindowPart  ----------------------

  void EnableSlicingPlanes(bool enable) override;
  bool IsSlicingPlanesEnabled() const override;


protected:

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
  void GetPreferenceDecorations(const berry::IBerryPreferences *preferences);

  void SetFocus() override;

  void OnPreferencesChanged(const berry::IBerryPreferences*) override;

  void CreateQtPartControl(QWidget* parent) override;

  /**
   * @brief GetColorForWidget helper method to convert a saved color string to mitk::Color.
   * @param widgetColorInHex color in hex format (#12356) where each diget is in the form (0-F).
   * @return the color in mitk format.
   */
  mitk::Color HexColorToMitkColor(const QString& widgetColorInHex);
  /**
   * @brief MitkColorToHex Convert an mitk::Color to hex string.
   * @param color mitk format.
   * @return String in hex (#RRGGBB).
   */
  QString MitkColorToHex(const mitk::Color& color);

  /**
   * @brief InitializePreferences Internal helper method to set default preferences.
   * This method is used to show the current preferences in the first call of
   * the preference page (the GUI).
   *
   * @param preferences berry preferences.
   */
  void InitializePreferences(berry::IBerryPreferences *preferences);

private:
  const QScopedPointer<QmitkStdMultiWidgetEditorPrivate> d;
};
#endif /*QmitkStdMultiWidgetEditor_h*/
