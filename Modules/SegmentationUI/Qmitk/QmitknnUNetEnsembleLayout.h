/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnUNetEnsembleLayout_h
#define QmitknnUNetEnsembleLayout_h


#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QLabel>
#include <ctkCheckableComboBox.h>
#include <ctkComboBox.h>
#include <MitkSegmentationUIExports.h>


QT_BEGIN_NAMESPACE

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetTaskParamsUITemplate
{

public:
  QLabel* trainerLabel;
  ctkComboBox* trainerBox;
  QLabel* plannerLabel;
  ctkComboBox* plannerBox;
  QLabel* foldLabel;
  ctkCheckableComboBox* foldBox;
  QLabel* modelLabel;
  ctkComboBox* modelBox;
  QWidget* parent;

  QmitknnUNetTaskParamsUITemplate(QWidget* inputGroupBox_1)
  {
    this->parent = inputGroupBox_1;
    QVBoxLayout* verticalLayout_x = new QVBoxLayout(inputGroupBox_1);
    verticalLayout_x->setObjectName(QString::fromUtf8("verticalLayout_x"));
    QGridLayout* g_x = new QGridLayout();
#ifndef Q_OS_MAC
    g_x->setSpacing(6);
#endif
#ifndef Q_OS_MAC
    g_x->setContentsMargins(0, 0, 0, 0);
#endif
    g_x->setObjectName(QString::fromUtf8("g_2"));

    modelLabel = new QLabel("Configuration", inputGroupBox_1);
    g_x->addWidget(modelLabel, 0, 0, 1, 1);
    trainerLabel = new QLabel("Trainer", inputGroupBox_1);
    g_x->addWidget(trainerLabel, 0, 1, 1, 1);

    modelBox = new ctkComboBox(inputGroupBox_1);
    modelBox->setObjectName(QString::fromUtf8("modelBox_1"));
    g_x->addWidget(modelBox, 1, 0, 1, 1);
    trainerBox = new ctkComboBox(inputGroupBox_1);
    trainerBox->setObjectName(QString::fromUtf8("trainerBox_1"));
    g_x->addWidget(trainerBox, 1, 1, 1, 1);

    plannerLabel = new QLabel("Planner", inputGroupBox_1);
    g_x->addWidget(plannerLabel, 2, 0, 1, 1);
    foldLabel = new QLabel("Fold", inputGroupBox_1);
    g_x->addWidget(foldLabel, 2, 1, 1, 1);

    plannerBox = new ctkComboBox(inputGroupBox_1);
    plannerBox->setObjectName(QString::fromUtf8("plannerBox_1"));
    g_x->addWidget(plannerBox, 3, 0, 1, 1);
    foldBox = new ctkCheckableComboBox(inputGroupBox_1);
    foldBox->setObjectName(QString::fromUtf8("foldBox_1"));
    g_x->addWidget(foldBox, 3, 1, 1, 1);

    verticalLayout_x->addLayout(g_x);
  }

  void setVisible(bool visiblity)
  {
    this->parent->setVisible(visiblity);
  }
};
QT_END_NAMESPACE

#endif
