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
#include "qxtconfigdialog.h"
#include "qxtconfigdialog_p.h"
#if QT_VERSION >= 0x040200
#include <QDialogButtonBox>
#else // QT_VERSION >= 0x040200
#include <QHBoxLayout>
#include <QPushButton>
#endif // QT_VERSION
#include <QStackedWidget>
#include <QApplication>
#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QSplitter>
#include <QPainter>

QxtConfigTableWidget::QxtConfigTableWidget(QWidget* parent) : QTableWidget(parent)
{
    int pm = style()->pixelMetric(QStyle::PM_LargeIconSize);
    setIconSize(QSize(pm, pm));
    setItemDelegate(new QxtConfigDelegate(this));
    viewport()->setAttribute(Qt::WA_Hover, true);
}

QStyleOptionViewItem QxtConfigTableWidget::viewOptions() const
{
    QStyleOptionViewItem option = QTableWidget::viewOptions();
    option.displayAlignment = Qt::AlignHCenter | Qt::AlignTop;
    option.decorationAlignment = Qt::AlignHCenter | Qt::AlignTop;
    option.decorationPosition = QStyleOptionViewItem::Top;
    option.showDecorationSelected = false;
    return option;
}

QSize QxtConfigTableWidget::sizeHint() const
{
    return QSize(sizeHintForColumn(0), sizeHintForRow(0));
}

bool QxtConfigTableWidget::hasHoverEffect() const
{
    return static_cast<QxtConfigDelegate*>(itemDelegate())->hover;
}

void QxtConfigTableWidget::setHoverEffect(bool enabled)
{
    static_cast<QxtConfigDelegate*>(itemDelegate())->hover = enabled;
}

QxtConfigDelegate::QxtConfigDelegate(QObject* parent)
        : QItemDelegate(parent), hover(true)
{
}

void QxtConfigDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    if (hover)
    {
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        if (option.state & QStyle::State_Selected)
            painter->fillRect(opt.rect, option.palette.brush(cg, QPalette::Highlight));
        else if ((option.state & QStyle::State_MouseOver) && (option.state & QStyle::State_Enabled))
        {
            QColor color = option.palette.color(cg, QPalette::Highlight).light();
            if (color == option.palette.color(cg, QPalette::Base))
                color = option.palette.color(cg, QPalette::AlternateBase);
            painter->fillRect(opt.rect, color);
        }
        else
            painter->fillRect(opt.rect, option.palette.brush(cg, QPalette::Base));

        opt.showDecorationSelected = false;
        opt.state &= ~QStyle::State_HasFocus;
        opt.state &= ~QStyle::State_Selected;
    }
    QItemDelegate::paint(painter, opt, index);
}

QSize QxtConfigDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    int margin = qApp->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    int textWidth = option.fontMetrics.width(index.data().toString());
    int width = qMax(textWidth, option.decorationSize.width()) + 2 * margin;
    int height = option.fontMetrics.height() + option.decorationSize.height() + margin;
    return QSize(width, height);
}

void QxtConfigDialogPrivate::init(QxtConfigDialog::IconPosition position)
{
    QxtConfigDialog* p = &qxt_p();
    splitter = new QSplitter(p);
    stack = new QStackedWidget(p);
    table = new QxtConfigTableWidget(p);
    pos = position;
    QObject::connect(table, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(setCurrentIndex(int, int)));
    QObject::connect(stack, SIGNAL(currentChanged(int)), p, SIGNAL(currentIndexChanged(int)));

#if QT_VERSION >= 0x040200
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, p);
    QObject::connect(buttons, SIGNAL(accepted()), p, SLOT(accept()));
    QObject::connect(buttons, SIGNAL(rejected()), p, SLOT(reject()));
#else // QT_VERSION >= 0x040200
    buttons = new QWidget(p);
    QHBoxLayout* layout = new QHBoxLayout(buttons);
    QPushButton* okButton = new QPushButton(QxtConfigDialog::tr("&OK"));
    QPushButton* cancelButton = new QPushButton(QxtConfigDialog::tr("&Cancel"));
    QObject::connect(okButton, SIGNAL(clicked()), p, SLOT(accept()));
    QObject::connect(cancelButton, SIGNAL(clicked()), p, SLOT(reject()));
    layout->addStretch();
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);
#endif
    QVBoxLayout* layout = new QVBoxLayout(p);
    layout->addWidget(splitter);
    layout->addWidget(buttons);
    initTable();
    relayout();
}

