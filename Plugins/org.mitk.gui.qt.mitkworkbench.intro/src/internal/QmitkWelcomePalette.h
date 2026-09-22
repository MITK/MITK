/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomePalette_h
#define QmitkWelcomePalette_h

#include <QColor>

/**
 * \brief Colors of the welcome screen for the current application theme.
 *
 * The welcome screen paints its own panels instead of inheriting the theme's
 * flat widget background, so it needs a matching set of colors per theme.
 * Illustrations use the magic color \#00ff00 (as MitkQtIconTheme icons do)
 * wherever they need the text color.
 */
struct QmitkWelcomePalette
{
  QColor Page;
  QColor Card;
  QColor Cell;
  QColor Text;
  QColor MutedText;
  QColor Accent;

  /**
   * \brief Returns the dark palette for the dark theme, the light palette otherwise.
   */
  static QmitkWelcomePalette GetCurrent();
};

#endif
