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
