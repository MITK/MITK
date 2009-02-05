#include "QmitkPropertyDelegate.h"

QmitkPropertyDelegate::QmitkPropertyDelegate(QObject *parent)
{
}

void QmitkPropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
}

QWidget* QmitkPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
  QWidget* editor = 0;

  return editor;
}

void QmitkPropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
}

void QmitkPropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel* model
  , const QModelIndex &index) const
{
}

void QmitkPropertyDelegate::commitAndCloseEditor()
{
}