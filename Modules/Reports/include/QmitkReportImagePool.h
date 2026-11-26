/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportImagePool_h
#define QmitkReportImagePool_h

#include <MitkReportsExports.h>

#include <QByteArray>
#include <QMap>
#include <QString>

#include <deque>
#include <utility>

/**
 * \brief The most recent PNG images that reports can refer to by name.
 *
 * The pool keeps at most 100 images; adding more drops the oldest. A report
 * copies the images it uses, so it stays complete after the pool dropped them.
 */
class MITKREPORTS_EXPORT QmitkReportImagePool
{
public:
  /** \brief Keep a PNG image and return its name, like "image-3.png". Names are not reused. */
  QString AddImage(const QByteArray& png);

  /**
   * \brief The images that the quoted src attributes of the HTML refer to.
   *
   * data: URIs are not images of the pool and are skipped. A name is taken from
   * ownImages first, then from the pool.
   *
   * \param ownImages The images a report has kept already.
   * \return The images by name.
   * \throw mitk::Exception if there are names that neither ownImages nor the pool has.
   */
  QMap<QString, QByteArray> CollectImages(const QString& html, const QMap<QString, QByteArray>& ownImages = {}) const;

private:
  std::deque<std::pair<QString, QByteArray>> m_Images;
  int m_NextImageId = 1;
};

#endif
