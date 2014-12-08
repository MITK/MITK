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

#include "QmitkAddNewPropertyDialog.h"
#include <mitkProperties.h>
#include <QMessageBox>
#include <cassert>

QmitkAddNewPropertyDialog::QmitkAddNewPropertyDialog(mitk::DataNode::Pointer dataNode, mitk::BaseRenderer::Pointer renderer, QWidget* parent)
  : QDialog(parent),
    m_DataNode(dataNode),
    m_Renderer(renderer)
{
  m_Controls.setupUi(this);

  QStringList types;
  types << "bool" << "double" << "float" << "int" << "string";

  m_Controls.typeComboBox->addItems(types);

  connect(m_Controls.typeComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ShowAdequateValueWidget(const QString&)));
  connect(m_Controls.addButton, SIGNAL(clicked()), this, SLOT(AddNewProperty()));
  connect(m_Controls.cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  this->ShowAdequateValueWidget(types[0]);
}

QmitkAddNewPropertyDialog::~QmitkAddNewPropertyDialog()
{
}

void QmitkAddNewPropertyDialog::AddNewProperty()
{
  if (m_Controls.nameLineEdit->text().isEmpty())
  {
    QMessageBox::critical(this, "No name specified", "Enter a property name.");
    return;
  }

  if (!this->ValidateValue())
  {
    QMessageBox::critical(this, "Invalid value", "Enter a valid " + m_Controls.typeComboBox->currentText() + " value.");
    return;
  }

  m_DataNode->SetProperty(m_Controls.nameLineEdit->text().toLatin1(), this->CreateProperty(), m_Renderer);

  this->accept();
}

mitk::BaseProperty::Pointer QmitkAddNewPropertyDialog::CreateProperty() const
{
  QString type = m_Controls.typeComboBox->currentText();

  if (type == "bool")
  {
    return mitk::BoolProperty::New(m_Controls.valueCheckBox->isChecked()).GetPointer();
  }
  else if (type == "double")
  {
    return mitk::DoubleProperty::New(m_Controls.valueLineEdit->text().toDouble()).GetPointer();
  }
  else if (type == "float")
  {
    return mitk::FloatProperty::New(m_Controls.valueLineEdit->text().toFloat()).GetPointer();
  }
  else if (type == "int")
  {
    return mitk::IntProperty::New(m_Controls.valueLineEdit->text().toInt()).GetPointer();
  }
  else if (type == "string")
  {
    return mitk::StringProperty::New(m_Controls.valueLineEdit->text().toStdString()).GetPointer();
  }
  else
  {
    assert(false && "Property creation for selected type not implemented!");
  }

  return NULL;
}

bool QmitkAddNewPropertyDialog::ValidateValue()
{
  QString type = m_Controls.typeComboBox->currentText();

  if (type == "bool")
  {
    return true;
  }
  else if (type == "double")
  {
    bool ok = false;
    m_Controls.valueLineEdit->text().toDouble(&ok);

    return ok;
  }
  else if (type == "float")
  {
    bool ok = false;
    m_Controls.valueLineEdit->text().toFloat(&ok);

    return ok;
  }
  else if (type == "int")
  {
    bool ok = false;
    m_Controls.valueLineEdit->text().toInt(&ok);

    return ok;
  }
  else if (type == "string")
  {
    return true;
  }
  else
  {
    assert(false && "Value validation for selected type not implemented!");
  }

  return false;
}

void QmitkAddNewPropertyDialog::ShowAdequateValueWidget(const QString& type)
{
  m_Controls.valueLineEdit->clear();
  m_Controls.valueLineEdit->hide();

  m_Controls.valueCheckBox->setChecked(false);
  m_Controls.valueCheckBox->hide();

  if (type == "bool")
  {
    m_Controls.valueCheckBox->show();
  }
  else if (type == "double")
  {
    m_Controls.valueLineEdit->setText("0");
    m_Controls.valueLineEdit->show();
  }
  else if (type == "float")
  {
    m_Controls.valueLineEdit->setText("0");
    m_Controls.valueLineEdit->show();
  }
  else if (type == "int")
  {
    m_Controls.valueLineEdit->setText("0");
    m_Controls.valueLineEdit->show();
  }
  else if (type == "string")
  {
    m_Controls.valueLineEdit->show();
  }
  else
  {
    assert(false && "No adequate value widget specified for selected type!");
  }
}