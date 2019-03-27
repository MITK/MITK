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

#ifndef QMITKMXNMULTIWIDGET_H
#define QMITKMXNMULTIWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkDisplayActionEventHandler.h>

// qt
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkRenderWindow;

namespace mitk
{
  class RenderingManager;
}

/**
* @brief The 'QmitkMxNMultiWidget' is a QWidget that is used to display multiple render windows at once.
*
*     Render windows can dynamically be added and removed to change the layout of the multi widget.
*/
class MITKQTWIDGETS_EXPORT QmitkMxNMultiWidget : public QWidget
{
  Q_OBJECT

public:

  using RenderWindowWidgetPointer = std::shared_ptr<QmitkRenderWindowWidget>;
  using RenderWindowWidgetMap = std::map<QString, std::shared_ptr<QmitkRenderWindowWidget>>;
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;

  QmitkMxNMultiWidget(QWidget* parent = 0,
                         Qt::WindowFlags f = 0,
                         mitk::RenderingManager* renderingManager = nullptr,
                         mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
                         const QString& multiWidgetName = "mxnmulti");

  virtual ~QmitkMxNMultiWidget();
  
  void SetDataStorage(mitk::DataStorage* dataStorage);
  void InitializeRenderWindowWidgets();

  mitk::InteractionEventHandler::Pointer GetInteractionEventHandler() { return m_DisplayActionEventBroadcast.GetPointer(); };

  void ResetLayout(int row, int column);
  void Synchronize(bool synchronized);

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
  
  unsigned int GetNumberOfRenderWindowWidgets() const;

  void RequestUpdate(const QString& widgetName);
  void RequestUpdateAll();
  void ForceImmediateUpdate(const QString& widgetName);
  void ForceImmediateUpdateAll();

  void ActivateAllCrosshairs(bool activate);
  const mitk::Point3D GetSelectedPosition(const QString& widgetName) const;

public Q_SLOTS:

  void SetSelectedPosition(const QString& widgetName, const mitk::Point3D& newPosition);

  // mouse events
  void wheelEvent(QWheelEvent* e) override;

  void mousePressEvent(QMouseEvent* e) override;

  void moveEvent(QMoveEvent* e) override;

Q_SIGNALS:

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
  void InitializeDisplayActionEventHandling();

  void CreateRenderWindowWidget(const std::string& cornerAnnotation = "");
  void DestroyRenderWindowWidget();
  void FillMultiWidgetLayout();

  QString GetNameFromIndex(int row, int column) const;
  QString GetNameFromIndex(size_t index) const;

  // #TODO: see T24173
  mitk::DataNode::Pointer GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

  QGridLayout* m_MxNMultiWidgetLayout;
  RenderWindowWidgetMap m_RenderWindowWidgets;

  RenderWindowWidgetPointer m_ActiveRenderWindowWidget;

  int m_MultiWidgetRows;
  int m_MultiWidgetColumns;

  int m_PlaneMode;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  QString m_MultiWidgetName;

  mitk::DisplayActionEventBroadcast::Pointer m_DisplayActionEventBroadcast;
  std::unique_ptr<mitk::DisplayActionEventHandler> m_DisplayActionEventHandler;

  mitk::DataStorage::Pointer m_DataStorage;
};

#endif // QMITKMXNMULTIWIDGET_H
