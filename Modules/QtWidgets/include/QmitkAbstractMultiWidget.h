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

#ifndef QMITKABSTRACTMULTIWIDGET_H
#define QMITKABSTRACTMULTIWIDGET_H

// mitk qt widgets module
#include "MitkQtWidgetsExports.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkPoint.h>

// qt
#include <QWidget>

// c++
#include <map>
#include <memory>

class QmitkRenderWindow;
class QmitkRenderWindowWidget;

namespace mitk
{
  class DataStorage;
  class InteractionEventHandler;
  class RenderingManager;
}

/**
* @brief The 'QmitkAbstractMultiWidget' is a 'QWidget' that is can be subclassed to display multiple render windows at once.
*        Render windows can dynamically be added and removed to change the layout of the multi widget.
*        A subclass of this multi widget can be used inside an 'QmitkAbstractMultiWidgetEditor'.
*/
class MITKQTWIDGETS_EXPORT QmitkAbstractMultiWidget : public QWidget
{
  Q_OBJECT

public:

  using RenderWindowWidgetPointer = std::shared_ptr<QmitkRenderWindowWidget>;
  using RenderWindowWidgetMap = std::map<QString, std::shared_ptr<QmitkRenderWindowWidget>>;
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;

  enum
  {
    AXIAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };

  QmitkAbstractMultiWidget(QWidget* parent = 0,
                           Qt::WindowFlags f = 0,
                           mitk::RenderingManager* renderingManager = nullptr,
                           mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
                           const QString& multiWidgetName = "multiwidget");

  virtual ~QmitkAbstractMultiWidget();
  
  void SetDataStorage(mitk::DataStorage* dataStorage);
  mitk::DataStorage* GetDataStorage() const;

  void SetRowCount(int row);
  int GetRowCount() const;
  void SetColumnCount(int column);
  int GetColumnCount() const;
  void SetLayout(int row, int column);
  void Synchronize(bool synchronized);
  mitk::InteractionEventHandler* GetInteractionEventHandler();

  RenderWindowWidgetMap GetRenderWindowWidgets() const;
  RenderWindowWidgetPointer GetRenderWindowWidget(int row, int column) const;
  RenderWindowWidgetPointer GetRenderWindowWidget(const QString& widgetName) const;
  RenderWindowHash GetRenderWindows() const;
  QmitkRenderWindow* GetRenderWindow(int row, int column) const;
  QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const;

  void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget);
  RenderWindowWidgetPointer GetActiveRenderWindowWidget() const;
  RenderWindowWidgetPointer GetFirstRenderWindowWidget() const;
  RenderWindowWidgetPointer GetLastRenderWindowWidget() const;
  
  void AddRenderWindowWidget(const QString& widgetName, RenderWindowWidgetPointer renderWindowWidget);
  void RemoveRenderWindowWidget();

  QString GetNameFromIndex(int row, int column) const;
  QString GetNameFromIndex(size_t index) const;

  unsigned int GetNumberOfRenderWindowWidgets() const;

  void RequestUpdate(const QString& widgetName);
  void RequestUpdateAll();
  void ForceImmediateUpdate(const QString& widgetName);
  void ForceImmediateUpdateAll();

  const mitk::Point3D GetSelectedPosition(const QString& widgetName) const;

public Q_SLOTS:

  /**
  * @brief Listener to the CrosshairPositionEvent
  *
  *   Ensures the CrosshairPositionEvent is handled only once and at the end of the Qt-Event loop
  */
  void HandleCrosshairPositionEvent();
  /**
  * @brief Receives the signal from HandleCrosshairPositionEvent, executes the StatusBar update
  *
  */
  void HandleCrosshairPositionEventDelayed();

  void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName);

  void ResetCrosshair();

  // mouse events
  void wheelEvent(QWheelEvent* e) override;

  void mousePressEvent(QMouseEvent* e) override;

  void moveEvent(QMoveEvent* e) override;

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();

private:

  virtual void InitializeGUI() = 0;

  /**
  * @brief This function will be called by the function 'SetLayout' and
  *        needs to be implemented and customized in the subclasses.
  */
  virtual void SetLayoutImpl() = 0;

  int m_PlaneMode;

  bool m_PendingCrosshairPositionEvent;
  bool m_CrosshairNavigationEnabled;

  struct Impl;
  std::unique_ptr<Impl> m_Impl;

};

#endif // QMITKABSTRACTMULTIWIDGET_H
