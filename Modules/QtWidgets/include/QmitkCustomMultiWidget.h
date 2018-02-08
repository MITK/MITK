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

// qt
#include <qwidget.h>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkLevelWindowWidget;
class QmitkRenderWindow;
class vtkCornerAnnotation;

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

  using RenderWindowWidgetMap = std::map<QString, QmitkRenderWindowWidget*>;
  using RenderWindowHash = QHash<QString, QmitkRenderWindow*>;
  RenderWindowWidgetMap GetRenderWindowWidgets() const;
  QmitkRenderWindowWidget* GetRenderWindowWidget(const QString& widgetID) const;

  QmitkRenderWindow* GetRenderWindow(const QString& widgetID) const;

  QmitkRenderWindowWidget* GetActiveRenderWindowWidget() const;
  QmitkRenderWindowWidget* GetFirstRenderWindowWidget() const;
  QmitkRenderWindowWidget* GetLastRenderWindowWidget() const;
  
  unsigned int GetNumberOfRenderWindowWidgets() const;

  void RequestUpdate(const QString& widgetID);
  void RequestUpdateAll();
  void ForceImmediateUpdate(const QString& widgetID);
  void ForceImmediateUpdateAll();

  const mitk::Point3D GetCrossPosition(const QString& widgetID) const;

public slots:

  void ShowLevelWindowWidget(const QString& widgetID, bool show);
  void ShowAllLevelWindowWidgets(bool show);
  /**
  * @brief Set a background color gradient for a specific render window.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  * @param widgetID   The widget identifier.
  */
  void SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower, const QString& widgetID);
  /**
  * @brief Set a background color gradient for all available render windows.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  */
  // #TODO: 'backgroundgradientcolor'
  void SetAllBackgroundColorGradients(const mitk::Color& upper, const mitk::Color& lower);
  void FillAllBackgroundColorGradientsWithBlack();
  void ShowBackgroundColorGradient(const QString& widgetID, bool show);
  void ShowAllBackgroundColorGradients(bool show);
  /**
  * @rief Return a render window (widget) specific background color gradient
  *
  * @param widgetID   The widget identifier.
  *
  * @return               A color gradient as a pair of colors.
  *                       First entry: upper color value
  *                       Second entry: lower color value
  */
  std::pair<mitk::Color, mitk::Color> GetBackgroundColorGradient(const QString& widgetID) const;
  bool GetBackgroundColorGradientFlag(const QString& widgetID) const;

  void SetDepartmentLogoPath(const char* path);
  void ShowDepartmentLogo(const QString& widgetID, bool show);
  void ShowAllDepartmentLogos(bool show);

  void SetDecorationColor(const QString& widgetID, const mitk::Color& color);
  mitk::Color GetDecorationColor(const QString& widgetID) const;

  void ShowColoredRectangle(const QString& widgetID, bool show);
  void ShowAllColoredRectangles(bool show);
  bool IsColoredRectangleVisible(const QString& widgetID) const;

  void ShowCornerAnnotation(const QString& widgetID, bool show);
  void ShowAllCornerAnnotations(bool show);
  bool IsCornerAnnotationVisible(const QString& widgetID) const;
  void SetCornerAnnotationText(const QString& widgetID, const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText(const QString& widgetID) const;

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

  void Fit();

  void EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p);

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
  void AddRenderWindowWidget();
  
  // #TODO: see T24173
  mitk::DataNode::Pointer GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

  QGridLayout* m_CustomMultiWidgetLayout;
  RenderWindowWidgetMap m_RenderWindowWidgets;

  QmitkRenderWindowWidget* m_ActiveRenderWindowWidget;

  int m_PlaneMode;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  QString m_MultiWidgetName;

  //mitk::MouseModeSwitcher::Pointer m_MouseModeSwitcher;
  mitk::SliceNavigationController *m_TimeNavigationController;

  mitk::DataStorage::Pointer m_DataStorage;

  bool m_PendingCrosshairPositionEvent;
  bool m_CrosshairNavigationEnabled;

};

#endif // QMITKCUSTOMMULTIWIDGET_H
