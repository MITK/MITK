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

#ifndef QMITKRENDERWINDOWWIDGET_H
#define QMITKRENDERWINDOWWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkRenderWindow.h"

// mitk core
#include <mitkDataStorage.h>
#include <mitkPointSet.h>
#include <mitkRenderWindow.h>
#include <vtkMitkRectangleProp.h>
#include <vtkCornerAnnotation.h>

// qt
#include <QWidget>
#include <QHBoxLayout>

/**
* @brief The 'QmitkRenderWindowWidget' is a QWidget that holds a render window
*        and some associates properties, like a crosshair (pointset) and decorations.
*        Decorations are corner annotation (text and color), frame color or background color
*        and can be set using this class.
*        The 'QmitkRenderWindowWidget' is used inside the 'QmitkMxNMultiWidget', where a map contains
*        several render window widgets to create the MxN display.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkRenderWindowWidget(
    QWidget* parent = nullptr,
    const QString& widgetName = "",
    mitk::DataStorage* dataStorage = nullptr,
    mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard
  );

  virtual ~QmitkRenderWindowWidget();

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

  void ActivateCrosshair(bool activate);

private:

  void InitializeGUI();
  void InitializeDecorations();

  void SetCrosshair(mitk::Point3D selectedPoint);

  QString m_WidgetName;
  QHBoxLayout* m_Layout;

  mitk::DataStorage* m_DataStorage;
  QmitkRenderWindow* m_RenderWindow;

  mitk::RenderingManager::Pointer m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;

  mitk::DataNode::Pointer m_PointSetNode;
  mitk::PointSet::Pointer m_PointSet;

  std::pair<mitk::Color, mitk::Color> m_GradientBackgroundColors;
  mitk::Color m_DecorationColor;
  vtkSmartPointer<vtkMitkRectangleProp> m_RectangleProp;
  vtkSmartPointer<vtkCornerAnnotation> m_CornerAnnotation;
};

#endif // QMITKRENDERWINDOWWIDGET_H
