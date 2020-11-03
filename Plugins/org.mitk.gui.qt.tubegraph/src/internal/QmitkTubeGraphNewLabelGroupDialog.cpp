/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTubeGraphNewLabelGroupDialog.h"

#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qtreewidget.h>
#include <qheaderview.h>

#include "mitkTubeGraphDefaultLabelGroups.h"


QmitkTubeGraphNewLabelGroupDialog::QmitkTubeGraphNewLabelGroupDialog(QWidget* parent)
:QDialog(parent)
,m_NewLabelGroup()
{
  QDialog::setFixedSize(200, 200);

  layout = new QVBoxLayout(this);
  layout->setMargin(5);
  layout->setSpacing(5);

  newLabelGroupButton = new QPushButton("Create new label group", this);
  layout->addWidget(newLabelGroupButton,  0, Qt::AlignHCenter);
  connect( newLabelGroupButton, SIGNAL(clicked()), this, SLOT(OnCreateNewLabelGroup()) );

  standardLabelGroupButton = new QPushButton("Add standard label group", this);
  layout->addWidget(standardLabelGroupButton, 0, Qt::AlignHCenter);
  connect( standardLabelGroupButton, SIGNAL(clicked()), this, SLOT(OnAddStandardLabelGroup()) );

  spacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Minimum );
  layout->addSpacerItem(spacer);

  buttonLayout = new QHBoxLayout();

  okButton = new QPushButton("Ok", this);
  buttonLayout->addWidget(okButton, 0, Qt::AlignRight);
  okButton->setVisible(false);

  cancleButton = new QPushButton("Cancle", this);
  buttonLayout->addWidget(cancleButton,  0, Qt::AlignRight);
  connect( cancleButton, SIGNAL(clicked()), this, SLOT(reject()) );

  layout->addLayout(buttonLayout);

  newLabelGroupButton->setFocus();

}

QmitkTubeGraphNewLabelGroupDialog::~QmitkTubeGraphNewLabelGroupDialog()
{
  //delete layout;
  ////delete buttonLayout;
  //delete spacer;
  //delete okButton;
  //delete cancleButton;

  //delete labelGroupDescriptionLabel;
 /* delete labelGroupLineEdit;

  delete labelDescriptionLabel;
  delete labelLineEdit;
  delete addLabelButton;
  delete labelListWidget; */
  //delete labelGroupTreeWidget;
}

void QmitkTubeGraphNewLabelGroupDialog::OnCreateNewLabelGroup()
{
  QDialog::setFixedSize(400, 200);

  layout->removeWidget(newLabelGroupButton);
  delete newLabelGroupButton;

  layout->removeWidget(standardLabelGroupButton);
  delete standardLabelGroupButton;

  layout->removeWidget(okButton);
  layout->removeWidget(cancleButton);

  layout->removeItem(spacer);
  layout->removeItem(buttonLayout);

  labelGroupDescriptionLabel = new QLabel("Enter the label group name!", this);
  layout->addWidget(labelGroupDescriptionLabel);

  labelGroupLineEdit = new QLineEdit(this);
  layout->addWidget(labelGroupLineEdit);

  labelDescriptionLabel = new QLabel("Enter a label name!", this);
  layout->addWidget(labelDescriptionLabel);


  auto   labelLayout = new QHBoxLayout();

  labelLineEdit = new QLineEdit(this);
  labelLayout->addWidget(labelLineEdit);

  addLabelButton = new QPushButton("+", this);
  labelLayout->addWidget(addLabelButton);
  connect( addLabelButton, SIGNAL(clicked()), this, SLOT(OnAddingLabel()) );

  layout->addLayout(labelLayout);

  labelListWidget = new QListWidget(this);
  labelListWidget->setDragDropMode(QAbstractItemView::InternalMove);
  labelListWidget->addItem("Undefined");
  layout->addWidget(labelListWidget);

  layout->addLayout(buttonLayout);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnAddingNewLabelGroup()) );

  okButton->setVisible(true);
}

