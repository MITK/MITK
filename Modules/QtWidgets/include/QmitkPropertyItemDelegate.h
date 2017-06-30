/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkPropertyItemDelegate_h
#define QmitkPropertyItemDelegate_h

#include <MitkQtWidgetsExports.h>
#include <QListView>
#include <QStyledItemDelegate>
#include <mitkPropertyList.h>
#include <mitkWeakPointer.h>

class QComboBox;
class QLineEdit;
class QToolButton;
class QResizeEvent;

class QmitkColorWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkColorWidget(QWidget *parent = nullptr);
  ~QmitkColorWidget();

  QColor GetColor() const;
  void SetColor(QColor color);

signals:
  void ColorPicked();

private slots:
  void OnButtonClicked();
  void OnLineEditEditingFinished();

private:
  QColor m_Color;
  QLineEdit *m_LineEdit;
  QToolButton *m_Button;
};

class QmitkComboBoxListView : public QListView
{
  Q_OBJECT

public:
  explicit QmitkComboBoxListView(QComboBox *comboBox = nullptr);
  ~QmitkComboBoxListView();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  QStyleOptionViewItem viewOptions() const override;

private:
  QComboBox *m_ComboBox;
};

class MITKQTWIDGETS_EXPORT QmitkPropertyItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkPropertyItemDelegate(QObject *parent = nullptr);
  ~QmitkPropertyItemDelegate();

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
  void SetPropertyList(mitk::PropertyList *propertyList);

private slots:
  void OnComboBoxCurrentIndexChanged(int index);
  void OnSpinBoxEditingFinished();
  void OnColorPicked();

private:
  std::string GetPropertyName(const QModelIndex &index) const;

  mitk::WeakPointer<mitk::PropertyList> m_PropertyList;
};

#endif
