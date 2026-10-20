/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportStore_h
#define QmitkReportStore_h

#include <QByteArray>
#include <QDateTime>
#include <QMap>
#include <QObject>
#include <QString>

#include <deque>
#include <map>
#include <memory>
#include <utility>
#include <vector>

class QTemporaryDir;

/**
 * \brief The reports of the session and the images they show.
 *
 * A report is the body of an HTML page; GetDocument() puts it into a page
 * with the reports' style sheet and the title. Images are kept in a pool
 * under names like "image-3.png" first. A report refers to them by name in
 * src attributes and keeps a copy of each, so it stays complete after the
 * pool dropped an image. Images from anywhere else cannot be shown.
 *
 * Use it in the GUI thread only.
 */
class QmitkReportStore : public QObject
{
  Q_OBJECT

public:
  struct Report
  {
    int Id = 0;
    QString Title;
    QString Html;
    QMap<QString, QByteArray> Images;
    QDateTime Modified;
  };

  explicit QmitkReportStore(QObject* parent = nullptr);
  ~QmitkReportStore() override;

  /** \brief Keep a PNG image for reports to show, and return its name. */
  QString AddImage(const QByteArray& png);

  /**
   * \brief Add a report and return its id.
   *
   * \param html The body of the page.
   * \throw mitk::Exception if the HTML refers to images the pool does not have.
   */
  int Add(const QString& title, const QString& html);

  /**
   * \brief Replace the title and body of a report.
   *
   * \throw mitk::Exception if there is no such report, or if the HTML refers
   *        to images that neither the report nor the pool has.
   */
  void Update(int id, const QString& title, const QString& html);

  /** \brief The report, or nullptr if there is none with this id. */
  const Report* Get(int id) const;

  std::vector<const Report*> GetReports() const;

  void Remove(int id);

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
   *        store, for programs like web browsers to open.
   *
   * \return The path of the file.
   * \throw mitk::Exception if the file cannot be written.
   */
  QString WriteTemporaryFile(int id);

  /** \brief A file name for the report, without folder and suffix. */
  static QString GetFileBaseName(const Report& report);

Q_SIGNALS:
  void Changed(int id);

private:
  QMap<QString, QByteArray> CollectImages(const QString& html, const Report* report) const;

  std::map<int, Report> m_Reports;
  std::deque<std::pair<QString, QByteArray>> m_Pool;
  int m_NextReportId = 1;
  int m_NextImageId = 1;
  std::unique_ptr<QTemporaryDir> m_TemporaryDir;
};

#endif
