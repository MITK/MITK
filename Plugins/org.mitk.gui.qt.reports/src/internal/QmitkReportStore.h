/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportStore_h
#define QmitkReportStore_h

#include <QmitkReport.h>
#include <QmitkReportImagePool.h>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

#include <deque>
#include <map>
#include <memory>
#include <vector>

class QLockFile;
class QTemporaryDir;
class QWidget;

/**
 * \brief The reports of the session and the images they show.
 *
 * A report is the body of an HTML page; GetDocument() puts it into a page
 * with the reports' style sheet and the title. Screenshots are kept in a pool
 * under names like "image-3.png" first. A report refers to them by name in
 * quoted src attributes and keeps a copy of each, so it stays complete after
 * the pool dropped an image. A client's own figures may be embedded as data:
 * URIs; images from anywhere else cannot be shown. The REST routes refuse
 * reports of more than 32 MB.
 *
 * A report counts as saved once SaveReport() wrote it, until it changes. The
 * store asks the user before it discards an unsaved report whose editor was
 * closed, and before the Workbench shuts down with unsaved reports.
 *
 * Use it in the GUI thread only.
 */
class QmitkReportStore : public QObject
{
  Q_OBJECT

public:
  using Report = QmitkReport;

  explicit QmitkReportStore(QObject* parent = nullptr);
  ~QmitkReportStore() override;

  /** \brief Keep a PNG image for reports to show, and return its name. */
  QString AddImage(const QByteArray& png);

  /**
   * \brief Add a report and return its id.
   *
   * \param title Stored with its white space simplified and cut to 200 characters.
   * \param html The body of the page.
   * \throw mitk::Exception if the HTML refers to images the pool does not have.
   */
  int Add(const QString& title, const QString& html);

  /**
   * \brief Replace the title and body of a report.
   *
   * The title is stored as Add() does.
   *
   * \throw mitk::Exception if there is no such report, or if the HTML refers
   *        to images that neither the report nor the pool has.
   */
  void Update(int id, const QString& title, const QString& html);

  /** \brief The report, or nullptr if there is none with this id. */
  const Report* Get(int id) const;

  std::vector<const Report*> GetReports() const;

  /** \brief Remove the report and the temporary files written for it. */
  void Remove(int id);

  /** \brief Whether the report was saved since it was added or last changed. */
  bool IsSaved(int id) const;

  /**
   * \brief Ask the user for a file and write the report to it.
   *
   * A written report counts as saved. If writing fails, the user is told.
   *
   * \return Whether the report was written; false if the user canceled the
   *         file dialog, the file could not be written, or there is no such report.
   */
  bool SaveReport(int id, QWidget* parent);

  /**
   * \brief Record that the user chose to give up the report without saving it.
   *
   * No question follows when its editor closes or the Workbench shuts down,
   * until the report changes.
   */
  void AcceptLoss(int id);

  /**
   * \brief Tell the store that the report's editor was closed.
   *
   * A saved report, or one the user gave up, is removed at once. For any other,
   * the user is asked once the close has completed whether to save or discard
   * it, as editors do not always ask before they close; the report is removed
   * afterwards.
   */
  void CloseReport(int id);

  /**
   * \brief The report as a complete HTML page, or an empty string if there is
   *        none with this id.
   *
   * \param forExport Whether the page leaves MITK. Its images are then part
   *        of the page, and a content security policy keeps browsers from
   *        running scripts or loading anything from elsewhere.
   */
  QString GetDocument(int id, bool forExport) const;

  /**
   * \brief Write the exported page to a file.
   *
   * \throw mitk::Exception if there is no such report or the file cannot be written.
   */
  void WriteDocument(int id, const QString& path) const;

  /**
   * \brief Write the exported page into a folder that lives as long as the
   *        store, for programs like web browsers to open. Remove() deletes
   *        the file again.
   *
   * The folder is locked while the store lives. When the store creates it,
   * it first removes the folders that earlier sessions left behind.
   *
   * \return The path of the file.
   * \throw mitk::Exception if the file cannot be written.
   */
  QString WriteTemporaryFile(int id);

Q_SIGNALS:
  void Changed(int id);

private:
  class WorkbenchListener;

  void MarkSaved(int id);
  void AskAboutClosedReports();
  bool ConfirmShutdown(bool forced);
  void ListenToWorkbench();

  /** The ids of the reports that are neither saved nor given up by the user. */
  std::vector<int> GetUnsavedReports() const;

  std::map<int, Report> m_Reports;
  std::deque<int> m_ClosedReports;
  bool m_AskingQueued = false;
  bool m_Asking = false;
  std::unique_ptr<WorkbenchListener> m_WorkbenchListener;
  QmitkReportImagePool m_Pool;
  int m_NextReportId = 1;
  std::map<int, QStringList> m_TemporaryFiles;

  // The lock is declared after the folder so that it is released before the folder is removed.
  std::unique_ptr<QTemporaryDir> m_TemporaryDir;
  std::unique_ptr<QLockFile> m_Lock;
};

#endif
