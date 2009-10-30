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

#include "qxtcrumbview.h"
#include "qxtcrumbview_p.h"
#include <QApplication>
#include <QPaintEvent>
#include <QListView>
#include <QToolButton>
#include <QBoxLayout>
#include <QModelIndex>
#include <QAbstractItemDelegate>
#include <QStyle>
#include <QStylePainter>
#include <QSize>
#include <QFont>
#include <QtAlgorithms>
#include <QtDebug>

// This class exists only to grant access to QListView's protected members
class QxtCrumbViewList : public QListView {
friend class QxtCrumbView;
public:
    QxtCrumbViewList(QWidget* parent) : QListView(parent) {}
};

class QxtCrumbViewDelegate : public QAbstractItemDelegate {
public:
    QxtCrumbViewDelegate(QAbstractItemDelegate* other, QObject* parent) : QAbstractItemDelegate(parent), delegate(other) {
        QObject::connect(other, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)),
                         this, SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
        QObject::connect(other, SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
        QObject::connect(other, SIGNAL(sizeHintChanged(QModelIndex)), this, SIGNAL(sizeHintChanged(QModelIndex)));

    }
    QAbstractItemDelegate* delegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
        delegate->paint(painter, option, index);

        if(!index.model()->hasChildren(index)) return;

        int arrow = 8;
        int pad = (option.rect.height() - arrow) / 2;
        QStyleOption arrowOption;
        arrowOption = option;
        arrowOption.rect = QRect(option.rect.right() - arrow, option.rect.top() + pad, arrow, arrow);
        QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &arrowOption, painter);
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
        return delegate->sizeHint(option, index) + QSize(8, 0);
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) {
        return delegate->createEditor(parent, option, index);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const {
        delegate->setEditorData(editor, index);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
        delegate->setModelData(editor, model, index);
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const {
        delegate->updateEditorGeometry(editor, option, index);
    }

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) {
        return delegate->editorEvent(event, model, option, index);
    }
};

class QxtCrumbViewButton : public QAbstractButton {
public:
    QxtCrumbViewButton(const QModelIndex& idx, QxtCrumbView* parent) : QAbstractButton(parent), index(idx) {
        // initializers only
    }

    QModelIndex index;

    QSize sizeHint() const {
        QStyleOptionViewItem itemOption;
        itemOption.initFrom(this);
        int border = style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;
        int arrow = 8;
        if(index.isValid())
            return static_cast<QxtCrumbView*>(parent())->itemDelegate()->sizeHint(itemOption, index) + QSize(border + arrow, border);
        else
            return QSize(border + arrow, border + arrow); 
    }

protected:
    void enterEvent(QEvent* event) {
        Q_UNUSED(event);
        update();
    }

    void leaveEvent(QEvent* event) {
        Q_UNUSED(event);
        update();
    }

    void paintEvent(QPaintEvent* event) {
        Q_UNUSED(event);
        QStylePainter painter(this);
        QStyleOptionButton option;
        option.initFrom(this);
        if(rect().contains(mapFromGlobal(QCursor::pos()))) {
            painter.drawPrimitive(QStyle::PE_PanelButtonTool, option);
        }
        int border = painter.style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        option.rect = option.rect.adjusted(border, border, -border, -border);
        QAbstractItemDelegate* delegate = static_cast<QxtCrumbView*>(parent())->itemDelegate();
        QStyleOptionViewItem itemOption;
        itemOption.initFrom(this);
        itemOption.rect = option.rect;
        delegate->paint(&painter, itemOption, index);
        int arrow = 8;
        int pad = (height() - (2 * border) - arrow) / 2;
        QStyleOption arrowOption;
        arrowOption.initFrom(this);
        arrowOption.rect = QRect(width() - border - arrow, pad, arrow, arrow);
        painter.drawPrimitive(QStyle::PE_IndicatorArrowRight, arrowOption);
    }
};

void QxtCrumbViewPrivate::addCrumb(const QModelIndex& index) {
    crumbs.append(index);
    QxtCrumbViewButton* button = new QxtCrumbViewButton(index, &qxt_p());
    button->setFocusPolicy(Qt::NoFocus);
    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    buttons.append(button);
    buttonLayout->insertWidget(buttons.count() - 1, button, 0);
    QObject::connect(button, SIGNAL(clicked()), this, SLOT(buttonPressed()));
}

void QxtCrumbViewPrivate::buttonPressed() {
    qxt_p().enterTree(static_cast<QxtCrumbViewButton*>(sender())->index);
}

