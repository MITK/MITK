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
#include <QmitkRenderWindow.h>
#include <QmitkLevelWindowWidget.h>

// mitk core
#include <mitkDataStorage.h>
#include <mitkRenderWindow.h>
//#include <mitkSliceNavigationController.h>
#include <vtkMitkRectangleProp.h>
#include <vtkCornerAnnotation.h>

// annotations module
#include <mitkLogoAnnotation.h>

// qt
#include <QWidget>
#include <QHBoxLayout>

/**
* @brief 
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkRenderWindowWidget(QWidget* parent = nullptr, const QString& UID = "", mitk::DataStorage* dataStorage = nullptr);
  ~QmitkRenderWindowWidget() override;

  void SetDataStorage(mitk::DataStorage* dataStorage);

  QmitkRenderWindow* GetRenderWindow() const { return m_RenderWindow; };
  QmitkLevelWindowWidget* GetlevelWindowWidget() const { return m_LevelWindowWidget; };

  mitk::SliceNavigationController* GetSliceNavigationController() const;

  void RequestUpdate();
  void ForceImmediateUpdate();

  // #TODO: 'backroundgradientcolor'
  void SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower);
  void ShowBackgroundColorGradient(bool enable);
  std::pair<mitk::Color, mitk::Color> GetRendererBackgroundColorGradient() const { return m_BackgroundColorGradient; };
  bool GetBackgroundColorGradientFlag() const { return m_BackgroundColorGradientFlag; };

  void ShowLevelWindowWidget(bool enable);
  void ShowDepartmentLogo(bool show);

  void SetDecorationColor(const mitk::Color& color);
  mitk::Color GetDecorationColor() const { return m_DecorationColor; };

  void ShowColoredRectangle(bool show);
  bool IsColoredRectangleVisible() const;

  void ShowCornerAnnotation(bool show);
  bool IsCornerAnnotationVisible() const;
  void SetCornerAnnotationText(const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText() const;

  /**
  * @brief Create a corner annotation and a colored rectangle for this widget.
  *
  * @par text   The text of the corner annotation.
  * @par color  The color of the text and the rectangle.
  */
  void SetDecorationProperties(std::string text, mitk::Color color);

  bool IsRenderWindowMenuActivated() const;

private:

  void InitializeGUI();
  void InitializeDecorations();

  QString m_UID;
  QHBoxLayout* m_Layout;

  mitk::DataStorage* m_DataStorage;
  QmitkRenderWindow* m_RenderWindow;
  QmitkLevelWindowWidget* m_LevelWindowWidget;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  mitk::SliceNavigationController *m_TimeNavigationController;

  std::pair<mitk::Color, mitk::Color> m_BackgroundColorGradient;
  bool m_BackgroundColorGradientFlag;
  mitk::LogoAnnotation::Pointer m_LogoAnnotation;
  mitk::Color m_DecorationColor;
  vtkSmartPointer<vtkMitkRectangleProp> m_RectangleProp;
  vtkSmartPointer<vtkCornerAnnotation> m_CornerAnnotation;
};

#endif // QMITKRENDERWINDOWWIDGET_H
