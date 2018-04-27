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

#ifndef QMITKCUSTOMMULTIWIDGET_H
#define QMITKCUSTOMMULTIWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkMouseModeSwitcher.h>
#include <mitkDisplayActionEventHandler.h>

// qt
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkLevelWindowWidget;
class QmitkRenderWindow;

namespace mitk
{
  class RenderingManager;
}

/**
* @brief The 'QmitkCustomMultiWidget' is a 'QWidget' that is used to display multiple render windows at once.
*
*     Render windows can dynamically be added and removed to change the layout of the multi widget.
*/
class MITKQTWIDGETS_EXPORT QmitkCustomMultiWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkCustomMultiWidget(QWidget* parent = 0,
                         Qt::WindowFlags f = 0,
                         mitk::RenderingManager* renderingManager = nullptr,
                         mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
                         const QString& multiWidgetName = "custommulti");

  virtual ~QmitkCustomMultiWidget();
  
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void InitializeRenderWindowWidgets();

  void ResetLayout(int row, int column);
  void Synchronize(bool synchronized);

  using RenderWindowWidgetMap = std::map<QString, std::shared_ptr<QmitkRenderWindowWidget>>;
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;
  RenderWindowWidgetMap GetRenderWindowWidgets() const;
  std::shared_ptr<QmitkRenderWindowWidget> GetRenderWindowWidget(const QString& widgetID) const;

  QString CreateRenderWindowWidget(const std::string& cornerAnnotation = "");
  void AddToMultiWidgetLayout(int row, int column, const QString& widgetID);
  void RemoveRenderWindowWidget(const QString& widgetID);

  QmitkRenderWindow* GetRenderWindow(const QString& widgetID) const;

  std::shared_ptr<QmitkRenderWindowWidget> GetActiveRenderWindowWidget() const;
  std::shared_ptr<QmitkRenderWindowWidget> GetFirstRenderWindowWidget() const;
  std::shared_ptr<QmitkRenderWindowWidget> GetLastRenderWindowWidget() const;
  
  unsigned int GetNumberOfRenderWindowWidgets() const;

  void RequestUpdate(const QString& widgetID);
  void RequestUpdateAll();
  void ForceImmediateUpdate(const QString& widgetID);
  void ForceImmediateUpdateAll();

  mitk::MouseModeSwitcher* GetMouseModeSwitcher();

  const mitk::Point3D GetCrossPosition(const QString& widgetID) const;

public slots:

  void ShowLevelWindowWidget(const QString& widgetID, bool show);
  void ShowAllLevelWindowWidgets(bool show);

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

  void MoveCrossToPosition(const QString& widgetID, const mitk::Point3D& newPosition);

  void ResetCrosshair();

  // mouse events
  void wheelEvent(QWheelEvent* e) override;

  void mousePressEvent(QMouseEvent* e) override;

  void moveEvent(QMoveEvent* e) override;

signals:

  void WheelMoved(QWheelEvent *);
  void Moved();

public:

  enum
  {
    AXIAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };

private:

  void InitializeGUI();
  void InitializeWidget();
  void InitializeDisplayActionEventHandling();

  void FillLayout(int row, int column);
  
  // #TODO: see T24173
  mitk::DataNode::Pointer GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

  QGridLayout* m_CustomMultiWidgetLayout;
  RenderWindowWidgetMap m_RenderWindowWidgets;

  QmitkRenderWindowWidget* m_ActiveRenderWindowWidget;

  int m_PlaneMode;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  QString m_MultiWidgetName;

  mitk::MouseModeSwitcher::Pointer m_MouseModeSwitcher;

  mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
  std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler;
  bool m_Synchronized;

  mitk::DataStorage::Pointer m_DataStorage;

  bool m_PendingCrosshairPositionEvent;
  bool m_CrosshairNavigationEnabled;

};

#endif // QMITKCUSTOMMULTIWIDGET_H
