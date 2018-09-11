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

#include <berryIQtStyleManager.h>
#include <org_blueberry_ui_qt_Export.h>

#include <QHash>

namespace berry {

class BERRY_UI_QT QtStyleManager : public QObject, public IQtStyleManager
{
  Q_OBJECT
  Q_INTERFACES(berry::IQtStyleManager)

public:
  static QIcon ThemeIcon(const QByteArray &originalSVG);
  static QIcon ThemeIcon(const QString &resourcePath);

  QtStyleManager();
  ~QtStyleManager() override;

  Style GetStyle() const override;
  QString GetStylesheet() const override;
  QString GetActiveTabStylesheet() const override;
  QString GetTabStylesheet() const override;
  QString GetFont() const override;

  void AddStyle(const QString& styleFileName,
                const QString& styleName = QString()) override;
  void AddStyles(const QString& path) override;
  void RemoveStyle(const QString& styleFileName) override;
  void RemoveStyles(const QString& path = QString()) override;

  void GetStyles(StyleList& styles) const override;
  void SetStyle(const QString& fileName) override;

  void GetFonts(QStringList& fontNames) const override;

  void SetFont(const QString& fontName) override;
  void SetFontSize(const int fontSize) override;
  void UpdateWorkbenchFont() override;

  Style GetDefaultStyle() const override;
  void SetDefaultStyle() override;

  bool Contains(const QString& fileName) const override;

  bool IsA( const std::type_info& type ) const;
  const std::type_info& GetType() const;

private:

  void AddDefaultStyle();
  void AddDefaultFonts();
  void ClearStyles();
  void ReadPreferences();

  void SetStyle(const QString& fileName, bool update);
  void SetDefaultStyle(bool update);

  struct ExtStyle : public Style
  {
    QString stylesheet;
    QString tabStylesheet;
    QString activeTabStylesheet;
  };

  QStringList m_customFontNames;
  void ReadStyleData(ExtStyle* style);

  typedef QHash<QString, ExtStyle*> FileNameToStyleMap;
  FileNameToStyleMap styles;

  QString m_currentFont;
  int m_currentFontSize;
  ExtStyle const* currentStyle;
  ExtStyle* defaultStyle;

};
}

#endif /* BERRYQTSTYLEMANAGER_H_ */
