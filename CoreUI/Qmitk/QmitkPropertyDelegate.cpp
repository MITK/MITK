#include "QmitkPropertyDelegate.h"

#include "QmitkCustomVariants.h"

#include <bitset>
#include <QPainter>
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStringList>
#include <QMessageBox>
#include <QPen>

QmitkPropertyDelegate::QmitkPropertyDelegate(QObject *parent)
{
}

void QmitkPropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{

  QVariant data = index.data(Qt::DisplayRole);

  QString name = data.value<QString>();

  if(index.column() == 1 && data.type() == QVariant::Color)
  {
    QColor qcol = data.value<QColor>();

    painter->save();
    painter->fillRect(option.rect, qcol);
    QRect rect = option.rect;
    rect.setWidth(rect.width()-1);
    rect.setHeight(rect.height()-1);
    QPen pen;
    pen.setWidth(1);
    painter->setPen(pen);
    painter->drawRect(rect);
    painter->restore();

  }
  else
  {
    QStyledItemDelegate::paint(painter, option, index);
  }
}

QWidget* QmitkPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);
  QString name = index.model()->data(index.model()->index(index.row(), index.column()-1)).value<QString>();

  if(data.isValid())
  {

    if(data.type() == QVariant::Color)
    {
      QPushButton* colorBtn = new QPushButton(parent);
      QColor color = data.value<QColor>();
      
      QColor result = QColorDialog::getColor(color);
      if(result.isValid())
      {       
        QPalette palette = colorBtn->palette();
        palette.setColor(QPalette::Button, result);
        colorBtn->setPalette(palette);
        colorBtn->setStyleSheet(QString("background-color: %1;foreground-color: %1; border-style: none;").arg(result.name()));
        //colorBtn->setFlat(true);
      }

      connect(colorBtn, SIGNAL(pressed()), this, SLOT(commitAndCloseEditor()));

      return colorBtn;
    }

/*
    else if(data.type() == QVariant::Bool)
    {
      QCheckBox *visibilityCheckBox = new QCheckBox(parent);
      connect(visibilityCheckBox, SIGNAL(editingFinished()),
        this, SLOT(commitAndCloseEditor()));

      return visibilityCheckBox;
    }*/


    else if(data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = new QSpinBox(parent);
      spinBox->setSingleStep(1);
      return spinBox;
    }
    // see qt documentation. cast is correct, it would be obsolete if we 
    // store doubles
    else if(static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
    {
      QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
      spinBox->setDecimals(2);
      spinBox->setSingleStep(0.1);
      if(name == "opacity")
      {
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1.0);
      }
      
      return spinBox;
    }

    else if(data.type() == QVariant::StringList)
    {
      QStringList entries = data.value<QStringList>();
      QComboBox* comboBox = new QComboBox(parent);
      comboBox->setEditable(false);
      comboBox->addItems(entries);

      return comboBox;
    }

    
    else
      return QStyledItemDelegate::createEditor(parent, option, index);

  }
  else
    return new QLabel(displayData.toString(), parent);

}

void QmitkPropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{

  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);

  if(data.isValid())
  {

    if(data.type() == QVariant::Color)
    {

      /*
      QPushButton *colorBtn = qobject_cast<QPushButton *>(editor);
      QColor qcol = data.value<QColor>();
      colorBtn->setPalette(QPalette(qcol));


      QColor result = QColorDialog::getColor(qcol);
      if(result.isValid())
      {
        colorBtn->setPalette(QPalette(result));
      }
*/

    }


/*
    else if(data.type() == QVariant::Bool)
    {
      QCheckBox *visibilityCheckBox = qobject_cast<QCheckBox *>(editor);
      visibilityCheckBox->setChecked(data.toBool());
    }*/

    /*else*/ if(data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = qobject_cast<QSpinBox *>(editor);
      spinBox->setValue(data.toInt());
    }
    // see qt documentation. cast is correct, it would be obsolete if we 
    // store doubles
    else if(static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
    {
      QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
      spinBox->setValue(data.toDouble());
    }

    else if(data.type() == QVariant::StringList)
    {
      QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
      QString displayString = displayData.value<QString>();
      comboBox->setCurrentIndex(comboBox->findData(displayString));

//       connect(comboBox, SIGNAL(currentIndexChanged(int)),
//         this, SLOT(ComboBoxCurrentIndexChanged(int)));
    }

    else
      return QStyledItemDelegate::setEditorData(editor, index);
  }
}

void QmitkPropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel* model
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);

  if(data.isValid())
  {

    if(data.type() == QVariant::Color)
    {
      QWidget *colorBtn = qobject_cast<QWidget *>(editor);
      QVariant colorVariant;
      colorVariant.setValue<QColor>(colorBtn->palette().color(QPalette::Button));
      model->setData(index, colorVariant);
    }

    else if(data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = qobject_cast<QSpinBox *>(editor);
      int intValue = spinBox->value();

      QVariant intValueVariant;
      intValueVariant.setValue<float>(static_cast<float>(intValue));
      model->setData(index, intValueVariant);
    }

    else if(static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
    {
      QDoubleSpinBox* spinBox = qobject_cast<QDoubleSpinBox *>(editor);
      double doubleValue = spinBox->value();

      QVariant doubleValueVariant;
      doubleValueVariant.setValue<float>(static_cast<float>(doubleValue));
      model->setData(index, doubleValueVariant);
    }

    else if(data.type() == QVariant::StringList)
    {
      QString displayData = data.value<QString>();

      QComboBox* comboBox = qobject_cast<QComboBox *>(editor);
      QString comboBoxValue = comboBox->currentText();

      QVariant comboBoxValueVariant;
      comboBoxValueVariant.setValue<QString>(comboBoxValue);
      model->setData(index, comboBoxValueVariant);
    }

    else
      QStyledItemDelegate::setModelData(editor, model, index);
  }

}

void QmitkPropertyDelegate::commitAndCloseEditor()
{
  QWidget* editor = 0;
  if(QPushButton *pushBtn = qobject_cast<QPushButton *>(sender()))
  {
/*
    QColor result = QColorDialog::getColor(pushBtn->palette().color(QPalette::Window));
    if(result.isValid())
    {
      QPalette palette = pushBtn->palette();
      palette.setColor(QPalette::Window, result);
      pushBtn->setPalette(palette);
    }*/

    editor = pushBtn;
  }


/*
  else if(QCheckBox *chkBox = qobject_cast<QCheckBox *>(sender()))
  {
    editor = chkBox;
  }*/


  if(editor)
  {
    emit commitData(editor);
    emit closeEditor(editor);
  }

}

void QmitkPropertyDelegate::updateEditorGeometry(QWidget *editor,
                                                    const QStyleOptionViewItem &option,
                                                    const QModelIndex &index) const
{
/*
  QRect rect = option.rect;

  if (QCheckBox* checkBoxEditor = qobject_cast<QCheckBox*>(editor))
  {
    const QStyle *style = QApplication::style();

    const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &option);
    const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &option);
    rect = QRect(option.rect.x()+ option.rect.width()/2-indicatorWidth/2,
      option.rect.y()+ option.rect.height()/2-indicatorHeight/2,
      indicatorWidth, indicatorHeight);
  }
*/

  editor->setGeometry(option.rect);
}

void QmitkPropertyDelegate::ComboBoxCurrentIndexChanged( int index )
{
  if(QComboBox *comboBox = qobject_cast<QComboBox *>(sender()))
  {
    emit commitData(comboBox);
    emit closeEditor(comboBox);
  }
}

void QmitkPropertyDelegate::SpinBoxValueChanged( const QString& value )
{
  QAbstractSpinBox *spinBox = 0;
  if((spinBox = qobject_cast<QSpinBox *>(sender()))
    || (spinBox = qobject_cast<QDoubleSpinBox *>(sender())))
  {
    emit commitData(spinBox);
    emit closeEditor(spinBox);
  }
}

void QmitkPropertyDelegate::showColorDialog()
{

}
