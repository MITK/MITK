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

#include "QmitkFileReaderWriterOptionsWidget.h"

#include <QFormLayout>

#include <limits>

static QString GetAnyWidgetLabel(const std::string& name)
{
  QString label;
  // get the last segment from the option name and use it as the label
  std::string::size_type pos = name.find_last_of('.');
  if (pos == std::string::npos)
  {
    label = QString::fromStdString(name);
  }
  else if (pos < name.length() - 1)
  {
    label = QString::fromStdString(name.substr(pos + 1));
  }
  return label;
}

static QWidget* GetAnyWidget(const std::string& name, const us::Any& any, const std::string& defaultValue)
{
  const std::type_info& anyType = any.Type();

  if (anyType == typeid(std::string))
  {
    return new QmitkAnyStringWidget(name, any);
  }
  else if (anyType == typeid(std::vector<std::string>))
  {
    return new QmitkAnyVectorWidget(name, any, QString::fromStdString(defaultValue));
  }
  //      else if (anyType == typeid(std::list<std::string>))
  //      {
  //        const std::list<std::string>& list = ref_any_cast<std::list<std::string> >(m_Any);
  //        for (std::list<std::string>::const_iterator it = list.begin();
  //             it != list.end(); ++it)
  //        {
  //        }
  //      }
  else if (anyType == typeid(bool))
  {
    return new QmitkAnyBoolWidget(name, any);
  }
  else if (anyType == typeid(short))
  {
    return new QmitkAnyShortWidget(name, any);
  }
  else if (anyType == typeid(int))
  {
    return new QmitkAnyIntWidget(name, any);
  }
  else if (anyType == typeid(unsigned short))
  {
    return new QmitkAnyUShortWidget(name, any);
  }
  else if (anyType == typeid(unsigned int))
  {
    return new QmitkAnyUIntWidget(name, any);
  }
  else if (anyType == typeid(float))
  {
    return new QmitkAnyFloatWidget(name, any);
  }
  else if (anyType == typeid(double))
  {
    return new QmitkAnyDoubleWidget(name, any);
  }
  else
  {
    return new QmitkInvalidAnyWidget(name, any);
  }
}

QmitkFileReaderWriterOptionsWidget::QmitkFileReaderWriterOptionsWidget(const Options& options, QWidget* parent)
  : QWidget(parent)
{
  Options filteredOptions = options;
  std::map<std::string, std::string> optionToDefaultValue;
  for(Options::const_iterator iter = options.begin(), iterEnd = options.end();
      iter != iterEnd; ++iter)
  {
    const std::string& name = iter->first;
    if (name.size() > 4 && name.substr(name.size() - 4) == "enum")
    {
      filteredOptions.erase(name);

      std::string nameWithoutEnum = name.substr(0, name.size() - 5);
      us::Any value = filteredOptions[nameWithoutEnum];
      if (!value.Empty())
      {
        optionToDefaultValue[nameWithoutEnum] = value.ToString();
      }
      filteredOptions[nameWithoutEnum] = iter->second;
    }
  }

  QFormLayout* formLayout = new QFormLayout();
  for(Options::const_iterator iter = filteredOptions.begin(), iterEnd = filteredOptions.end();
      iter != iterEnd; ++iter)
  {
    formLayout->addRow(GetAnyWidgetLabel(iter->first),
                       GetAnyWidget(iter->first, iter->second, optionToDefaultValue[iter->first]));
  }

  this->setLayout(formLayout);
}

QmitkFileReaderWriterOptionsWidget::Options QmitkFileReaderWriterOptionsWidget::GetOptions() const
{
  Options options;

  QFormLayout* layout = qobject_cast<QFormLayout*>(this->layout());
  const int rows = layout->rowCount();
  for(int i = 0; i < rows; ++i)
  {
    QmitkAnyAdapter* anyAdapter = dynamic_cast<QmitkAnyAdapter*>(layout->itemAt(i, QFormLayout::FieldRole)->widget());
    if (anyAdapter)
    {
      options.insert(std::make_pair(anyAdapter->GetName(), anyAdapter->GetAny()));
    }
  }
  return options;
}


QmitkAnyStringWidget::QmitkAnyStringWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QLineEdit(parent)
  , QmitkAnyAdapter(name)
{
  this->setText(QString::fromStdString(any.ToString()));
}

us::Any QmitkAnyStringWidget::GetAny() const
{
  return us::Any(this->text().toStdString());
}


QmitkAnyBoolWidget::QmitkAnyBoolWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QCheckBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setChecked(us::any_cast<bool>(any));
}

us::Any QmitkAnyBoolWidget::GetAny() const
{
  return us::Any(this->isChecked());
}


QmitkAnyShortWidget::QmitkAnyShortWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<short>::min(), std::numeric_limits<short>::max());
  this->setValue(us::any_cast<short>(any));
}

us::Any QmitkAnyShortWidget::GetAny() const
{
  return us::Any(static_cast<short>(this->value()));
}


QmitkAnyUShortWidget::QmitkAnyUShortWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<unsigned short>::min(), std::numeric_limits<unsigned short>::max());
  this->setValue(us::any_cast<unsigned short>(any));
}

us::Any QmitkAnyUShortWidget::GetAny() const
{
  return us::Any(static_cast<unsigned short>(this->value()));
}


QmitkAnyIntWidget::QmitkAnyIntWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
  this->setValue(us::any_cast<int>(any));
}

us::Any QmitkAnyIntWidget::GetAny() const
{
  return us::Any(this->value());
}


QmitkAnyUIntWidget::QmitkAnyUIntWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max());
  this->setValue(us::any_cast<unsigned int>(any));
}

us::Any QmitkAnyUIntWidget::GetAny() const
{
  return us::Any(static_cast<unsigned int>(this->value()));
}


QmitkAnyFloatWidget::QmitkAnyFloatWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QDoubleSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
  this->setValue(static_cast<double>(us::any_cast<float>(any)));
}

us::Any QmitkAnyFloatWidget::GetAny() const
{
  return us::Any(static_cast<float>(this->value()));
}


QmitkAnyDoubleWidget::QmitkAnyDoubleWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QDoubleSpinBox(parent)
  , QmitkAnyAdapter(name)
{
  this->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
  this->setValue(us::any_cast<double>(any));
}

us::Any QmitkAnyDoubleWidget::GetAny() const
{
  return us::Any(this->value());
}


QmitkInvalidAnyWidget::QmitkInvalidAnyWidget(const std::string& name, const us::Any& any, QWidget* parent)
  : QLabel(parent)
  , QmitkAnyAdapter(name)
{
  this->setText(QString("Unknown option type '%1'").arg(any.Type().name()));
}

us::Any QmitkInvalidAnyWidget::GetAny() const
{
  return us::Any();
}

QmitkAnyVectorWidget::QmitkAnyVectorWidget(const std::string& name, const us::Any& any, const QString& defaultValue, QWidget* parent)
  : QComboBox(parent)
  , QmitkAnyAdapter(name)
{
  const std::vector<std::string>& entries = us::ref_any_cast<std::vector<std::string> >(any);
  int index = 0;
  int defaultIndex = 0;
  for (std::vector<std::string>::const_iterator iter = entries.begin(), iterEnd = entries.end();
       iter != iterEnd; ++iter, ++index)
  {
    QString item = QString::fromStdString(*iter);
    this->addItem(item);
    if (item == defaultValue)
    {
      defaultIndex = index;
    }
  }
  this->setCurrentIndex(defaultIndex);
}

us::Any QmitkAnyVectorWidget::GetAny() const
{
  return us::Any(std::string(this->currentText().toStdString()));
}
