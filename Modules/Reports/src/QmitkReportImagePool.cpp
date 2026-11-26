/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportImagePool.h"

#include "QmitkReportSource.h"

#include <mitkExceptionMacro.h>

#include <QStringList>

#include <algorithm>

namespace
{
  constexpr std::size_t MaxPoolSize = 100;
}

QString QmitkReportImagePool::AddImage(const QByteArray& png)
{
  const auto name = QStringLiteral("image-%1.png").arg(m_NextImageId++);
  m_Images.emplace_back(name, png);

  while (m_Images.size() > MaxPoolSize)
    m_Images.pop_front();

  return name;
}

QMap<QString, QByteArray> QmitkReportImagePool::CollectImages(const QString& html, const QMap<QString, QByteArray>& ownImages) const
{
  QMap<QString, QByteArray> images;
  QStringList unknown;
  auto sources = QmitkReportSource::GetAttribute().globalMatch(html);

  while (sources.hasNext())
  {
    const auto source = sources.next().captured(2);

    // An empty src refers to no image.
    if (source.isEmpty() || QmitkReportSource::IsDataUri(source) || images.contains(source))
      continue;

    if (ownImages.contains(source))
    {
      images.insert(source, ownImages.value(source));
      continue;
    }

    const auto pooled = std::find_if(m_Images.begin(), m_Images.end(), [&source](const auto& image) { return image.first == source; });

    if (pooled != m_Images.end())
    {
      images.insert(source, pooled->second);
      continue;
    }

    if (!unknown.contains(source))
      unknown << source;
  }

  if (!unknown.isEmpty())
    mitkThrow() << "Unknown images: " << unknown.join(QStringLiteral(", ")).toStdString() << ".";

  return images;
}