void QxtConfigDialogPrivate::initTable()
{
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setTabKeyNavigation(true);
    table->setAcceptDrops(false);
    table->setDragEnabled(false);
    table->setShowGrid(false);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
}

void QxtConfigDialogPrivate::relayout()
{
    if (pos == QxtConfigDialog::North)
    {
        splitter->setOrientation(Qt::Vertical);
        table->setRowCount(1);
        table->setColumnCount(0);
        table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        table->verticalHeader()->setResizeMode(QHeaderView::Stretch);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        splitter->setOrientation(Qt::Horizontal);
        table->setRowCount(0);
        table->setColumnCount(1);
        table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        table->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    // clear
    for (int i = splitter->count() - 1; i >= 0; --i)
    {
        splitter->widget(i)->setParent(0);
    }

    // relayout
    switch (pos)
    {
    case QxtConfigDialog::North:
        // +-----------+
        // |   Icons   |
        // +-----------|
        // |   Stack   |
        // +-----------|
        // |  Buttons  |
        // +-----------+
        splitter->addWidget(table);
        splitter->addWidget(stack);
        break;

    case QxtConfigDialog::West:
        // +---+-------+
        // | I |       |
        // | c |       |
        // | o | Stack |
        // | n |       |
        // | s |       |
        // +---+-------+
        // |  Buttons  |
        // +-----------+
        splitter->addWidget(table);
        splitter->addWidget(stack);
        break;

    case QxtConfigDialog::East:
        // +-------+---+
        // |       | I |
        // |       | c |
        // | Stack | o |
        // |       | n |
        // |       | s |
        // +-------+---+
        // |  Buttons  |
        // +-----------+
        splitter->addWidget(stack);
        splitter->addWidget(table);
        break;

    default:
        qWarning("QxtConfigDialogPrivate::relayout(): unknown position");
        break;
    }

    if (pos == QxtConfigDialog::East)
    {
        splitter->setStretchFactor(0, 10);
        splitter->setStretchFactor(1, 1);
    }
    else
    {
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 10);
    }
}

QTableWidgetItem* QxtConfigDialogPrivate::item(int index) const
{
    return pos == QxtConfigDialog::North ? table->item(0, index) : table->item(index, 0);
}

void QxtConfigDialogPrivate::setCurrentIndex(int row, int column)
{
    if (pos == QxtConfigDialog::North)
        setCurrentIndex(column);
    else
        setCurrentIndex(row);
}

void QxtConfigDialogPrivate::setCurrentIndex(int index)
{
    int previousIndex = stack->currentIndex();
    if (previousIndex != -1 && previousIndex != index)
        qxt_p().cleanupPage(previousIndex);

    stack->setCurrentIndex(index);
    table->setCurrentItem(item(index));

    if (index != -1)
        qxt_p().initializePage(index);
}

/*!
    \class QxtConfigDialog QxtConfigDialog
    \ingroup QxtGui
    \brief A configuration dialog.

    QxtConfigDialog provides a convenient interface for building
    common configuration dialogs. QxtConfigDialog consists of a
    list of icons and a stack of pages.

    Example usage:
    \code
    QxtConfigDialog dialog;
    dialog.addPage(new ConfigurationPage(&dialog), QIcon(":/images/config.png"), tr("Configuration"));
    dialog.addPage(new UpdatePage(&dialog), QIcon(":/images/update.png"), tr("Update"));
    dialog.addPage(new QueryPage(&dialog), QIcon(":/images/query.png"), tr("Query"));
    dialog.exec();
    \endcode

    \image html qxtconfigdialog.png "QxtConfigDialog with page icons on the left (QxtConfigDialog::West)."
 */

/*!
    \enum IconPosition::IconPosition

    This enum describes the page icon position.

    \sa QxtCheckComboBox::iconPosition
 */

/*!
    \var QxtConfigDialog::IconPosition QxtConfigDialog::North

    The icons are located above the pages.
 */

/*!
    \var QxtConfigDialog::IconPosition QxtConfigDialog::West

    The icons are located to the left of the pages.
 */

/*!
    \var QxtConfigDialog::IconPosition QxtConfigDialog::East

    The icons are located to the right of the pages.
 */

