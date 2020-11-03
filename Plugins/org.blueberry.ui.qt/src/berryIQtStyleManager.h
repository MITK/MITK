/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIQTSTYLEMANAGER_H_
#define BERRYIQTSTYLEMANAGER_H_

#include <QString>
#include <QList>
#include <QtPlugin>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct BERRY_UI_QT IQtStyleManager
{

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

  typedef QList<Style> StyleList;

  virtual ~IQtStyleManager();

  virtual Style GetStyle() const = 0;
  virtual QString GetStylesheet() const = 0;
  virtual QString GetActiveTabStylesheet() const = 0;
  virtual QString GetTabStylesheet() const = 0;
  virtual QString GetFont() const = 0;

  virtual void AddStyle(const QString& styleFileName, const QString& styleName = QString()) = 0;
  virtual void AddStyles(const QString& path) = 0;
  virtual void RemoveStyle(const QString& styleFileName) = 0;
  virtual void RemoveStyles(const QString& repository = QString()) = 0;

  virtual void GetStyles(StyleList& styles) const = 0;
  virtual void SetStyle(const QString& fileName) = 0;

  virtual void GetFonts(QStringList& fontNames) const = 0;
  virtual void SetFont(const QString& fontName) = 0;
  virtual void SetFontSize(const int fontSize) = 0;
  virtual void UpdateWorkbenchFont() = 0;

  virtual Style GetDefaultStyle() const = 0;
  virtual void SetDefaultStyle() = 0;

  virtual bool Contains(const QString& fileName) const = 0;

};

}

Q_DECLARE_INTERFACE(berry::IQtStyleManager, "org.blueberry.service.IQtStyleManager")

#endif /* BERRYIQTSTYLEMANAGER_H_ */
