/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReport_h
#define QmitkReport_h

#include <MitkReportsExports.h>

#include <QByteArray>
#include <QDateTime>
#include <QMap>
#include <QString>

/**
 * \brief A report: the body of an HTML page with a title and the images it shows.
 *
 * GetDocument() puts the body into a complete page with the reports' style
 * sheet. The body refers to its images by name in quoted src attributes; Images
 * holds the PNG data under these names. A client's own figures may be embedded
 * as data: URIs; images from anywhere else cannot be shown.
 */
struct MITKREPORTS_EXPORT QmitkReport
{
  int Id = 0;
  QString Title;
  QString Html;
  QMap<QString, QByteArray> Images;
  QDateTime Modified;
  bool Saved = false;
  bool LossAccepted = false;

  /** \brief A title as reports store it: white space simplified and cut to 200 characters. */
  static QString NormalizeTitle(const QString& title);

  /** \brief A file name for the report, without folder and suffix; never empty. */
  QString GetFileBaseName() const;

  /**
   * \brief The report as a complete HTML page.
   *
   * \param forExport Whether the page leaves MITK. The images named in the body
   *        are then part of the page, <meta>, <base> and <form> are shown as
   *        text, and a content security policy keeps browsers from running
   *        scripts or loading anything from elsewhere.
   * \param darkTheme Whether the page uses the dark style.
   */
  QString GetDocument(bool forExport, bool darkTheme) const;

  /**
   * \brief Write the exported page to a file; the file is replaced as a whole or not at all.
   *
   * \throw mitk::Exception if the file cannot be written.
   */
  void WriteDocument(const QString& path, bool darkTheme) const;
};

#endif