/*!
    \fn QxtConfigDialog::currentIndexChanged(int index)

    This signal is emitted whenever the current page \a index changes.

    \sa currentIndex()
 */

/*!
    Constructs a new QxtConfigDialog with \a parent and \a flags.
 */
QxtConfigDialog::QxtConfigDialog(QWidget* parent, Qt::WindowFlags flags)
        : QDialog(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigDialog);
    qxt_d().init();
}

/*!
    Constructs a new QxtConfigDialog with icon \a position, \a parent and \a flags.
 */
QxtConfigDialog::QxtConfigDialog(QxtConfigDialog::IconPosition position, QWidget* parent, Qt::WindowFlags flags)
        : QDialog(parent, flags)
{
    QXT_INIT_PRIVATE(QxtConfigDialog);
    qxt_d().init(position);
}

/*!
    Destructs the config dialog.
 */
QxtConfigDialog::~QxtConfigDialog()
{}

/*!
    Returns the dialog button box.

    The default buttons are \b QDialogButtonBox::Ok and \b QDialogButtonBox::Cancel.

    \note QDialogButtonBox is available in Qt 4.2 or newer.

    \sa setDialogButtonBox()
*/
#if QT_VERSION >= 0x040200
QDialogButtonBox* QxtConfigDialog::dialogButtonBox() const
{
    return qxt_d().buttons;
}
#endif // QT_VERSION

/*!
    Sets the dialog \a buttonBox.

    \sa dialogButtonBox()
*/
#if QT_VERSION >= 0x040200
void QxtConfigDialog::setDialogButtonBox(QDialogButtonBox* buttonBox)
{
    if (qxt_d().buttons != buttonBox)
    {
        if (qxt_d().buttons && qxt_d().buttons->parent() == this)
        {
            delete qxt_d().buttons;
        }
        qxt_d().buttons = buttonBox;
        qxt_d().relayout();
    }
}
#endif // QT_VERSION

/*!
    \property QxtConfigDialog::hoverEffect
    \brief This property holds whether a hover effect is shown for page icons

    The default value is \b true.

    \note Hovered (but not selected) icons are highlighted with lightened \b QPalette::Highlight
    (whereas selected icons are highlighted with \b QPalette::Highlight). In case lightened
    \b QPalette::Highlight ends up same as \b QPalette::Base, \b QPalette::AlternateBase is used
    as a fallback color for the hover effect. This usually happens when \b QPalette::Highlight
    already is a light color (eg. light gray).
 */
bool QxtConfigDialog::hasHoverEffect() const
{
    return qxt_d().table->hasHoverEffect();
}

void QxtConfigDialog::setHoverEffect(bool enabled)
{
    qxt_d().table->setHoverEffect(enabled);
}

/*!
    \property QxtConfigDialog::iconPosition
    \brief This property holds the position of page icons
 */
QxtConfigDialog::IconPosition QxtConfigDialog::iconPosition() const
{
    return qxt_d().pos;
}

void QxtConfigDialog::setIconPosition(QxtConfigDialog::IconPosition position)
{
    if (qxt_d().pos != position)
    {
        qxt_d().pos = position;
        qxt_d().relayout();
    }
}

/*!
    \property QxtConfigDialog::iconSize
    \brief This property holds the size of page icons
 */
QSize QxtConfigDialog::iconSize() const
{
    return qxt_d().table->iconSize();
}

void QxtConfigDialog::setIconSize(const QSize& size)
{
    qxt_d().table->setIconSize(size);
}

/*!
    Adds a \a page with \a icon and \a title.

    In case \a title is an empty string, \b QWidget::windowTitle is used.

    Returns the index of added page.

    \warning Adding and removing pages dynamically at run time might cause flicker.

    \sa insertPage()
*/
int QxtConfigDialog::addPage(QWidget* page, const QIcon& icon, const QString& title)
{
    return insertPage(-1, page, icon, title);
}

