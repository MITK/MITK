#pragma once

#include <set>

#include <QString>
#include <QTextStream>
#include <QTextCodec>

#include <MitkUtilitiesExports.h>

// GOST 7.79-2000 http://www.internet-law.ru/gosts/gost/6464/
class MITKUTILITIES_EXPORT RuEnTransliterator : public std::unary_function<QString, QString>
{
public:
  QString operator()(const QString& input) const;

private:
  QChar add(QTextStream& output, const QString& input, bool isLower) const;

  static const std::map<QString, QString> CHAR_TRANSFORM;
  static const QChar CHAR_C_CZ;
  static const std::set<QChar> CHAR_C_CZ_TRANSFORM;
};

