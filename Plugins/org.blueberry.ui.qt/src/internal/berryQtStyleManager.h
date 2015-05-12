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

  Style GetStyle() const override;
  QString GetStylesheet() const override;
  QString GetActiveTabStylesheet() const override;
  QString GetTabStylesheet() const override;

  void AddStyle(const QString& styleFileName, const QString& styleName = QString()) override;
  void AddStyles(const QString& path) override;
  void RemoveStyle(const QString& styleFileName) override;
  void RemoveStyles(const QString& path = QString()) override;

  void GetStyles(StyleList& styles) const override;
  void SetStyle(const QString& fileName) override;

  Style GetDefaultStyle() const override;
  void SetDefaultStyle() override;

  void GetIconThemes(IconThemeList& iconThemes) const override;

  void SetIconTheme(const QString& themeName) override;

  bool Contains(const QString& fileName) const override;

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
