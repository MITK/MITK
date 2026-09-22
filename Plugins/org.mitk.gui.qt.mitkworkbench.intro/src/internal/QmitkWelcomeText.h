/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomeText_h
#define QmitkWelcomeText_h

#include <QString>

/**
 * \brief Wraps HTML into a single paragraph with the line height of the welcome screen.
 *
 * Meant for word-wrapped rich text labels, whose line spacing cannot be set
 * by style sheets.
 */
QString QmitkWelcomeTextWithLineHeight(QString html);

#endif
