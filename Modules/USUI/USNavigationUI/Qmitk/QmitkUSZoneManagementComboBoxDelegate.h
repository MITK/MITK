#ifndef QMITKUSZONEMANAGEMENTCOMBOBOXDELEGATE_H
#define QMITKUSZONEMANAGEMENTCOMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

class QmitkUSZoneManagementComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit QmitkUSZoneManagementComboBoxDelegate(QObject* parent = 0);
    ~QmitkUSZoneManagementComboBoxDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;


    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // QMITKUSZONEMANAGEMENTCOMBOBOXDELEGATE_H
