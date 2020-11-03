/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNewSegmentationDialog.h"

#include <itkRGBPixel.h>

#include <QAbstractItemModel>
#include <QColorDialog>
#include <QStringListModel>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(QWidget *parent)
  : QDialog(parent) // true, modal
  , selectedOrgan(tr("undefined"))
  , newOrganEntry(false)
{
  QDialog::setFixedSize(250, 105);

  QBoxLayout *verticalLayout = new QVBoxLayout(this);
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  // to enter a name for the segmentation
  lblPrompt = new QLabel(tr("Name and color of the segmentation"), this);
  verticalLayout->addWidget(lblPrompt);

  // to choose a color
  btnColor = new QPushButton("", this);
  btnColor->setFixedWidth(25);
  btnColor->setAutoFillBackground(true);
  btnColor->setStyleSheet("background-color:rgb(255,0,0)");

  connect(btnColor, SIGNAL(clicked()), this, SLOT(onColorBtnClicked()));

  lineEditName = new QLineEdit("", this);
  QStringList completionList;
  completionList << "";
  completer = new QCompleter(completionList);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  lineEditName->setCompleter(completer);

  connect(completer, SIGNAL(activated(const QString &)), this, SLOT(onColorChange(const QString &)));

  QBoxLayout *horizontalLayout2 = new QHBoxLayout();
  verticalLayout->addLayout(horizontalLayout2);
  horizontalLayout2->addWidget(btnColor);
  horizontalLayout2->addWidget(lineEditName);

  // buttons for closing the dialog
  btnOk = new QPushButton(tr("Ok"), this);
  btnOk->setDefault(true);
  connect(btnOk, SIGNAL(clicked()), this, SLOT(onAcceptClicked()));

  QPushButton *btnCancel = new QPushButton(tr("Cancel"), this);
  connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

  QBoxLayout *horizontalLayout = new QHBoxLayout();
  verticalLayout->addLayout(horizontalLayout);
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget(btnOk);
  horizontalLayout->addWidget(btnCancel);

  lineEditName->setFocus();
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

const QString QmitkNewSegmentationDialog::GetSegmentationName()
{
  return m_SegmentationName;
}

mitk::Color QmitkNewSegmentationDialog::GetColor()
{
  mitk::Color colorProperty;
  if (m_Color.spec() == 0)
  {
    colorProperty.SetRed(1);
    colorProperty.SetGreen(0);
    colorProperty.SetBlue(0);
  }
  else
  {
    colorProperty.SetRed(m_Color.redF());
    colorProperty.SetGreen(m_Color.greenF());
    colorProperty.SetBlue(m_Color.blueF());
  }
  return colorProperty;
}

const char *QmitkNewSegmentationDialog::GetOrganType()
{
  return selectedOrgan.toLocal8Bit().constData();
}

void QmitkNewSegmentationDialog::SetSegmentationName(const QString &segmentationName)
{
  lineEditName->setText(segmentationName);
}

void QmitkNewSegmentationDialog::SetColor(const mitk::Color &color)
{
  m_Color.setRedF(color.GetRed());
  m_Color.setGreenF(color.GetGreen());
  m_Color.setBlueF(color.GetBlue());

  btnColor->setStyleSheet(QString("background-color:rgb(%1, %2, %3)").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
}

void QmitkNewSegmentationDialog::SetSuggestionList(QStringList organColorList)
{
  QStringList::iterator iter;
  for (iter = organColorList.begin(); iter != organColorList.end(); ++iter)
  {
    QString &element = *iter;
    QString colorName = element.right(7);
    QColor color(colorName);
    QString organName = element.left(element.size() - 7);

    organList.push_back(organName);
    colorList.push_back(color);
  }

  QStringListModel *completeModel = static_cast<QStringListModel *>(completer->model());
  completeModel->setStringList(organList);
}

void QmitkNewSegmentationDialog::setPrompt(const QString &prompt)
{
  lblPrompt->setText(prompt);
}

void QmitkNewSegmentationDialog::onAcceptClicked()
{
  m_SegmentationName = lineEditName->text();
  accept();
}

void QmitkNewSegmentationDialog::onNewOrganNameChanged(const QString &newText)
{
  if (!newText.isEmpty())
  {
    btnOk->setEnabled(true);
  }

  selectedOrgan = newText;
  SetSegmentationName(newText);
}

void QmitkNewSegmentationDialog::onColorBtnClicked()
{
  m_Color = QColorDialog::getColor();
  if (m_Color.spec() == 0)
  {
    m_Color.setRed(255);
    m_Color.setGreen(0);
    m_Color.setBlue(0);
  }
  btnColor->setStyleSheet(QString("background-color:rgb(%1, %2, %3)").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
}

void QmitkNewSegmentationDialog::onColorChange(const QString &completedWord)
{
  if (organList.contains(completedWord))
  {
    int j = organList.indexOf(completedWord);
    m_Color = colorList.at(j);
    btnColor->setStyleSheet(QString("background-color:rgb(%1, %2, %3)").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
  }
}
