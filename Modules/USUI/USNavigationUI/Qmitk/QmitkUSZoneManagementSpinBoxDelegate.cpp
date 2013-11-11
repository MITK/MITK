#include "QmitkUSZoneManagementSpinBoxDelegate.h"
#include <QSpinBox>

QmitkUSZoneManagementSpinBoxDelegate::QmitkUSZoneManagementSpinBoxDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QmitkUSZoneManagementSpinBoxDelegate::~QmitkUSZoneManagementSpinBoxDelegate()
{
}

QWidget* QmitkUSZoneManagementSpinBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QSpinBox* editor = new QSpinBox(parent);
    editor->setMaximum(300);

    return editor;
}

void QmitkUSZoneManagementSpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
    spinbox->setValue(value);
}

void QmitkUSZoneManagementSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
    int value = spinbox->value();

    model->setData(index, value, Qt::EditRole);
}

void QmitkUSZoneManagementSpinBoxDelegate::updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
