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

class QLabel;

/**
 * \brief Wraps HTML into a single paragraph with the line height of the welcome screen.
 *
 * Meant for word-wrapped rich text labels, whose line spacing cannot be set
 * by style sheets.
 */
QString QmitkWelcomeTextWithLineHeight(QString html);

/**
 * \brief Shows the target of the link under the mouse as the tooltip of \p label.
 *
 * Link texts name what they lead to, not where; the tooltip lets users see
 * the address before they open it in their browser.
 */
void QmitkWelcomeShowLinkTargets(QLabel* label);

#endif
