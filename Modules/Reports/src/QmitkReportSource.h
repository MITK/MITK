/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkReportSource_h
#define QmitkReportSource_h

#include <QRegularExpression>
#include <QString>

namespace QmitkReportSource
{
  /** src attributes, with the quote in group 1 and the value in group 2. */
  inline const QRegularExpression& GetAttribute()
  {
    static const QRegularExpression expression(QStringLiteral("\\bsrc\\s*=\\s*([\"'])(.*?)\\1"),
      QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);

    return expression;
  }

  inline bool IsDataUri(const QString& source)
  {
    return source.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive);
  }
}

#endif
