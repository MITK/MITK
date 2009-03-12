#include "QmitkPropertyDelegate.h"

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


QmitkPropertyDelegate::QmitkPropertyDelegate(QObject *parent)
{
}

void QmitkPropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
  std::bitset<2> paintFlags;

  if(index.column() == 1)
  {
    //Get item data
    QVariant data = index.data(Qt::DisplayRole);

    if (option.state & QStyle::State_Selected)
    {
      QPalette::ColorGroup cg = option.state & QStyle::State_HasFocus
        ? QPalette::Normal : QPalette::Disabled;
      painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }    

    // draw a coloured button
    if(data.type() == QVariant::Color)
    {
      QColor qcol = data.value<QColor>();

      painter->save();
      const QStyle *style = QApplication::style();
      QStyleOptionButton opt;
      opt.state |= QStyle::State_Raised |QStyle::State_Enabled;
      opt.features |= QStyleOptionButton::DefaultButton;
      opt.palette = QPalette(qcol);
      opt.rect = option.rect;

      // draw item data as CheckBox
      style->drawControl(QStyle::CE_PushButton,&opt,painter);
      painter->restore();

      paintFlags.set(0, true);
    }

/*
    else if(data.type() == QVariant::Bool)
    {
      bool boolValue = data.value<bool>();

      const QStyle *style = QApplication::style();
      QStyleOptionButton opt;
      opt.state |= boolValue ? QStyle::State_On : QStyle::State_Off;
      opt.state |= QStyle::State_Enabled;
      opt.rect = option.rect;

      const int indicatorWidth = style->pixelMetric(QStyle::PM_IndicatorWidth, &opt);
      const int indicatorHeight = style->pixelMetric(QStyle::PM_IndicatorHeight, &opt);
      opt.rect = QRect(opt.rect.x()+ opt.rect.width()/2-indicatorWidth/2,
        opt.rect.y()+ opt.rect.height()/2-indicatorHeight/2,
        indicatorWidth, indicatorHeight); 

      // draw item data as CheckBox
      style->drawControl(QStyle::CE_CheckBox,&opt,painter);

      paintFlags.set(1, true);
    }*/


  }

  if(paintFlags.none())
  {
    QItemDelegate::paint(painter, option, index);
  }
}

QWidget* QmitkPropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option
  , const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);
  QVariant displayData = index.data(Qt::DisplayRole);

  if(data.isValid())
  {
    if(data.type() == QVariant::Color)
    {
      QPushButton* colorBtn = new QPushButton(parent);
      QColor color = data.value<QColor>();

      QColor result = QColorDialog::getColor(color);
      if(result.isValid())
      {
        colorBtn->setPalette(QPalette(result));
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
    }
*/
    else if(data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = new QSpinBox(parent);
      spinBox->setSingleStep(1);
    }

    else if(data.type() == QMetaType::Float)
    {
      QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
      spinBox->setDecimals(2);
      spinBox->setSingleStep(0.1);
      return spinBox;
    }

    else if(data.type() == QVariant::StringList)
    {
      QStringList entries = data.value<QStringList>();
      QComboBox* comboBox = new QComboBox(parent);
      comboBox->setEditable(false);
      comboBox->addItems(entries);

      return(comboBox);
    }

    else
      return QItemDelegate::createEditor(parent, option, index);

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
      //QPushButton *colorBtn = qobject_cast<QPushButton *>(editor);
      //QColor qcol = data.value<QColor>();
      //colorBtn->setPalette(QPalette(qcol));

/*
      QColor result = QColorDialog::getColor(qcol);
      if(result.isValid())
      {
        colorBtn->setPalette(QPalette(result));
      }*/

    }

/*
    else if(data.type() == QVariant::Bool)
    {
      QCheckBox *visibilityCheckBox = qobject_cast<QCheckBox *>(editor);
      visibilityCheckBox->setChecked(data.toBool());
    }*/

    else if(data.type() == QVariant::Int)
    {
      QSpinBox* spinBox = qobject_cast<QSpinBox *>(editor);
      spinBox->setValue(data.toInt());
    }

    else if(data.type() == QMetaType::Float)
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
      return QItemDelegate::setEditorData(editor, index);
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
      QPushButton *colorBtn = qobject_cast<QPushButton *>(editor);
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

    else if(data.type() == QMetaType::Float)
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
      QItemDelegate::setModelData(editor, model, index);
  }

}

void QmitkPropertyDelegate::commitAndCloseEditor()
{
  QWidget* editor = 0;
  if(QPushButton *pushBtn = qobject_cast<QPushButton *>(sender()))
  {
    QColor result = QColorDialog::getColor(pushBtn->palette().color(QPalette::Button));
    if(result.isValid())
    {
      pushBtn->setPalette(QPalette(result));
      editor = pushBtn;
    }
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