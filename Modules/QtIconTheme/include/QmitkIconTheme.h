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
#include <QObject>
#include <QString>

/**
 * \brief Theme-aware icon and color management.
 *
 * SVG icons are recolored by replacing the magic icon color \#00ff00 and the
 * magic accent color \#ff00ff, in six-digit or three-digit notation, with the
 * icon colors declared in the application style sheet. Icons created by
 * GetIcon() are rendered from the SVG at the size and device pixel ratio they
 * are displayed with, so they stay sharp at any scale, and they follow theme
 * switches at runtime: after Refresh() they re-render in the current colors
 * on their next repaint, so callers can store them like any other QIcon.
 * Consumers that derive and cache anything else from the theme colors connect
 * to Changed().
 *
 * \sa QmitkColoredNodeDescriptor
 */
class MITKQTICONTHEME_EXPORT QmitkIconTheme : public QObject
{
  Q_OBJECT

public:
  /**
   * \brief Returns the single instance, which exists to emit Changed().
   */
  static QmitkIconTheme *GetInstance();

  /**
   * \brief Creates a theme-colored icon from raw SVG data.
   * \param[in] originalSVG The original SVG content as a byte array.
   * \return A QIcon that renders in the icon colors of the current theme.
   */
  static QIcon GetIcon(const QByteArray &originalSVG);

  /**
   * \brief Creates an icon from raw SVG data with a custom icon color.
   *
   * The magic icon color is replaced by \p color instead of the icon color
   * of the current theme, while the magic accent color still follows the
   * theme.
   *
   * \param[in] originalSVG The original SVG content as a byte array.
   * \param[in] color A CSS-compatible color string (e.g., "#ff0000").
   * \return A QIcon that renders the SVG in the given icon color.
   */
  static QIcon GetIcon(const QByteArray &originalSVG, const QString &color);

  /**
   * \brief Creates a theme-colored icon from an SVG resource file.
   * \param[in] resourcePath The Qt resource path to the SVG file.
   * \return A QIcon that renders in the icon colors of the current theme,
   *         or a null icon if the resource cannot be read.
   */
  static QIcon GetIcon(const QString &resourcePath);

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

  /**
   * \brief Re-reads the icon colors from the application style sheet.
   *
   * All icons created by GetIcon() re-render in the new colors on their
   * next repaint, and Changed() is emitted. Call this right after
   * QApplication::setStyleSheet().
   */
  static void Refresh();

Q_SIGNALS:
  /**
   * \brief Emitted by Refresh() after the icon colors have been re-read.
   */
  void Changed();

private:
  QmitkIconTheme();
  ~QmitkIconTheme() override;
};

#endif
