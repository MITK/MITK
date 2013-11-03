#ifndef QMITKUSZONEMANAGEMENTSLIDERDELEGATE_H
#define QMITKUSZONEMANAGEMENTSLIDERDELEGATE_H

#include <QStyledItemDelegate>

class QmitkUSZoneManagementSliderDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    explicit QmitkUSZoneManagementSliderDelegate(QObject* parent = 0);
    ~QmitkUSZoneManagementSliderDelegate();

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void setEditorData(QWidget* editor, const QModelIndex& index) const;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;


    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // QMITKUSZONEMANAGEMENTSLIDERDELEGATE_H
