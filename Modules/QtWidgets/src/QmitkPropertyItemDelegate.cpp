/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPropertyItemDelegate.h"
#include "QmitkPropertyItemModel.h"
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QSpinBox>
#include <QToolButton>
#include <algorithm>
#include <mitkBaseProperty.h>
#include <mitkCoreServices.h>
#include <mitkFloatPropertyExtension.h>
#include <mitkIPropertyExtensions.h>
#include <mitkIntPropertyExtension.h>

QmitkColorWidget::QmitkColorWidget(QWidget *parent)
  : QWidget(parent), m_LineEdit(new QLineEdit), m_Button(new QToolButton)
{
  m_LineEdit->setText(m_Color.name());
  m_Button->setText("...");

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(m_LineEdit);
  layout->addWidget(m_Button);

  this->setFocusProxy(m_LineEdit);
  this->setLayout(layout);

  connect(m_LineEdit, SIGNAL(editingFinished()), this, SLOT(OnLineEditEditingFinished()));
  connect(m_Button, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
}

QmitkColorWidget::~QmitkColorWidget()
{
}

QColor QmitkColorWidget::GetColor() const
{
  return m_Color;
}

void QmitkColorWidget::SetColor(QColor color)
{
  m_Color = color;
  m_LineEdit->setText(color.name());
}

void QmitkColorWidget::OnLineEditEditingFinished()
{
  if (!QColor::isValidColor(m_LineEdit->text()))
    m_LineEdit->setText("#000000");

  m_Color.setNamedColor(m_LineEdit->text());
}

void QmitkColorWidget::OnButtonClicked()
{
  QColor color = QColorDialog::getColor(m_Color, QApplication::activeWindow());

  if (color.isValid())
  {
    this->SetColor(color);
    emit ColorPicked();
  }
}

QmitkComboBoxListView::QmitkComboBoxListView(QComboBox *comboBox) : m_ComboBox(comboBox)
{
}

QmitkComboBoxListView::~QmitkComboBoxListView()
{
}

void QmitkComboBoxListView::paintEvent(QPaintEvent *event)
{
  if (m_ComboBox != nullptr)
  {
    QStyleOptionComboBox option;

    option.initFrom(m_ComboBox);
    option.editable = m_ComboBox->isEditable();

    if (m_ComboBox->style()->styleHint(QStyle::SH_ComboBox_Popup, &option, m_ComboBox))
    {
      QStyleOptionMenuItem menuOption;
      menuOption.initFrom(this);
      menuOption.palette = this->palette();
      menuOption.state = QStyle::State_None;
      menuOption.checkType = QStyleOptionMenuItem::NotCheckable;
      menuOption.menuRect = event->rect();
      menuOption.maxIconWidth = 0;
      menuOption.tabWidth = 0;

      QPainter painter(this->viewport());
      m_ComboBox->style()->drawControl(QStyle::CE_MenuEmptyArea, &menuOption, &painter, this);
    }
  }

  QListView::paintEvent(event);
}

void QmitkComboBoxListView::resizeEvent(QResizeEvent *event)
{
  int width = this->viewport()->width();
  int height = this->contentsSize().height();

  this->resizeContents(width, height);

  QListView::resizeEvent(event);
}

QStyleOptionViewItem QmitkComboBoxListView::viewOptions() const
{
  QStyleOptionViewItem option = QListView::viewOptions();
  option.showDecorationSelected = true;

  if (m_ComboBox != nullptr)
    option.font = m_ComboBox->font();

  return option;
}

class PropertyEqualTo
{
public:
  PropertyEqualTo(const mitk::BaseProperty *property) : m_Property(property) {}
  bool operator()(const mitk::PropertyList::PropertyMapElementType &pair) const
  {
    return pair.second.GetPointer() == m_Property;
  }

private:
  const mitk::BaseProperty *m_Property;
};

QmitkPropertyItemDelegate::QmitkPropertyItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QmitkPropertyItemDelegate::~QmitkPropertyItemDelegate()
{
}

QWidget *QmitkPropertyItemDelegate::createEditor(QWidget *parent,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (data.isValid())
  {
    if (data.type() == QVariant::Int)
    {
      QSpinBox *spinBox = new QSpinBox(parent);

      mitk::CoreServicePointer<mitk::IPropertyExtensions> extensions(mitk::CoreServices::GetPropertyExtensions());
      std::string name = this->GetPropertyName(index);

      if (!name.empty() && extensions->HasExtension(name))
      {
        mitk::IntPropertyExtension::Pointer extension =
          dynamic_cast<mitk::IntPropertyExtension *>(extensions->GetExtension(name).GetPointer());

        if (extension.IsNotNull())
        {
          spinBox->setMinimum(extension->GetMinimum());
          spinBox->setMaximum(extension->GetMaximum());
          spinBox->setSingleStep(extension->GetSingleStep());
        }
      }

      connect(spinBox, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingFinished()));

      return spinBox;
    }

    if (data.type() == QVariant::Double || static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
    {
      QDoubleSpinBox *spinBox = new QDoubleSpinBox(parent);

      mitk::CoreServicePointer<mitk::IPropertyExtensions> extensions(mitk::CoreServices::GetPropertyExtensions());
      std::string name = this->GetPropertyName(index);

      if (!name.empty() && extensions->HasExtension(name))
      {
        mitk::FloatPropertyExtension::Pointer extension =
          dynamic_cast<mitk::FloatPropertyExtension *>(extensions->GetExtension(name).GetPointer());

        if (extension.IsNotNull())
        {
          spinBox->setMinimum(extension->GetMinimum());
          spinBox->setMaximum(extension->GetMaximum());
          spinBox->setSingleStep(extension->GetSingleStep());
          spinBox->setDecimals(extension->GetDecimals());
        }
      }
      else
      {
        spinBox->setSingleStep(0.1);
        spinBox->setDecimals(4);
      }

      if (name == "opacity") // TODO
      {
        spinBox->setMinimum(0.0);
        spinBox->setMaximum(1.0);
      }

      connect(spinBox, SIGNAL(editingFinished()), this, SLOT(OnSpinBoxEditingFinished()));

      return spinBox;
    }

    if (data.type() == QVariant::StringList)
    {
      QComboBox *comboBox = new QComboBox(parent);
      comboBox->setView(new QmitkComboBoxListView(comboBox));

      comboBox->addItems(data.toStringList());

      connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxCurrentIndexChanged(int)));

      return comboBox;
    }

    if (data.type() == QVariant::Color)
    {
      QmitkColorWidget *colorWidget = new QmitkColorWidget(parent);

      connect(colorWidget, SIGNAL(ColorPicked()), this, SLOT(OnColorPicked()));

      return colorWidget;
    }
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}

