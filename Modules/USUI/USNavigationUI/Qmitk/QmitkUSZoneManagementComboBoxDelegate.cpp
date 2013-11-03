#include "QmitkUSZoneManagementComboBoxDelegate.h"
#include <QComboBox>

QmitkUSZoneManagementComboBoxDelegate::QmitkUSZoneManagementComboBoxDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QmitkUSZoneManagementComboBoxDelegate::~QmitkUSZoneManagementComboBoxDelegate()
{
}

QWidget* QmitkUSZoneManagementComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QComboBox* editor = new QComboBox(parent);
    editor->addItem("Red");
    editor->addItem("Yellow");
    editor->addItem("Blue");
    editor->addItem("Green");

    return editor;
}

void QmitkUSZoneManagementComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int comboIndex = comboBox->findText(value);

    if ( comboIndex != -1 )
    {
      comboBox->setCurrentIndex(comboIndex);
    }
}

void QmitkUSZoneManagementComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString value = comboBox->currentText();

    model->setData(index, value, Qt::EditRole);
}

void QmitkUSZoneManagementComboBoxDelegate::updateEditorGeometry(QWidget *editor,
                                    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void QmitkUSZoneManagementComboBoxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);
}
