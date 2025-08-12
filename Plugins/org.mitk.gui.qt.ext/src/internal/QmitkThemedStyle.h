/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkThemedStyle_h
#define QmitkThemedStyle_h

#include <QProxyStyle>

/**
 * \brief Applies the application's icon theme to Qt's standard icons.
 *
 * When an icon theme is set, Qt uses our icons in certain places (e.g., message
 * boxes). However, our standard icons are SVG templates with placeholder colors,
 * meant to be tinted according to the application's style (dark/light). They
 * should not be used directly, as the placeholders would appear as green and
 * magenta.
 *
 * This proxy style intercepts standard icon requests and applies the correct
 * theme colors before returning them, ensuring Qt's built-in icons match the
 * application's theme.
 */
class QmitkThemedStyle : public QProxyStyle
{
public:
  using QProxyStyle::QProxyStyle;

  QIcon standardIcon(StandardPixmap sp, const QStyleOption* opt, const QWidget* widget) const override;
};

#endif
