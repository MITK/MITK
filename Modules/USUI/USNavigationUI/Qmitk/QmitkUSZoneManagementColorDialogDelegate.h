#ifndef QMITKUSZONEMANAGEMENTCOLORDIALOGDELEGATE_H
#define QMITKUSZONEMANAGEMENTCOLORDIALOGDELEGATE_H

#include <QStyledItemDelegate>

/**
 * \brief QStyledItemDelegate that provides a QColorDialog as editor.
 */
class QmitkUSZoneManagementColorDialogDelegate : public QStyledItemDelegate
{
public:
  explicit QmitkUSZoneManagementColorDialogDelegate(QObject* parent = 0);
  ~QmitkUSZoneManagementColorDialogDelegate();

  /**
   * \brief A QColorDialog is opened on double click events.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif // QMITKUSZONEMANAGEMENTCOLORDIALOGDELEGATE_H
