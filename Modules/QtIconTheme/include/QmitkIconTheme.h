/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIconTheme_h
#define QmitkIconTheme_h

#include <MitkQtIconThemeExports.h>

#include <QByteArray>
#include <QIcon>
#include <QString>

/**
 * \brief Utility class for theme-aware icon and color management.
 *
 * Provides static methods to create icons whose colors adapt to the current
 * application theme (dark/light mode). SVG icons are recolored by replacing
 * magic colors with the theme-appropriate colors.
 *
 * This class cannot be instantiated.
 *
 * \sa QmitkColoredNodeDescriptor
 */
class MITKQTICONTHEME_EXPORT QmitkIconTheme
{
public:
  /**
   * \brief Creates a theme-colored icon from raw SVG data.
   * \param[in] originalSVG The original SVG content as a byte array.
   * \return A QIcon with colors adapted to the current theme.
   */
  static QIcon GetIcon(const QByteArray &originalSVG);

  /**
   * \brief Creates a theme-colored icon from an SVG resource file.
   * \param[in] resourcePath The Qt resource path to the SVG file.
   * \return A QIcon with colors adapted to the current theme.
   */
  static QIcon GetIcon(const QString &resourcePath);

  /**
   * \brief Replaces the magic icon color in SVG data by a custom color.
   *
   * Both the six-digit and the equivalent three-digit notation of the magic
   * color are replaced.
   *
   * \param[in] svg The original SVG content.
   * \param[in] color A CSS-compatible color string (e.g., "#ffffff").
   * \return The SVG content with the magic icon color replaced.
   */
  static QString ReplaceColor(const QString &svg, const QString &color);

  /**
   * \brief Returns the primary icon color for the current theme.
   * \return A CSS-compatible color string (e.g., "#ffffff").
   */
  static QString GetColor();

  /**
   * \brief Returns the accent icon color for the current theme.
   * \return A CSS-compatible color string.
   */
  static QString GetAccentColor();

  QmitkIconTheme() = delete;
  ~QmitkIconTheme() = delete;
};

#endif
