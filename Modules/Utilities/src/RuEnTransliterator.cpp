#include "RuEnTransliterator.h"

const std::map<QString, QString> RuEnTransliterator::CHAR_TRANSFORM = {
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "a" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "b" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "v" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "g" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "d" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "e" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "yo" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "zh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "z" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "i" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "j" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "k" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "l" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "m" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "n" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "o" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "p" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "r" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "s" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "t" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "u" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "f" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "x" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "cz" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "ch" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "sh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "shh" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "``" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "y'" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "`" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "e'" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "yu" },
  { QTextCodec::codecForName("Windows-1251")->toUnicode("�"), "ya" }
};
const QChar RuEnTransliterator::CHAR_C_CZ = '�';
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

