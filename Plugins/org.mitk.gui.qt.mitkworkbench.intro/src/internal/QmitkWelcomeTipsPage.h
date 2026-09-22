/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomeTipsPage_h
#define QmitkWelcomeTipsPage_h

#include "QmitkWelcomePalette.h"

#include <QStringList>
#include <QWidget>

class QmitkHtmlWidget;

/**
 * \brief Welcome screen page that cycles through usage tips.
 *
 * Every HTML document in the tips resource folder is a tip. A random tip is
 * shown first, "Next tip" moves on to the next one.
 */
class QmitkWelcomeTipsPage : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkWelcomeTipsPage(const QmitkWelcomePalette& palette, QWidget* parent = nullptr);

  void SetPalette(const QmitkWelcomePalette& palette);

  /**
   * \brief Shows the tip of the given name, i.e., its file name without extension.
   *
   * The current tip is kept if there is no tip of that name.
   */
  void SelectTip(const QString& name);

private:
  QByteArray GetResource(const QUrl& url) const;
  void ShowTip(int index);

  QmitkWelcomePalette m_Palette;
  QmitkHtmlWidget* m_TipView;
  QStringList m_TipFiles;
  int m_CurrentTip;
};

#endif