void QmitkTubeGraphNewLabelGroupDialog::OnAddStandardLabelGroup()
{
  QDialog::setFixedSize(400, 200);

  layout->removeWidget(newLabelGroupButton);
  delete newLabelGroupButton;

  layout->removeWidget(standardLabelGroupButton);
  delete standardLabelGroupButton;

  layout->removeWidget(okButton);
  layout->removeWidget(cancleButton);

  layout->removeItem(spacer);
  layout->removeItem(buttonLayout);

  labelGroupDescriptionLabel = new QLabel("Choose one of the standard label group!", this);
  layout->addWidget(labelGroupDescriptionLabel);

  labelGroupTreeWidget = new QTreeWidget(this);
  labelGroupTreeWidget->header()->close();


  auto   defaultLabelGroups = new mitk::TubeGraphDefaultLabelGroups();

  m_LabelGroupsLiver = defaultLabelGroups->GetLabelGroupForLiver();
  m_LabelGroupsLung = defaultLabelGroups->GetLabelGroupForLung();

  auto   liverItem = new QTreeWidgetItem(labelGroupTreeWidget);
  liverItem->setText(0, tr("Liver"));
  QTreeWidgetItem* liverChildItem;
  for (unsigned int k = 0; k < m_LabelGroupsLiver.size(); k++)
  {
    liverChildItem = new QTreeWidgetItem(liverItem);
    liverChildItem->setText(0, QString::fromStdString(m_LabelGroupsLiver.at(k)->labelGroupName));
  }
  liverItem->setExpanded(true);

  auto   lungItem = new QTreeWidgetItem(labelGroupTreeWidget);
  lungItem->setText(0, tr("Lung"));
  QTreeWidgetItem* lungChildItem;
  for (unsigned int k = 0; k < m_LabelGroupsLung.size(); k++)
  {
    lungChildItem = new QTreeWidgetItem(lungItem);
    lungChildItem->setText(0, QString::fromStdString(m_LabelGroupsLung.at(k)->labelGroupName));
  }
  lungItem->setExpanded(true);


  labelGroupTreeWidget->insertTopLevelItem(1,lungItem);

  delete defaultLabelGroups;

  layout->addWidget(labelGroupTreeWidget);

  layout->addLayout(buttonLayout);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnAddingStandardLabelGroup()) );

  okButton->setVisible(true);

}

void QmitkTubeGraphNewLabelGroupDialog::OnAddingLabel()
{
  if (labelLineEdit->text().isEmpty())
  {
    labelLineEdit->setStyleSheet("border: 1px solid red");
    return;
  }

  labelListWidget->addItem(labelLineEdit->text());
  labelLineEdit->clear();
}

void QmitkTubeGraphNewLabelGroupDialog::OnAddingNewLabelGroup()
{
  if (labelGroupLineEdit->text().isEmpty())
  {
    labelGroupLineEdit->setStyleSheet("border: 1px solid red");
    return;
  }
  m_NewLabelGroup   = new LabelGroupType();
  m_NewLabelGroup->labelGroupName = (labelGroupLineEdit->text()).toStdString();

  for (int i =0; i < labelListWidget->count(); i++)
  {
    auto   label = new LabelType();
    label->labelName = (labelListWidget->item(i)->text()).toStdString();
    mitk::Color color;

    if(label->labelName.compare("Undefined") == 0)
    {
      color[0] = 170; color[1] = 170; color[2] = 169;
    }
    else
    {
      color[0] = rand() % 255; color[1] = rand() % 255; color[2] = rand() % 255;
    }
    label->labelColor = color;
    label->isVisible = true;
    m_NewLabelGroup->labels.push_back(label);
  }

  this->accept();
}

void QmitkTubeGraphNewLabelGroupDialog::OnAddingStandardLabelGroup()
{
  if (labelGroupTreeWidget->selectedItems().isEmpty())
  {
    labelGroupDescriptionLabel->setStyleSheet("border: 1px solid red");
    return;
  }

  if(labelGroupTreeWidget->selectedItems().at(0)->parent()->text(0) == "Lung")
  {
    for (unsigned int k = 0; k < m_LabelGroupsLung.size(); k++)
    {
      if(m_LabelGroupsLung.at(k)->labelGroupName == (labelGroupTreeWidget->selectedItems().at(0)->text(0)).toStdString())
        m_NewLabelGroup = m_LabelGroupsLung.at(k);
    }
  }
  else
  {
    for (unsigned int k = 0; k < m_LabelGroupsLiver.size(); k++)
    {
      if(m_LabelGroupsLiver.at(k)->labelGroupName == (labelGroupTreeWidget->selectedItems().at(0)->text(0)).toStdString())
        m_NewLabelGroup = m_LabelGroupsLiver.at(k);
    }
  }
  if (m_NewLabelGroup == nullptr)
    return;


  this->accept();

}

QmitkTubeGraphNewLabelGroupDialog::LabelGroupType* QmitkTubeGraphNewLabelGroupDialog::GetLabelGroup()
{
  return m_NewLabelGroup;
}
