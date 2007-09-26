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

#include "QmitkPluginListViewItem.h"

QmitkPluginListViewItem::QmitkPluginListViewItem( QString ChiliOID, QListView * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
:QListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 ),
  m_ChiliOID( ChiliOID )
{}

QmitkPluginListViewItem::QmitkPluginListViewItem( QString ChiliOID, QListViewItem * parent, QString label1, QString label2, QString label3, QString label4, QString label5, QString label6, QString label7, QString label8 )
:QListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 ),
  m_ChiliOID( ChiliOID )
{}

QmitkPluginListViewItem::~QmitkPluginListViewItem()
{
}

QString QmitkPluginListViewItem::GetChiliOID()
{
  return m_ChiliOID;
}

