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

#ifndef BERRYHELPEDITORFINDWIDGET_H
#define BERRYHELPEDITORFINDWIDGET_H

#include <QWidget>

class QCheckBox;
class QLabel;
class QLineEdit;
class QToolButton;


namespace berry {

class HelpEditorFindWidget : public QWidget
{
  Q_OBJECT

public:

  HelpEditorFindWidget(QWidget *parent = 0);
  ~HelpEditorFindWidget();

  void show();
  void showAndClear();

  QString text() const;
  bool caseSensitive() const;

  void setPalette(bool found);

Q_SIGNALS:

  void escapePressed();

  void findNext();
  void findPrevious();
  void find(const QString &text, bool forward);

protected:

  void hideEvent(QHideEvent* event);
  void showEvent(QShowEvent * event);

private Q_SLOTS:

  void updateButtons();
  void textChanged(const QString &text);

private:

  bool eventFilter(QObject *object, QEvent *e);
  QToolButton* setupToolButton(const QString &text, const QString &icon);

private:

  QPalette appPalette;

  QLineEdit *editFind;
  QCheckBox *checkCase;
  QToolButton *toolNext;
  QToolButton *toolClose;
  QToolButton *toolPrevious;

};

} // end namespace berry

#endif  // BERRYHELPEDITORFINDWIDGET_H
