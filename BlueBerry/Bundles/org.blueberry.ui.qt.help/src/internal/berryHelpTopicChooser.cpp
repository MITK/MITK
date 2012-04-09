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

#include "berryHelpTopicChooser.h"

namespace berry {

HelpTopicChooser::HelpTopicChooser(QWidget *parent, const QString &keyword,
                                   const QMap<QString, QUrl> &links)
  : QDialog(parent)
{
  ui.setupUi(this);
  ui.label->setText(tr("Choose a topic for <b>%1</b>:").arg(keyword));

  QMap<QString, QUrl>::const_iterator it = links.constBegin();
  for (; it != links.constEnd(); ++it)
  {
    ui.listWidget->addItem(it.key());
    m_links.append(it.value());
  }

  if (ui.listWidget->count() != 0)
    ui.listWidget->setCurrentRow(0);
  ui.listWidget->setFocus();

  connect(ui.buttonDisplay, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(ui.listWidget, SIGNAL(itemActivated(QListWidgetItem*)), this,
          SLOT(accept()));
}

QUrl HelpTopicChooser::link() const
{
  QListWidgetItem *item = ui.listWidget->currentItem();
  if (!item)
    return QUrl();

  QString title = item->text();
  if (title.isEmpty())
    return QUrl();

  const int row = ui.listWidget->row(item);
  Q_ASSERT(row < m_links.count());
  return m_links.at(row);
}

}
