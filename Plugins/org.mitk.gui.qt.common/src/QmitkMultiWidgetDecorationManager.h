/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiWidgetDecorationManager_h
#define QmitkMultiWidgetDecorationManager_h

#include <org_mitk_gui_qt_common_Export.h>

// mitk core
#include <mitkColorProperty.h>

// mitk annotation
#include <mitkLogoAnnotation.h>

// mitk qt widgets
#include <QmitkAbstractMultiWidget.h>

// vtk
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

// qt
#include <QString>
#include <QStringList>

namespace mitk
{
  class IPreferences;
}

/**
* @brief
*
*
*/
class MITK_QT_COMMON QmitkMultiWidgetDecorationManager
{

public:

  QmitkMultiWidgetDecorationManager(QmitkAbstractMultiWidget* multiWidget);

  enum class Colormap
  {
    BlackAndWhite = 0 // black background, white decoration
  };

  void DecorationPreferencesChanged(const mitk::IPreferences* preferences);

  /**
  * @brief Show or hide decorations like like colored borders or background, logos, menu widgets, logos and
  *     text annotations.
  *
  * \@par  Show the decorations specified in decorations if true. Hide them, if not.
  * \@par  A list of decoration names. If empty, all supported decorations are affected.
  */
  void ShowDecorations(bool show, const QStringList& decorations);
  /**
  * @brief Return if a specific decoration is visible.
  *
  * \return True, if the specified decoration is shown, false if not.
  */
  bool IsDecorationVisible(const QString &decoration) const;
  QStringList GetDecorations() const;

  void SetupLogo(const char* path);
  void ShowLogo(bool show);
  bool IsLogoVisible() const;

  void SetColormap(Colormap colormap);

  void SetDecorationColor(const QString& widgetID, const mitk::Color& color);
  void SetAllDecorationColors(const mitk::Color& color);
  mitk::Color GetDecorationColor(const QString& widgetID) const;

  void ShowColoredRectangle(const QString& widgetID, bool show);
  void ShowAllColoredRectangles(bool show);
  bool IsColoredRectangleVisible(const QString& widgetID) const;
  bool AreAllColoredRectanglesVisible() const;

  /**
  * @brief Set a background color gradient for a specific render window.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  * @param widgetID   The widget identifier.
  */
  void SetGradientBackgroundColors(const mitk::Color& upper, const mitk::Color& lower, const QString& widgetID);
  /**
  * @brief Set a background color gradient for all available render windows.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  */
  void SetAllGradientBackgroundColors(const mitk::Color& upper, const mitk::Color& lower);
  void FillAllGradientBackgroundColorsWithBlack();
  void ShowGradientBackground(const QString& widgetID, bool show);
  void ShowAllGradientBackgrounds(bool show);
  /**
  * @brief Return a render window (widget) specific background color gradient
  *
  * @param widgetID   The widget identifier.
  *
  * @return               A color gradient as a pair of colors.
  *                       First entry: upper color value
  *                       Second entry: lower color value
  */
  std::pair<mitk::Color, mitk::Color> GetGradientBackgroundColors(const QString& widgetID) const;
  bool IsGradientBackgroundOn(const QString& widgetID) const;
  bool AreAllGradientBackgroundsOn() const;

  void SetCornerAnnotationText(const QString& widgetID, const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText(const QString& widgetID) const;
  void ShowCornerAnnotation(const QString& widgetID, bool show);
  void ShowAllCornerAnnotations(bool show);
  bool IsCornerAnnotationVisible(const QString& widgetID) const;
  bool AreAllCornerAnnotationsVisible() const;

private:

  vtkSmartPointer<vtkImageData> GetVtkLogo(const char* path);
  void SetLogo(vtkSmartPointer<vtkImageData> vtkLogo);

  QmitkAbstractMultiWidget* m_MultiWidget;
  mitk::LogoAnnotation::Pointer m_LogoAnnotation;

};

#endif
