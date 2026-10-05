/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpUiUtils_h
#define QmitkCrashDumpUiUtils_h

#include <mitkCrashDumpFacility.h>

#include <QColor>
#include <QString>

#include <filesystem>
#include <vector>

class QWidget;

/** Wording and actions shared by the crash-dump dialogs. */
namespace QmitkCrashDumpUi
{
  QString ToQString(const std::filesystem::path& path);

  QString KindLabel(mitk::DumpKind kind);

  /** Colours of the stylesheet's "font.warning" and "font.highlight" classes,
   *  so the dialogs follow the light and dark theme; palette-based fallbacks
   *  when no MITK stylesheet is active. */
  QColor WarningColor();
  QColor AccentColor();

  /** \p text as rich text in the warning colour, bold. */
  QString Warning(const QString& text);

  /** \p text as a rich-text heading or paragraph, with the line spacing of
   *  MITK's other message boxes. */
  QString Heading(const QString& text);
  QString Paragraph(const QString& text);

  /** What to consider before passing a dump on, as a heading and
   *  paragraphs of rich text. With \p pointToManager, it says where kept
   *  dumps can be found, which the manager itself has no need for. */
  QString PrivacyNote(bool pointToManager = true);

  /** The band at the top of the crash-dump dialogs: the application icon
   *  with an accent-coloured badge marking an exceptional state, a title as
   *  a heading, and a subtitle, which is rich text. */
  QWidget* CreateHeader(const QString& title, const QString& subtitle);

  /** Opens each distinct folder containing one of \p files. */
  void ShowInFolders(const std::vector<std::filesystem::path>& files);

  /** Hands \p dumps to the registered report service, if there is one.
   *  Returns whether a service took them. */
  bool FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent);
}

#endif
