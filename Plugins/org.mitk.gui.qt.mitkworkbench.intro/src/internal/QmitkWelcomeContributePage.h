/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkWelcomeContributePage_h
#define QmitkWelcomeContributePage_h

#include "QmitkWelcomePalette.h"

#include <QWidget>

#include <utility>
#include <vector>

class QLabel;

/**
 * \brief Welcome screen page that invites users to contribute to MITK.
 *
 * Each card names one way to contribute and leads to it: reporting a
 * problem or an idea, sharing saved crash data (only in builds with crash
 * handling), contributing code or documentation, and citing MITK.
 */
class QmitkWelcomeContributePage : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkWelcomeContributePage(const QmitkWelcomePalette& palette, QWidget* parent = nullptr);
  ~QmitkWelcomeContributePage() override;

  void SetPalette(const QmitkWelcomePalette& palette);

private:
  struct Link
  {
    QString Url;
    QString Text;
  };

  /** A card with the pictogram at resource \p path. */
  QWidget* CreateCard(const QString& path, const QString& title, const QString& text, QWidget* action);
  QWidget* CreateButton(const QString& text, void (QmitkWelcomeContributePage::*onClicked)());
  QWidget* CreateLinks(const std::vector<Link>& links);
  QString CreateLinksText(const std::vector<Link>& links) const;

  void OnSendFeedback();
  void OnShowDiagnosticData();

  QmitkWelcomePalette m_Palette;

  /** Labels showing a pictogram, with its resource path. A label keeps a
   *  rendered pixmap, so a theme change has to render it again. */
  std::vector<std::pair<QLabel*, QString>> m_Pictograms;

  /** Labels showing links, which are colored by an inline span. */
  std::vector<std::pair<QLabel*, std::vector<Link>>> m_Links;
};

#endif
