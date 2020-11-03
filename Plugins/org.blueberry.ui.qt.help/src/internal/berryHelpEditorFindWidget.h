/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  HelpEditorFindWidget(QWidget *parent = nullptr);
  ~HelpEditorFindWidget() override;

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

  void hideEvent(QHideEvent* event) override;
  void showEvent(QShowEvent * event) override;

private Q_SLOTS:

  void updateButtons();
  void textChanged(const QString &text);

private:

  bool eventFilter(QObject *object, QEvent *e) override;
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
