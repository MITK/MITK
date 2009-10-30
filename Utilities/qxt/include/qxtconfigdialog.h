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
#ifndef QXTCONFIGDIALOG_H
#define QXTCONFIGDIALOG_H

#include <QDialog>
#include "qxtglobal.h"
#include "qxtpimpl.h"

QT_FORWARD_DECLARE_CLASS(QTableWidget)
QT_FORWARD_DECLARE_CLASS(QStackedWidget)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
class QxtConfigDialogPrivate;

class QXT_GUI_EXPORT QxtConfigDialog : public QDialog
{
    Q_OBJECT
    QXT_DECLARE_PRIVATE(QxtConfigDialog);
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(bool hoverEffect READ hasHoverEffect WRITE setHoverEffect)
    Q_PROPERTY(QxtConfigDialog::IconPosition iconPosition READ iconPosition WRITE setIconPosition)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_ENUMS(IconPosition)

public:
    enum IconPosition { North, West, East };

    explicit QxtConfigDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
    explicit QxtConfigDialog(QxtConfigDialog::IconPosition position, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~QxtConfigDialog();

    QDialogButtonBox* dialogButtonBox() const;
    void setDialogButtonBox(QDialogButtonBox* buttonBox);

    bool hasHoverEffect() const;
    void setHoverEffect(bool enabled);

    QxtConfigDialog::IconPosition iconPosition() const;
    void setIconPosition(QxtConfigDialog::IconPosition position);

    QSize iconSize() const;
    void setIconSize(const QSize& size);

    int addPage(QWidget* page, const QIcon& icon, const QString& title = QString());
    int insertPage(int index, QWidget* page, const QIcon& icon, const QString& title = QString());
    QWidget* takePage(int index);

    int count() const;
    int currentIndex() const;
    QWidget* currentPage() const;

    int indexOf(QWidget* page) const;
    QWidget* page(int index) const;

    bool isPageEnabled(int index) const;
    void setPageEnabled(int index, bool enabled);

    bool isPageHidden(int index) const;
    void setPageHidden(int index, bool hidden);

    QIcon pageIcon(int index) const;
    void setPageIcon(int index, const QIcon& icon);

    QString pageTitle(int index) const;
    void setPageTitle(int index, const QString& title);

    QString pageToolTip(int index) const;
    void setPageToolTip(int index, const QString& tooltip);

    QString pageWhatsThis(int index) const;
    void setPageWhatsThis(int index, const QString& whatsthis);

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setCurrentPage(QWidget* page);

    virtual void accept();
    virtual void reject();

Q_SIGNALS:
    void currentIndexChanged(int index);

protected:
    QTableWidget* tableWidget() const;
    QStackedWidget* stackedWidget() const;

    virtual void cleanupPage(int index);
    virtual void initializePage(int index);
};

#endif // QXTCONFIGDIALOG_H
