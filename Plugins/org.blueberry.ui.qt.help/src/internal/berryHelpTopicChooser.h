/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPTOPICCHOOSER_H
#define BERRYHELPTOPICCHOOSER_H

#include <ui_berryHelpTopicChooser.h>

#include <QList>
#include <QMap>
#include <QString>
#include <QUrl>

#include <QDialog>

namespace berry {

class HelpTopicChooser : public QDialog
{
  Q_OBJECT

public:
  HelpTopicChooser(QWidget *parent, const QString &keyword,
                   const QMap<QString, QUrl> &links);

  QUrl link() const;

private:
  Ui::HelpTopicChooser ui;
  QList<QUrl> m_links;
};

}

#endif // BERRYHELPTOPICCHOOSER_H
