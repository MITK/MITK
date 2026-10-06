/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomeCard_h
#define QmitkWelcomeCard_h

#include <QString>

#include <utility>

class QLabel;
class QVBoxLayout;
class QWidget;

/**
 * \brief Creates a welcome screen card with the given title.
 *
 * With a \p pictogram, the card shows it to the left of the title and of
 * everything added to the returned layout.
 *
 * \return The card and the layout to add its content to, below the title.
 */
std::pair<QWidget*, QVBoxLayout*> QmitkWelcomeCreateCard(const QString& title, QWidget* pictogram = nullptr);

/**
 * \brief Creates a word-wrapped rich text label with the welcome screen's line height.
 */
QLabel* QmitkWelcomeCreateText(const QString& html = QString());

/**
 * \brief Like QmitkWelcomeCreateText(), in the muted text color.
 */
QLabel* QmitkWelcomeCreateDetailText(const QString& html = QString());

#endif
