/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/
#ifndef QXTSORTFILTERPROXYMODEL_H_INCLUDED
#define QXTSORTFILTERPROXYMODEL_H_INCLUDED

#include <QSortFilterProxyModel>
#include <QRegExp>
#include <QMap>
#include <QVariant>

#include "qxtglobal.h"
#include "qxtpimpl.h"

class QxtSortFilterProxyModelPrivate;

class QXT_GUI_EXPORT QxtSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtSortFilterProxyModel);

    public:
        QxtSortFilterProxyModel                         ( QObject *parent = 0 );
        virtual bool                filterAcceptsRow    ( int source_row, const QModelIndex &source_parent ) const;
        void                        beginDeclareFilter  ( );
        void                        endDeclareFilter    ( );
        void                        setFilter           ( const int column , const QVariant &value, const int role = Qt::DisplayRole, Qt::MatchFlags flags = Qt::MatchContains);
        void                        setFilterValue      ( const int column , const QVariant &value );
        void                        setFilterRole       ( const int column , const int role = Qt::DisplayRole );
        void                        setFilterFlags      ( const int column , const Qt::MatchFlags flags = Qt::MatchContains );
        void                        removeFilter        ( const int column );

        QVariant                    filterValue         ( const int column ) const;
        int                         filterRole          ( const int column ) const;
        Qt::MatchFlags              filterFlags         ( const int column ) const;

        bool                        isFiltered          ( const int column );
};
#endif
