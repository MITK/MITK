/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNewSegmentationDialog.h"
#include <QmitkSegmentationOrganNamesHandling.h>

#include <ui_QmitkNewSegmentationDialog.h>

#include <QColorDialog>
#include <QCompleter>
#include <QPushButton>
#include <QStringListModel>

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(QWidget *parent, Mode mode)
  : QDialog(parent),
    m_Ui(new Ui::QmitkNewSegmentationDialog),
    m_Color(Qt::red)
{
  m_Ui->setupUi(this);

  if (RenameLabel == mode)
  {
    this->setWindowTitle("Rename Label");
    m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Rename label");
  }
  else
  {
    m_Ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Create label");
  }

  auto* completer = new QCompleter(QStringList());
  completer->setCaseSensitivity(Qt::CaseInsensitive);

  m_Ui->nameLineEdit->setCompleter(completer);
  m_Ui->nameLineEdit->setFocus();

  connect(completer, qOverload<const QString&>(&QCompleter::activated), this, qOverload<const QString&>(&QmitkNewSegmentationDialog::OnSuggestionSelected));
  connect(m_Ui->colorButton, &QToolButton::clicked, this, &QmitkNewSegmentationDialog::OnColorButtonClicked);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkNewSegmentationDialog::OnAccept);

  this->UpdateColorButtonBackground();
  this->SetSuggestionList(mitk::OrganNamesHandling::GetDefaultOrganColorString());
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

void QmitkNewSegmentationDialog::UpdateColorButtonBackground()
{
  m_Ui->colorButton->setStyleSheet("background-color:" + m_Color.name());
}

QString QmitkNewSegmentationDialog::GetName() const
{
  return m_Name;
}

mitk::Color QmitkNewSegmentationDialog::GetColor() const
{
  mitk::Color color;

  if (m_Color.isValid())
  {
    color.SetRed(m_Color.redF());
    color.SetGreen(m_Color.greenF());
    color.SetBlue(m_Color.blueF());
  }
  else
  {
    color.Set(1.0f, 0.0f, 0.0f);
  }

  return color;
}

void QmitkNewSegmentationDialog::SetName(const QString& name)
{
  m_Ui->nameLineEdit->setText(name);
}

void QmitkNewSegmentationDialog::SetColor(const mitk::Color& color)
{
  m_Color.setRgbF(color.GetRed(), color.GetGreen(), color.GetBlue());
  this->UpdateColorButtonBackground();
}

void QmitkNewSegmentationDialog::SetSuggestionList(const QStringList& suggestionList)
{
  for (const auto& suggestion : suggestionList)
  {
    m_NameSuggestions.push_back(suggestion.left(suggestion.length() - 7));
    m_ColorSuggestions.push_back(suggestion.right(7));
  }

  auto* completerModel = static_cast<QStringListModel*>(m_Ui->nameLineEdit->completer()->model());
  completerModel->setStringList(m_NameSuggestions);
}

void QmitkNewSegmentationDialog::OnAccept()
{
  m_Name = m_Ui->nameLineEdit->text();
  this->accept();
}

void QmitkNewSegmentationDialog::OnColorButtonClicked()
{
  auto color = QColorDialog::getColor(m_Color);

  if (color.isValid())
  {
    m_Color = color;
    this->UpdateColorButtonBackground();
  }
}

void QmitkNewSegmentationDialog::OnSuggestionSelected(const QString &name)
{
  auto i = m_NameSuggestions.indexOf(name);

  if (-1 != i)
  {
    m_Color = m_ColorSuggestions[i];
    this->UpdateColorButtonBackground();
  }
}