/*!
    Inserts a \a page with \a icon and \a title.

    In case \a title is an empty string, \b QWidget::windowTitle is used.

    Returns the index of inserted page.

    \warning Inserting and removing pages dynamically at run time might cause flicker.

    \sa addPage()
*/
int QxtConfigDialog::insertPage(int index, QWidget* page, const QIcon& icon, const QString& title)
{
    if (!page)
    {
        qWarning("QxtConfigDialog::insertPage(): Attempt to insert null page");
        return -1;
    }

    index = qxt_d().stack->insertWidget(index, page);
    const QString label = !title.isEmpty() ? title : page->windowTitle();
    if (label.isEmpty())
        qWarning("QxtConfigDialog::insertPage(): Inserting a page with an empty title");
    QTableWidgetItem* item = new QTableWidgetItem(icon, label);
    item->setToolTip(label);
    if (qxt_d().pos == QxtConfigDialog::North)
    {
        qxt_d().table->model()->insertColumn(index);
        qxt_d().table->setItem(0, index, item);
        qxt_d().table->resizeRowToContents(0);
    }
    else
    {
        qxt_d().table->model()->insertRow(index);
        qxt_d().table->setItem(index, 0, item);
        qxt_d().table->resizeColumnToContents(0);
    }
    qxt_d().table->updateGeometry();
    return index;
}

/*!
   Removes the page at \a index and returns it.

   \note Does not delete the page widget.
*/
QWidget* QxtConfigDialog::takePage(int index)
{
    if (QWidget* page = qxt_d().stack->widget(index))
    {
        qxt_d().stack->removeWidget(page);
        delete qxt_d().item(index);
        return page;
    }
    else
    {
        qWarning("QxtConfigDialog::removePage(): Unknown index");
        return 0;
    }
}

/*!
    \property QxtConfigDialog::count
    \brief This property holds the number of pages
*/
int QxtConfigDialog::count() const
{
    return qxt_d().stack->count();
}

/*!
    \property QxtConfigDialog::currentIndex
    \brief This property holds the index of current page
*/
int QxtConfigDialog::currentIndex() const
{
    return qxt_d().stack->currentIndex();
}

void QxtConfigDialog::setCurrentIndex(int index)
{
    qxt_d().setCurrentIndex(index);
}

/*!
    Returns the current page.

    \sa currentIndex(), setCurrentPage()
*/
QWidget* QxtConfigDialog::currentPage() const
{
    return qxt_d().stack->currentWidget();
}

/*!
    Sets the current \a page.

    \sa currentPage(), currentIndex()
*/
void QxtConfigDialog::setCurrentPage(QWidget* page)
{
    qxt_d().setCurrentIndex(qxt_d().stack->indexOf(page));
}

/*!
    Returns the index of \a page or \b -1 if the page is unknown.
*/
int QxtConfigDialog::indexOf(QWidget* page) const
{
    return qxt_d().stack->indexOf(page);
}

/*!
    Returns the page at \a index or \b 0 if the \a index is out of range.
*/
QWidget* QxtConfigDialog::page(int index) const
{
    return qxt_d().stack->widget(index);
}

/*!
    Returns \b true if the page at \a index is enabled; otherwise \b false.

    \sa setPageEnabled(), QWidget::isEnabled()
*/
bool QxtConfigDialog::isPageEnabled(int index) const
{
    const QWidget* widget = page(index);
    return widget && widget->isEnabled();
}

/*!
    Sets the page at \a index \a enabled. The corresponding
    page icon is also \a enabled.

    \sa isPageEnabled(), QWidget::setEnabled()
*/
void QxtConfigDialog::setPageEnabled(int index, bool enabled)
{
    QWidget* page = qxt_d().stack->widget(index);
    QTableWidgetItem* item = qxt_d().item(index);
    if (page && item)
    {
        page->setEnabled(enabled);
        if (enabled)
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        else
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }
    else
    {
        qWarning("QxtConfigDialog::setPageEnabled(): Unknown index");
    }
}

/*!
    Returns \b true if the page at \a index is hidden; otherwise \b false.

    \sa setPageHidden(), QWidget::isVisible()
*/
bool QxtConfigDialog::isPageHidden(int index) const
{
    if (qxt_d().pos == QxtConfigDialog::North)
        return qxt_d().table->isColumnHidden(index);
    return qxt_d().table->isRowHidden(index);
}

/*!
    Sets the page at \a index \a hidden. The corresponding
    page icon is also \a hidden.

    \sa isPageHidden(), QWidget::setVisible()
*/
void QxtConfigDialog::setPageHidden(int index, bool hidden)
{
    if (qxt_d().pos == QxtConfigDialog::North)
        qxt_d().table->setColumnHidden(index, hidden);
    else
        qxt_d().table->setRowHidden(index, hidden);
}

