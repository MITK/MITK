/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryQtStyleManager.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDirIterator>

#include <cherryLog.h>
#include <cherryPlatform.h>
#include <cherryIPreferencesService.h>
#include <cherryIPreferences.h>

#include "../cherryQtPreferences.h"

namespace cherry
{

bool QtStyleManager::IsA(const std::type_info& type) const
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info& QtStyleManager::GetType() const
{
  return typeid(cherry::IQtStyleManager);
}

QtStyleManager::QtStyleManager()
{
  AddDefaultStyle();
  ReadPreferences();
}

void QtStyleManager::ReadPreferences()
{
  IPreferencesService::Pointer prefService
    = Platform::GetServiceRegistry()
    .GetServiceById<IPreferencesService>(IPreferencesService::ID);

  IPreferences::Pointer stylePref = prefService->GetSystemPreferences()->Node(QtPreferences::QT_STYLES_NODE);

  QString paths = QString::fromStdString(stylePref->Get(QtPreferences::QT_STYLE_SEARCHPATHS, ""));
  QStringList pathList = paths.split(";", QString::SkipEmptyParts);
  QStringListIterator it(pathList);
  while (it.hasNext())
  {
    AddStyles(it.next());
  }
}

QtStyleManager::~QtStyleManager()
{
  for (FileNameToStyleMap::const_iterator i = styles.begin(); i != styles.end(); ++i)
  {
    delete i.value();
  }
}

void QtStyleManager::AddDefaultStyle()
{
  AddStyle(":/org.opencherry.ui.qt/defaultstyle.qss", "Default");
  defaultStyle = styles[":/org.opencherry.ui.qt/defaultstyle.qss"];
}

void QtStyleManager::ClearStyles()
{
  for (FileNameToStyleMap::const_iterator i = styles.begin(); i != styles.end(); ++i)
  {
    delete i.value();
  }
  styles.clear();
  AddDefaultStyle();
  SetDefaultStyle();
}

QtStyleManager::Style QtStyleManager::GetStyle() const
{
  return Style(currentStyle->name, currentStyle->fileName);
}

QString QtStyleManager::GetStylesheet() const
{
  return currentStyle->stylesheet;
}

QString QtStyleManager::GetActiveTabStylesheet() const
{
  return currentStyle->activeTabStylesheet;
}

QString QtStyleManager::GetTabStylesheet() const
{
  return currentStyle->tabStylesheet;
}

void QtStyleManager::AddStyle(const QString& styleFileName,
    const QString& styleName)
{
  ExtStyle* newStyle = new ExtStyle();

  if (styleName.isEmpty())
  {
    QFileInfo info(styleFileName);
    newStyle->name = info.completeBaseName();
  }
  else
  {
    newStyle->name = styleName;
  }

  newStyle->fileName = styleFileName;

  styles.insert(newStyle->fileName, newStyle);
}

void QtStyleManager::AddStyles(const QString& path)
{
  QDirIterator dirIt(path);
  while (dirIt.hasNext())
  {
    QString current = dirIt.next();
    QFileInfo info = dirIt.fileInfo();
    if (info.isFile() && info.isReadable())
    {
      QString fileName = info.fileName();
      if (fileName.endsWith("-tab.qss") || fileName.endsWith("-activetab.qss"))
        continue;

      if (fileName.endsWith(".qss"))
        AddStyle(current);
    }
  }
}

void QtStyleManager::ReadStyleData(ExtStyle* style)
{
  QString tabStyleFileName(style->fileName);
  QString activeTabStyleFileName(style->fileName);

  int index = style->fileName.lastIndexOf(".qss");
  tabStyleFileName.replace(index, 4, "-tab.qss");
  activeTabStyleFileName.replace(index, 4, "-activetab.qss");

  QFile styleFile(style->fileName);
  if (styleFile.open(QIODevice::ReadOnly))
  {
    QTextStream in(&styleFile);
    style->stylesheet = in.readAll();
  }
  else
  {
    CHERRY_WARN << "Could not read " << style->fileName.toStdString();
  }

  QFile tabStyleFile(tabStyleFileName);
  if (tabStyleFile.open(QIODevice::ReadOnly))
  {
    QTextStream in(&tabStyleFile);
    style->tabStylesheet = in.readAll();
  }
  else
  {
    CHERRY_WARN << "Could not read " << tabStyleFileName.toStdString();
  }

  QFile activeTabStyleFile(activeTabStyleFileName);
  if (activeTabStyleFile.open(QIODevice::ReadOnly))
  {
    QTextStream in(&activeTabStyleFile);
    style->activeTabStylesheet = in.readAll();
  }
  else
  {
    CHERRY_WARN << "Could not read " << activeTabStyleFileName.toStdString();
  }
}

void QtStyleManager::RemoveStyle(const QString& styleFileName)
{
  if (currentStyle->fileName == styleFileName)
  {
    SetDefaultStyle();
  }

  delete styles.take(styleFileName);
}

void QtStyleManager::RemoveStyles(const QString& repo)
{
  if (repo.isEmpty())
  {
    ClearStyles();
    return;
  }

  for (FileNameToStyleMap::iterator i = styles.begin(); i != styles.end();)
  {
    ExtStyle* style = i.value();
    QFileInfo info(style->fileName);
    if (info.absolutePath() == repo)
    {
      if (style->name == currentStyle->name)
      {
        SetDefaultStyle();
      }

      i = styles.erase(i);
      delete style;
    }
    else
    {
      ++i;
    }
  }
}

void QtStyleManager::GetStyles(StyleList& styleNames) const
{
  for (FileNameToStyleMap::const_iterator i = styles.begin(); i != styles.end(); ++i)
    styleNames.push_back(Style(i.value()->name, i.value()->fileName));
}

void QtStyleManager::SetStyle(const QString& fileName)
{
  if (fileName.isEmpty())
  {
    SetDefaultStyle();
    return;
  }

  FileNameToStyleMap::const_iterator i = styles.find(fileName);

  ExtStyle* style = 0;
  if (i == styles.end())
  {
    CHERRY_WARN << "Style " + fileName.toStdString() << " does not exist";
    style = defaultStyle;
  }
  else
  {
    style = i.value();
  }
  currentStyle = style;

  ReadStyleData(style);
  qApp->setStyleSheet(currentStyle->stylesheet);
}

QtStyleManager::Style QtStyleManager::GetDefaultStyle() const
{
  return Style(defaultStyle->name, defaultStyle->fileName);
}

void QtStyleManager::SetDefaultStyle()
{
  SetStyle(defaultStyle->fileName);
}

bool QtStyleManager::Contains(const QString& fileName) const
{
  return styles.contains(fileName);
}

}
