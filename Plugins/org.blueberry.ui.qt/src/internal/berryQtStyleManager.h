/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYQTSTYLEMANAGER_H_
#define BERRYQTSTYLEMANAGER_H_

#include "berryIQtStyleManager.h"

#include <QHash>

namespace berry {

class QtStyleManager : public QObject, public IQtStyleManager
{
  Q_OBJECT
  Q_INTERFACES(berry::IQtStyleManager)

public:

  QtStyleManager();
  ~QtStyleManager();

  Style GetStyle() const;
  QString GetStylesheet() const;
  QString GetActiveTabStylesheet() const;
  QString GetTabStylesheet() const;

  void AddStyle(const QString& styleFileName, const QString& styleName = QString());
  void AddStyles(const QString& path);
  void RemoveStyle(const QString& styleFileName);
  void RemoveStyles(const QString& path = QString());

  void GetStyles(StyleList& styles) const;
  void SetStyle(const QString& fileName);

  Style GetDefaultStyle() const;
  void SetDefaultStyle();

  void GetIconThemes(IconThemeList& iconThemes) const;

  void SetIconTheme(const QString& themeName);

  bool Contains(const QString& fileName) const;

  bool IsA( const std::type_info& type ) const;
  const std::type_info& GetType() const;

private:

  void AddDefaultStyle();
  void ClearStyles();
  void ReadPreferences();

  void SetStyle(const QString& fileName, bool update);
  void SetDefaultStyle(bool update);

  void SetIconTheme(const QString& themeName, bool update);

  struct ExtStyle : public Style
  {
    QString stylesheet;
    QString tabStylesheet;
    QString activeTabStylesheet;
  };

  void ReadStyleData(ExtStyle* style);

  typedef QHash<QString, ExtStyle*> FileNameToStyleMap;
  FileNameToStyleMap styles;

  ExtStyle const* currentStyle;
  ExtStyle* defaultStyle;

};
}

#endif /* BERRYQTSTYLEMANAGER_H_ */