QxtCrumbView::QxtCrumbView(QWidget* parent) : QAbstractItemView(parent) {
    QXT_INIT_PRIVATE(QxtCrumbView);
    setAutoFillBackground(false);
    setBackgroundRole(QPalette::Window);
    viewport()->setAutoFillBackground(false);
    viewport()->setBackgroundRole(QPalette::Window);
    setFrameStyle(QFrame::NoFrame);
    QVBoxLayout* layout = new QVBoxLayout(this);
    qxt_d().buttonLayout = new QHBoxLayout;
    qxt_d().buttonLayout->setSpacing(0);
    qxt_d().buttonLayout->addStretch(1);
    QToolButton* backButton = new QToolButton(this);
    backButton->setIcon(style()->standardPixmap(QStyle::SP_FileDialogBack));
    backButton->setAutoRaise(true);
    backButton->setFocusPolicy(Qt::NoFocus);
    qxt_d().buttonLayout->addWidget(backButton);
    layout->addLayout(qxt_d().buttonLayout, 0);
    qxt_d().view = new QxtCrumbViewList(this);
    layout->addWidget(qxt_d().view, 1);
    qxt_d().addCrumb(QModelIndex());

    QObject::connect(qxt_d().view, SIGNAL(activated(QModelIndex)), this, SLOT(enterTree(QModelIndex)));
    QObject::connect(backButton, SIGNAL(clicked()), this, SLOT(back()));
}

void QxtCrumbView::setModel(QAbstractItemModel* model) {
    qxt_d().view->setModel(model);
    QAbstractItemView::setModel(model);
    delete qxt_d().view->itemDelegate();
    qxt_d().view->setItemDelegate(new QxtCrumbViewDelegate(itemDelegate(), this));
}

void QxtCrumbView::reset() {
    QAbstractItemView::reset();
    qDeleteAll(qxt_d().buttons);
    qxt_d().crumbs.clear();
    qxt_d().buttons.clear();
    qxt_d().addCrumb(QModelIndex());
}

int QxtCrumbView::horizontalOffset() const {
    return qxt_d().view->horizontalOffset();
}

int QxtCrumbView::verticalOffset() const {
    return qxt_d().view->verticalOffset();
}

QModelIndex QxtCrumbView::indexAt(const QPoint& point) const {
    return qxt_d().view->indexAt(qxt_d().view->mapFromParent(point));
}

bool QxtCrumbView::isIndexHidden(const QModelIndex& index) const {
    return qxt_d().view->isIndexHidden(index);
}

QModelIndex QxtCrumbView::moveCursor(CursorAction action, Qt::KeyboardModifiers mods) {
    return qxt_d().view->moveCursor(action, mods);
}

void QxtCrumbView::scrollTo(const QModelIndex& index, ScrollHint hint) {
    if(index.parent() != qxt_d().view->rootIndex()) {
        // set the breadcrumbs and the view's root index correctly
    }
    qxt_d().view->scrollTo(index, hint);
}

void QxtCrumbView::setSelection(const QRect& rect, QItemSelectionModel::SelectionFlags flags) {
    QPoint tl = qxt_d().view->mapFromParent(rect.topLeft());
    qxt_d().view->setSelection(QRect(tl, rect.size()), flags);
}

QRect QxtCrumbView::visualRect(const QModelIndex& index) const {
    QRect rect = qxt_d().view->visualRect(index);
    return QRect(qxt_d().view->mapToParent(rect.topLeft()), rect.size());
}

QRegion QxtCrumbView::visualRegionForSelection(const QItemSelection& selection) const {
    QRegion region = qxt_d().view->visualRegionForSelection(selection);
    return region.translated(qxt_d().view->pos());
}

void QxtCrumbView::enterTree(const QModelIndex& index) {
    if(!model() || !model()->hasChildren(index)) return;
    if(index == qxt_d().view->rootIndex()) {
        // do nothing but reload the view
    } else if(index.parent() != qxt_d().view->rootIndex()) {
        foreach(QxtCrumbViewButton* b, qxt_d().buttons) b->deleteLater();
        qxt_d().crumbs.clear();
        qxt_d().buttons.clear();
        qxt_d().addCrumb(QModelIndex());
        QList<QModelIndex> chain;
        QModelIndex pos = index;
        while(pos.isValid()) {
            chain.append(pos);
            pos = pos.parent();
        }
        while(!chain.isEmpty()) {
            qxt_d().addCrumb(chain.last());
            chain.removeLast();
        }
    } else {
        qxt_d().addCrumb(index);
    }
    qxt_d().view->setRootIndex(index);
}

void QxtCrumbView::back() {
    if(qxt_d().crumbs.count() <= 1) return;
    qxt_d().buttons.last()->deleteLater();
    qxt_d().buttons.removeLast();
    qxt_d().crumbs.removeLast();
    qxt_d().view->setRootIndex(qxt_d().crumbs.last());
}

void QxtCrumbView::setItemDelegate(QAbstractItemDelegate* delegate) {
    QAbstractItemView::setItemDelegate(delegate);
    delete qxt_d().view->itemDelegate();
    qxt_d().view->setItemDelegate(new QxtCrumbViewDelegate(itemDelegate(), this));
}

QListView* QxtCrumbView::listView() const {
    return qxt_d().view;
}
