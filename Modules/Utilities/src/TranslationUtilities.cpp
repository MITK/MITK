#include "TranslationUtilities.h"

#include <QObject>

namespace mitk {

std::vector<std::string> TranslationUtilities::getTranslatedTransferFunctionPresets()
{
  std::vector<std::string> names;
  names.push_back(QObject::tr("choose an internal transferfunction preset").toStdString());
  names.push_back(QObject::tr("CT Generic").toStdString());
  names.push_back(QObject::tr("CT Black & White").toStdString());
  names.push_back(QObject::tr("CT Thorax large").toStdString());
  names.push_back(QObject::tr("CT Thorax small").toStdString());
  names.push_back(QObject::tr("CT Bone").toStdString());
  names.push_back(QObject::tr("CT Bone (with Gradient)").toStdString());
  names.push_back(QObject::tr("CT Cardiac").toStdString());
  names.push_back(QObject::tr("MR Generic").toStdString());
  return names;
}

std::string TranslationUtilities::getDefaultUserPresetName()
{
  return QObject::tr("User Preset #").toStdString();
}

}