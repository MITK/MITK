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

#include "berryHelpEditorFindWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>

namespace berry {

HelpEditorFindWidget::HelpEditorFindWidget(QWidget *parent)
  : QWidget(parent)
  , appPalette(qApp->palette())
{
  installEventFilter(this);
  QHBoxLayout *hboxLayout = new QHBoxLayout(this);
  QString resourcePath = QLatin1String(":/org.blueberry.ui.qt.help");

#ifndef Q_OS_MAC
  hboxLayout->setMargin(0);
  hboxLayout->setSpacing(6);
#endif

  toolClose = setupToolButton(QLatin1String(""),
                              resourcePath + QLatin1String("/close.png"));
  hboxLayout->addWidget(toolClose);
  connect(toolClose, SIGNAL(clicked()), SLOT(hide()));

  editFind = new QLineEdit(this);
  hboxLayout->addWidget(editFind);
  editFind->setMinimumSize(QSize(150, 0));
  connect(editFind, SIGNAL(textChanged(QString)), this,
          SLOT(textChanged(QString)));
  connect(editFind, SIGNAL(returnPressed()), this, SIGNAL(findNext()));
  connect(editFind, SIGNAL(textChanged(QString)), this, SLOT(updateButtons()));

  toolPrevious = setupToolButton(tr("Previous"),
                                 resourcePath + QLatin1String("/go-previous.png"));
  connect(toolPrevious, SIGNAL(clicked()), this, SIGNAL(findPrevious()));

  hboxLayout->addWidget(toolPrevious);

  toolNext = setupToolButton(tr("Next"),
                             resourcePath + QLatin1String("/go-next.png"));
  hboxLayout->addWidget(toolNext);
  connect(toolNext, SIGNAL(clicked()), this, SIGNAL(findNext()));

  checkCase = new QCheckBox(tr("Case Sensitive"), this);
  hboxLayout->addWidget(checkCase);

  setMinimumWidth(minimumSizeHint().width());

  updateButtons();
}

HelpEditorFindWidget::~HelpEditorFindWidget()
{
}

void HelpEditorFindWidget::show()
{
  QWidget::show();
  editFind->selectAll();
  editFind->setFocus(Qt::ShortcutFocusReason);
}

void HelpEditorFindWidget::showAndClear()
{
  show();
  editFind->clear();
}

QString HelpEditorFindWidget::text() const
{
  return editFind->text();
}

bool HelpEditorFindWidget::caseSensitive() const
{
  return checkCase->isChecked();
}

void HelpEditorFindWidget::setPalette(bool found)
{
  QPalette palette = editFind->palette();
  palette.setColor(QPalette::Active, QPalette::Base, found ? Qt::white
                                                           : QColor(255, 102, 102));
  editFind->setPalette(palette);
}

void HelpEditorFindWidget::hideEvent(QHideEvent* event)
{
  // TODO: remove this once webkit supports setting the palette
  if (!event->spontaneous())
    qApp->setPalette(appPalette);
}

void HelpEditorFindWidget::showEvent(QShowEvent* event)
{
  // TODO: remove this once webkit supports setting the palette
  if (!event->spontaneous())
  {
    QPalette p = appPalette;
    p.setColor(QPalette::Inactive, QPalette::Highlight,
               p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText,
               p.color(QPalette::Active, QPalette::HighlightedText));
    qApp->setPalette(p);
  }
}

void HelpEditorFindWidget::updateButtons()
{
  const bool enable = !editFind->text().isEmpty();
  toolNext->setEnabled(enable);
  toolPrevious->setEnabled(enable);
}

void HelpEditorFindWidget::textChanged(const QString &text)
{
  emit find(text, true);
}

bool HelpEditorFindWidget::eventFilter(QObject *object, QEvent *e)
{
  if (e->type() == QEvent::KeyPress)
  {
    if ((static_cast<QKeyEvent*>(e))->key() == Qt::Key_Escape)
    {
      hide();
      emit escapePressed();
    }
  }
  return QWidget::eventFilter(object, e);
}

QToolButton* HelpEditorFindWidget::setupToolButton(const QString &text, const QString &icon)
{
  QToolButton *toolButton = new QToolButton(this);

  toolButton->setText(text);
  toolButton->setAutoRaise(true);
  toolButton->setIcon(QIcon(icon));
  toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  return toolButton;
}

}
