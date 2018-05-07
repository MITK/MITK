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
#include "QmitkMultiWidgetDecorationManager.h"

// mitk qt widgets module
#include <QmitkCustomMultiWidget.h>
#include <QmitkInteractionSchemeToolBar.h>
#include <QmitkMultiWidgetConfigurationToolBar.h>

// mitk render window manager
#include <mitkRenderWindowViewDirectionController.h>

#include <memory>

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

private slots:

  void OnLayoutSet(int row, int column);
  void OnSynchronize(bool synchronized);
  void OnViewDirectionChanged(mitk::SliceNavigationController::ViewDirection viewDirection);

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

  void SetControlledRenderer();
  /**
  * @brief
  *
  *
  */
  void RequestActivateMenuWidget(bool on);

  QmitkCustomMultiWidget* m_CustomMultiWidget;
  QmitkInteractionSchemeToolBar* m_InteractionSchemeToolBar;
  QmitkMultiWidgetConfigurationToolBar* m_ConfigurationToolBar;
  std::unique_ptr<QmitkMultiWidgetDecorationManager> m_MultiWidgetDecorationManager;

  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;

};

#endif // QMITKCUSTOMMULTIWIDGETEDITOR_H
