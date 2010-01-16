/*=========================================================================
 
 Program:   BlueBerry Platform
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


#ifndef BERRYIQTSTYLEMANAGER_H_
#define BERRYIQTSTYLEMANAGER_H_

#include <berryService.h>

#include <QString>
#include <QList>

#include <Poco/Exception.h>

#include "berryUiQtDll.h"

namespace berry {

struct BERRY_UI_QT IQtStyleManager : public Service
{

  berryInterfaceMacro(IQtStyleManager, berry)

  struct Style {
    QString name;
    QString fileName;

    Style() {}

    Style(const QString& name, const QString& fn)
    : name(name), fileName(fn) {}

    Style& operator=(const Style& s)
    {
      this->name = s.name;
      this->fileName = s.fileName;
      return *this;
    }

    bool operator<(const Style& s) const
    { return name < s.name; }

    bool operator==(const Style& s) const
    { return name == s.name; }
  };

  static const std::string ID; // = "org.blueberry.service.qtstylemanager";

  typedef QList<Style> StyleList;

  virtual Style GetStyle() const = 0;
  virtual QString GetStylesheet() const = 0;
  virtual QString GetActiveTabStylesheet() const = 0;
  virtual QString GetTabStylesheet() const = 0;

  virtual void AddStyle(const QString& styleFileName, const QString& styleName = QString()) = 0;
  virtual void AddStyles(const QString& path) = 0;
  virtual void RemoveStyle(const QString& styleFileName) = 0;
  virtual void RemoveStyles(const QString& repository = QString()) = 0;

  virtual void GetStyles(StyleList& styles) const = 0;
  virtual void SetStyle(const QString& fileName) = 0;

  virtual Style GetDefaultStyle() const = 0;
  virtual void SetDefaultStyle() = 0;

  virtual bool Contains(const QString& fileName) const = 0;

};

}

#endif /* BERRYIQTSTYLEMANAGER_H_ */