std::string QmitkPropertyItemDelegate::GetPropertyName(const QModelIndex &index) const
{
  if (!m_PropertyList.IsExpired())
  {
    mitk::BaseProperty *property =
      reinterpret_cast<mitk::BaseProperty *>(index.data(mitk::PropertyRole).value<void *>());
    const mitk::PropertyList::PropertyMap *propertyMap = m_PropertyList.Lock()->GetMap();

    mitk::PropertyList::PropertyMap::const_iterator it =
      std::find_if(propertyMap->begin(), propertyMap->end(), PropertyEqualTo(property));

    if (it != propertyMap->end())
      return it->first;
  }

  return "";
}

void QmitkPropertyItemDelegate::OnComboBoxCurrentIndexChanged(int)
{
  QComboBox *comboBox = qobject_cast<QComboBox *>(sender());

  emit commitData(comboBox);
  emit closeEditor(comboBox);
}

void QmitkPropertyItemDelegate::OnSpinBoxEditingFinished()
{
  QAbstractSpinBox *spinBox = qobject_cast<QAbstractSpinBox *>(sender());

  emit commitData(spinBox);
  emit closeEditor(spinBox);
}

void QmitkPropertyItemDelegate::OnColorPicked()
{
  QmitkColorWidget *colorWidget = qobject_cast<QmitkColorWidget *>(sender());

  emit commitData(colorWidget);
  emit closeEditor(colorWidget);
}

void QmitkPropertyItemDelegate::paint(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
  QVariant data = index.data();

  if (index.column() == 1 && data.type() == QVariant::Color)
  {
    painter->fillRect(option.rect, data.value<QColor>());
    return;
  }

  QStyledItemDelegate::paint(painter, option, index);
}

void QmitkPropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (!data.isValid())
    return;

  if (data.type() == QVariant::StringList)
  {
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    comboBox->setCurrentIndex(comboBox->findText(index.data().toString()));
  }
  if (data.type() == QVariant::Color)
  {
    QmitkColorWidget *colorWidget = qobject_cast<QmitkColorWidget *>(editor);
    colorWidget->SetColor(data.value<QColor>());
  }
  else
  {
    QStyledItemDelegate::setEditorData(editor, index);
  }
}

void QmitkPropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  QVariant data = index.data(Qt::EditRole);

  if (!data.isValid())
    return;

  if (data.type() == QVariant::Int)
  {
    QSpinBox *spinBox = qobject_cast<QSpinBox *>(editor);
    model->setData(index, spinBox->value());
  }
  else if (data.type() == QVariant::Double)
  {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    model->setData(index, spinBox->value());
  }
  else if (static_cast<QMetaType::Type>(data.type()) == QMetaType::Float)
  {
    QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *>(editor);
    model->setData(index, static_cast<float>(spinBox->value()));
  }
  else if (data.type() == QVariant::StringList)
  {
    QComboBox *comboBox = qobject_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentText());
  }
  else if (data.type() == QVariant::Color)
  {
    QmitkColorWidget *colorWidget = qobject_cast<QmitkColorWidget *>(editor);
    model->setData(index, colorWidget->GetColor());
  }
  else
  {
    QStyledItemDelegate::setModelData(editor, model, index);
  }
}

void QmitkPropertyItemDelegate::SetPropertyList(mitk::PropertyList *propertyList)
{
  if (m_PropertyList != propertyList)
    m_PropertyList = propertyList;
}
