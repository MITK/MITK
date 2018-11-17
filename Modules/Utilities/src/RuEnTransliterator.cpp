#include "RuEnTransliterator.h"

const std::map<QString, QString> RuEnTransliterator::CHAR_TRANSFORM = {
  { QTextCodec::codecForName("Windows-1251")->toUnicode("à"), "a" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("á"), "b" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("â"), "v" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ã"), "g" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ä"), "d" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("å"), "e" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("¸"), "yo" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("æ"), "zh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ç"), "z" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("è"), "i" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("é"), "j" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ê"), "k" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ë"), "l" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ì"), "m" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("í"), "n" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("î"), "o" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ï"), "p" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ð"), "r" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ñ"), "s" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ò"), "t" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ó"), "u" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ô"), "f" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("õ"), "x" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ö"), "cz" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("÷"), "ch" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ø"), "sh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ù"), "shh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ú"), "``" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("û"), "y'" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ü"), "`" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ý"), "e'" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("þ"), "yu" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("ÿ"), "ya" }
};
const QChar RuEnTransliterator::CHAR_C_CZ = 'ö';
const std::set<QChar> RuEnTransliterator::CHAR_C_CZ_TRANSFORM = {
  'i', 'e', 'y', 'j'
};

QString RuEnTransliterator::operator()(const QString& input) const
{
  if (input.isEmpty()) {
    return QString();
  }
  QString result;
  QTextStream output(&result);
  QChar prevChar;
  for (auto iter = input.data() + input.length() - 1; iter >= input.data(); --iter) {
    const auto inputChar = iter->toLower();
    const auto it = CHAR_TRANSFORM.find(inputChar);
    if (CHAR_TRANSFORM.end() == it) {
      prevChar = inputChar;
      output << *iter;
    } else {
      if (CHAR_C_CZ == inputChar && CHAR_C_CZ_TRANSFORM.end() != CHAR_C_CZ_TRANSFORM.find(prevChar)) {
        prevChar = add(output, "c", iter->isLower());
      } else {
        prevChar = add(output, it->second, iter->isLower());
      }
    }
  }
  std::reverse(result.begin(), result.end());
  return result;
}

QChar RuEnTransliterator::add(QTextStream& output, const QString& input, bool isLower) const
{
  for (auto i = input.length() - 1; i >= 0; --i) {
    if (i || isLower) {
      output << input[i];
    } else {
      output << input[0].toUpper();
    }
  }
  return input[0];
}

