/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStyleManager_h
#define QmitkStyleManager_h

#include <MitkQtWidgetsExports.h>

#include <QByteArray>
#include <QIcon>
#include <QString>

/**
 * \brief Utility class for theme-aware icon and color management.
 *
 * Provides static methods to create icons whose colors adapt to the current
 * application theme (dark/light mode). SVG icons are recolored by replacing
 * a placeholder color with the theme-appropriate icon color.
 *
 * This class cannot be instantiated.
 *
 * \sa QmitkColoredNodeDescriptor
 */
class MITKQTWIDGETS_EXPORT QmitkStyleManager
{
public:
  /**
   * \brief Creates a theme-colored icon from raw SVG data.
   * \param[in] originalSVG The original SVG content as a byte array.
   * \return A QIcon with colors adapted to the current theme.
   */
  static QIcon ThemeIcon(const QByteArray &originalSVG);

  /**
   * \brief Creates a theme-colored icon from an SVG resource file.
   * \param[in] resourcePath The Qt resource path to the SVG file.
   * \return A QIcon with colors adapted to the current theme.
   */
  static QIcon ThemeIcon(const QString &resourcePath);

  /**
   * \brief Returns the primary icon color for the current theme.
   * \return A CSS-compatible color string (e.g., "#ffffff").
   */
  static QString GetIconColor();

  /**
   * \brief Returns the accent icon color for the current theme.
   * \return A CSS-compatible color string.
   */
  static QString GetIconAccentColor();

  QmitkStyleManager() = delete;
  ~QmitkStyleManager() = delete;
};

#endif
