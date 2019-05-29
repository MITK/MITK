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

#ifndef QMITKMXNMULTIWIDGETDECORATIONMANAGER_H
#define QMITKMXNMULTIWIDGETDECORATIONMANAGER_H

// mxn multi widget editor
#include <org_mitk_gui_qt_mxnmultiwidgeteditor_Export.h>

// mitk core
#include <mitkColorProperty.h>

// mitk annotation
#include <mitkLogoAnnotation.h>

// mitk qtwidgets
#include <QmitkMxNMultiWidget.h>

// berry
#include <berryIBerryPreferences.h>

// vtk
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

// qt
#include <QString>
#include <QStringList>

/**
* @brief
*
*
*/
class MXNMULTIWIDGETEDITOR_EXPORT QmitkMultiWidgetDecorationManager
{

public:

  QmitkMultiWidgetDecorationManager(QmitkMxNMultiWidget* mxnMultiWidget);

  enum class Colormap
  {
    BlackAndWhite = 0 // black background, white decoration
  };

  void DecorationPreferencesChanged(const berry::IBerryPreferences* preferences);

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

private:

  void SetupLogo(const char* path);
  vtkSmartPointer<vtkImageData> GetVtkLogo(const char* path);
  void SetLogo(vtkSmartPointer<vtkImageData> vtkLogo);
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

  QmitkMxNMultiWidget* m_MxNMultiWidget;
  mitk::LogoAnnotation::Pointer m_LogoAnnotation;

};

#endif // QMITKMXNMULTIWIDGETDECORATIONMANAGER_H
