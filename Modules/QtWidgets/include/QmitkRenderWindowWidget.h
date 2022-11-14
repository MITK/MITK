/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKRENDERWINDOWWIDGET_H
#define QMITKRENDERWINDOWWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkRenderWindow.h"
#include "QmitkRenderWindowUtilityWidget.h"

// mitk core
#include <mitkCrosshairManager.h>
#include <mitkDataStorage.h>
#include <mitkRenderWindow.h>

// qt
#include <QFrame>
#include <QMouseEvent>

class vtkCornerAnnotation;

/**
* @brief The 'QmitkRenderWindowWidget' is a QFrame that holds a render window
*        and some associates properties, e.g. decorations.
*        Decorations are corner annotation (text and color), frame color or background color
*        and can be set using this class.
*        The 'QmitkRenderWindowWidget' is used inside a 'QmitkAbstractMultiWidget', where a map contains
*        several render window widgets to create the multi widget display.
*        This class uses a CrosshairManager, which allows to use plane geometries as crosshair.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowWidget : public QFrame
{
  Q_OBJECT

public:

  QmitkRenderWindowWidget(
    QWidget* parent = nullptr,
    const QString& widgetName = "",
    mitk::DataStorage* dataStorage = nullptr,
    bool windowControls = false);

  ~QmitkRenderWindowWidget() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

  const QString& GetWidgetName() const { return m_WidgetName; };
  QmitkRenderWindow* GetRenderWindow() const { return m_RenderWindow; };

  mitk::SliceNavigationController* GetSliceNavigationController() const;

  void RequestUpdate();
  void ForceImmediateUpdate();

  void SetGradientBackgroundColors(const mitk::Color& upper, const mitk::Color& lower);
  void ShowGradientBackground(bool enable);
  std::pair<mitk::Color, mitk::Color> GetGradientBackgroundColors() const { return m_GradientBackgroundColors; };
  bool IsGradientBackgroundOn() const;

  void SetDecorationColor(const mitk::Color& color);
  mitk::Color GetDecorationColor() const { return m_DecorationColor; };

  void ShowColoredRectangle(bool show);
  bool IsColoredRectangleVisible() const;

  void ShowCornerAnnotation(bool show);
  bool IsCornerAnnotationVisible() const;
  void SetCornerAnnotationText(const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText() const;

  bool IsRenderWindowMenuActivated() const;

  void SetCrosshairVisibility(bool visible);
  bool GetCrosshairVisibility();
  void SetCrosshairGap(unsigned int gapSize);

  void AddPlanesToDataStorage();
  void RemovePlanesFromDataStorage();

  void SetCrosshairPosition(const mitk::Point3D& newPosition);
  mitk::Point3D GetCrosshairPosition() const;

  void SetGeometry(const itk::EventObject& event);
  void SetGeometrySlice(const itk::EventObject& event);

private Q_SLOTS:

  void OnReinitAction(QList<mitk::DataNode::Pointer> selectedNodes);
  void OnResetAction(QList<mitk::DataNode::Pointer> selectedNodes);

private:

  void InitializeGUI();
  void InitializeDecorations();
  void ComputeInvertedSliceNavigation();

  QString m_WidgetName;
  QVBoxLayout* m_Layout;

  mitk::DataStorage* m_DataStorage;

  QmitkRenderWindow* m_RenderWindow;

  mitk::CrosshairManager::Pointer m_CrosshairManager;

  std::pair<mitk::Color, mitk::Color> m_GradientBackgroundColors;
  mitk::Color m_DecorationColor;
  vtkSmartPointer<vtkCornerAnnotation> m_CornerAnnotation;

  QmitkRenderWindowUtilityWidget* m_UtilityWidget;

  bool m_WindowControls;

};

#endif // QMITKRENDERWINDOWWIDGET_H
