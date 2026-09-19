/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkColoredNodeDescriptor.h>
#include <QmitkIconTheme.h>
#include <mitkNodePredicateDataType.h>
#include <QFile>

static QString ConvertRGBTripletToHexColorCode(float rgbTriplet[3])
{
  return QString("#%1%2%3")
    .arg(std::max(0, std::min(255, static_cast<int>(rgbTriplet[0] * 255))), 2, 16, QLatin1Char('0'))
    .arg(std::max(0, std::min(255, static_cast<int>(rgbTriplet[1] * 255))), 2, 16, QLatin1Char('0'))
    .arg(std::max(0, std::min(255, static_cast<int>(rgbTriplet[2] * 255))), 2, 16, QLatin1Char('0'));
}

struct QmitkColoredNodeDescriptor::Impl
{
  void CreateCachedIcon(const QString &hexColorCode);

  QHash<QString, QIcon> IconCache;
  QByteArray IconTemplate;
};

void QmitkColoredNodeDescriptor::Impl::CreateCachedIcon(const QString &hexColorCode)
{
  this->IconCache[hexColorCode] = QmitkIconTheme::GetIcon(this->IconTemplate, hexColorCode);
}

QmitkColoredNodeDescriptor::QmitkColoredNodeDescriptor(const QString &className, const QString &pathToIcon, mitk::NodePredicateBase *predicate, QObject *parent)
  : QmitkNodeDescriptor(className, QStringLiteral(""), predicate, parent),
    m_Impl(new Impl)
{
  QFile iconTemplateFile(pathToIcon);

  if (iconTemplateFile.open(QIODevice::ReadOnly))
    m_Impl->IconTemplate = iconTemplateFile.readAll();
}

QmitkColoredNodeDescriptor::~QmitkColoredNodeDescriptor()
{
  delete m_Impl;
}

QIcon QmitkColoredNodeDescriptor::GetIcon(const mitk::DataNode *node) const
{
  if (nullptr == node)
    return QIcon();

  float rgbTriplet[] = { 1.0f, 1.0f, 1.0f };
  node->GetColor(rgbTriplet);

  auto hexColorCode = ConvertRGBTripletToHexColorCode(rgbTriplet);

  if (!m_Impl->IconCache.contains(hexColorCode))
    m_Impl->CreateCachedIcon(hexColorCode);

  return m_Impl->IconCache[hexColorCode];
}
