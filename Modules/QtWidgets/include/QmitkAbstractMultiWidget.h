/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKABSTRACTMULTIWIDGET_H
#define QMITKABSTRACTMULTIWIDGET_H

// mitk qt widgets module
#include "MitkQtWidgetsExports.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDisplayActionEventHandler.h>
#include <mitkInteractionSchemeSwitcher.h>
#include <mitkPoint.h>

// qt
#include <QWidget>

// c++
#include <map>
#include <memory>

class QmitkMultiWidgetLayoutManager;
class QmitkRenderWindow;
class QmitkRenderWindowWidget;

namespace mitk
{
  class DataStorage;
  class InteractionEventHandler;
}

/**
* @brief The 'QmitkAbstractMultiWidget' is a 'QWidget' that can be subclassed to display multiple render windows at once.
*        Render windows can dynamically be added and removed to change the layout of the multi widget.
*        A subclass of this multi widget can be used inside a 'QmitkAbstractMultiWidgetEditor'.
*
*        The class uses the 'DisplayActionEventBroadcast' and 'DisplayActionEventHandler' classes to
*        load a state machine and set an event configuration.
*
*        Using the 'Synchronize' function the user can enable or disable the synchronization of display action events.
*        See 'DisplayActionEventFunctions'-class for the different synchronized and non-synchronized functions used.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractMultiWidget : public QWidget
{
  Q_OBJECT

public:

  using RenderWindowWidgetPointer = std::shared_ptr<QmitkRenderWindowWidget>;
  using RenderWindowWidgetMap = std::map<QString, std::shared_ptr<QmitkRenderWindowWidget>>;
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;
  using ViewDirection = mitk::BaseRenderer::ViewDirection;

  QmitkAbstractMultiWidget(QWidget* parent = 0,
                           Qt::WindowFlags f = 0,
                           const QString& multiWidgetName = "multiwidget");

  virtual ~QmitkAbstractMultiWidget();

  virtual void InitializeMultiWidget() = 0;
  virtual void MultiWidgetOpened() { }
  virtual void MultiWidgetClosed() { }

  virtual void SetDataStorage(mitk::DataStorage* dataStorage);
  mitk::DataStorage* GetDataStorage() const;

  int GetRowCount() const;
  int GetColumnCount() const;
  virtual void SetLayout(int row, int column);

  virtual void Synchronize(bool) { };
  virtual void SetInteractionScheme(mitk::InteractionSchemeSwitcher::InteractionScheme scheme);

  mitk::InteractionEventHandler* GetInteractionEventHandler();
  void SetDisplayActionEventHandler(std::unique_ptr<mitk::DisplayActionEventHandler> displayActionEventHandler);
  mitk::DisplayActionEventHandler* GetDisplayActionEventHandler();

  RenderWindowWidgetMap GetRenderWindowWidgets() const;
  RenderWindowWidgetMap Get2DRenderWindowWidgets() const;
  RenderWindowWidgetMap Get3DRenderWindowWidgets() const;
  RenderWindowWidgetPointer GetRenderWindowWidget(int row, int column) const;
  RenderWindowWidgetPointer GetRenderWindowWidget(const QString& widgetName) const;
  RenderWindowWidgetPointer GetRenderWindowWidget(const QmitkRenderWindow* renderWindow) const;
  RenderWindowHash GetRenderWindows() const;
  QmitkRenderWindow* GetRenderWindow(int row, int column) const;
  virtual QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const;
  virtual QmitkRenderWindow* GetRenderWindow(const ViewDirection& viewDirection) const = 0;

  virtual void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget);
  RenderWindowWidgetPointer GetActiveRenderWindowWidget() const;
  RenderWindowWidgetPointer GetFirstRenderWindowWidget() const;
  RenderWindowWidgetPointer GetLastRenderWindowWidget() const;

  virtual QString GetNameFromIndex(int row, int column) const;
  virtual QString GetNameFromIndex(size_t index) const;

  unsigned int GetNumberOfRenderWindowWidgets() const;

  void RequestUpdate(const QString& widgetName);
  void RequestUpdateAll();
  void ForceImmediateUpdate(const QString& widgetName);
  void ForceImmediateUpdateAll();

  virtual void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) = 0;
  virtual const mitk::Point3D GetSelectedPosition(const QString& widgetName) const = 0;

  virtual void SetCrosshairVisibility(bool visible) = 0;
  virtual bool GetCrosshairVisibility() const = 0;

  virtual void ResetCrosshair() = 0;

  virtual void SetWidgetPlaneMode(int mode) = 0;

  virtual void ActivateMenuWidget(bool state);
  virtual bool IsMenuWidgetEnabled() const;

  QmitkMultiWidgetLayoutManager* GetMultiWidgetLayoutManager() const;

Q_SIGNALS:

  void ActiveRenderWindowChanged();

protected:

  virtual void AddRenderWindowWidget(const QString& widgetName, RenderWindowWidgetPointer renderWindowWidget);
  virtual void RemoveRenderWindowWidget();

private:

  /**
  * @brief This function will be called by the function 'SetLayout' and
  *        can be implemented and customized in the subclasses.
  */
  virtual void SetLayoutImpl() = 0;
  /**
  * @brief This function will be called by the function 'SetInteractionScheme' and
  *        can be implemented and customized in the subclasses.
  */
  virtual void SetInteractionSchemeImpl() = 0;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif // QMITKABSTRACTMULTIWIDGET_H