/*!
    Returns the icon of page at \a index.

    \sa setPageIcon()
*/
QIcon QxtConfigDialog::pageIcon(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->icon() : QIcon());
}

/*!
    Sets the \a icon of page at \a index.

    \sa pageIcon()
*/
void QxtConfigDialog::setPageIcon(int index, const QIcon& icon)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setIcon(icon);
    }
    else
    {
        qWarning("QxtConfigDialog::setPageIcon(): Unknown index");
    }
}

/*!
    Returns the title of page at \a index.

    \sa setPageTitle()
*/
QString QxtConfigDialog::pageTitle(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->text() : QString());
}

/*!
    Sets the \a title of page at \a index.

    \sa pageTitle()
*/
void QxtConfigDialog::setPageTitle(int index, const QString& title)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setText(title);
    }
    else
    {
        qWarning("QxtConfigDialog::setPageTitle(): Unknown index");
    }
}

/*!
    Returns the tooltip of page at \a index.

    \sa setPageToolTip()
*/
QString QxtConfigDialog::pageToolTip(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->toolTip() : QString());
}

/*!
    Sets the \a tooltip of page at \a index.

    \sa pageToolTip()
*/
void QxtConfigDialog::setPageToolTip(int index, const QString& tooltip)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setToolTip(tooltip);
    }
    else
    {
        qWarning("QxtConfigDialog::setPageToolTip(): Unknown index");
    }
}

/*!
    Returns the what's this of page at \a index.

    \sa setPageWhatsThis()
*/
QString QxtConfigDialog::pageWhatsThis(int index) const
{
    const QTableWidgetItem* item = qxt_d().item(index);
    return (item ? item->whatsThis() : QString());
}

/*!
    Sets the \a whatsthis of page at \a index.

    \sa pageWhatsThis()
*/
void QxtConfigDialog::setPageWhatsThis(int index, const QString& whatsthis)
{
    QTableWidgetItem* item = qxt_d().item(index);
    if (item)
    {
        item->setWhatsThis(whatsthis);
    }
    else
    {
        qWarning("QxtConfigDialog::setPageWhatsThis(): Unknown index");
    }
}

/*!
    Reimplemented from QDialog.

    \note The default implementation calls SLOT(accept()) of
    each page page provided that such slot exists.

    \sa reject()
 */
void QxtConfigDialog::accept()
{
    Q_ASSERT(qxt_d().stack);
    for (int i = 0; i < qxt_d().stack->count(); ++i)
    {
        QMetaObject::invokeMethod(qxt_d().stack->widget(i), "accept");
    }
    QDialog::accept();
}

/*!
    Reimplemented from QDialog.

    \note The default implementation calls SLOT(reject()) of
    each page provided that such slot exists.

    \sa accept()
 */
void QxtConfigDialog::reject()
{
    Q_ASSERT(qxt_d().stack);
    for (int i = 0; i < qxt_d().stack->count(); ++i)
    {
        QMetaObject::invokeMethod(qxt_d().stack->widget(i), "reject");
    }
    QDialog::reject();
}

/*!
    Returns the internal table widget used for showing page icons.

    \sa stackedWidget()
*/
QTableWidget* QxtConfigDialog::tableWidget() const
{
    return qxt_d().table;
}

/*!
    Returns the internal stacked widget used for stacking pages.

    \sa tableWidget()
*/
QStackedWidget* QxtConfigDialog::stackedWidget() const
{
    return qxt_d().stack;
}

/*!
    This virtual function is called to clean up previous
    page at \a index before switching to a new page.

    \note The default implementation calls SLOT(cleanup()) of
    the corresponding page provided that such slot exists.

    \sa initializePage()
*/
void QxtConfigDialog::cleanupPage(int index)
{
    Q_ASSERT(qxt_d().stack);
    QMetaObject::invokeMethod(qxt_d().stack->widget(index), "cleanup");
}

/*!
    This virtual function is called to initialize page at
    \a index before switching to it.

    \note The default implementation calls SLOT(initialize()) of
    the corresponding page provided that such slot exists.

    \sa cleanupPage()
*/
void QxtConfigDialog::initializePage(int index)
{
    Q_ASSERT(qxt_d().stack);
    QMetaObject::invokeMethod(qxt_d().stack->widget(index), "initialize");
}
