/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-09-14 18:57:01 +0200 (Fr, 14 Sep 2007) $
Version:   $Revision: 12077 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkPluginListViewItem_h_included
#define QmitkPluginListViewItem_h_included

#include <qlistview.h>

class QmitkPluginListViewItem : public QListViewItem
{

    public:

      QmitkPluginListViewItem( QString ChiliOID, QListView * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );

      QmitkPluginListViewItem( QString ChiliOID, QListViewItem * parent, QString label1, QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null, QString label5 = QString::null, QString label6 = QString::null, QString label7 = QString::null, QString label8 = QString::null );

      virtual ~QmitkPluginListViewItem();

      QString GetChiliOID();

    protected:

      QString m_ChiliOID;
};

#endif
