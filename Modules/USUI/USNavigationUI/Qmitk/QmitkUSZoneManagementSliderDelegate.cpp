#include "QmitkUSZoneManagementSliderDelegate.h"
#include <QSlider>

QmitkUSZoneManagementSliderDelegate::QmitkUSZoneManagementSliderDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QmitkUSZoneManagementSliderDelegate::~QmitkUSZoneManagementSliderDelegate()
{
}

QWidget* QmitkUSZoneManagementSliderDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QSlider* editor = new QSlider(Qt::Vertical, parent);

    return editor;
}

void QmitkUSZoneManagementSliderDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();

    QSlider *slider = static_cast<QSlider*>(editor);
    slider->setValue(value);
}

void QmitkUSZoneManagementSliderDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QSlider *slider = static_cast<QSlider*>(editor);
    int value = slider->value();

    model->setData(index, value, Qt::EditRole);
}

void QmitkUSZoneManagementSliderDelegate::updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void QmitkUSZoneManagementSliderDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);
}
