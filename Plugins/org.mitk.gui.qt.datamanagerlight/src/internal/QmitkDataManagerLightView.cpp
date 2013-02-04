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

#include "QmitkDataManagerLightView.h"
#include <QtGui>

const std::string QmitkDataManagerLightView::VIEW_ID = "org.mitk.views.datamanagerlight";

struct QmitkDataManagerLightViewData
{

};

QmitkDataManagerLightView::QmitkDataManagerLightView()
    : d( new QmitkDataManagerLightViewData )
{
}

QmitkDataManagerLightView::~QmitkDataManagerLightView()
{
    delete d;
}

void QmitkDataManagerLightView::CreateQtPartControl(QWidget* parent)
{

    QPushButton* loadButton = new QPushButton("Load image");

    QListWidget* listWidget = new QListWidget;

    QLabel* imageInfoLabel = new QLabel("Unknown name");

    QGridLayout* layout = new QGridLayout;
    layout->addWidget( loadButton, 0,0 );
    layout->addWidget( listWidget, 1,0 );
    layout->addWidget( imageInfoLabel, 2,0 );

    parent->setLayout(layout);
}

void QmitkDataManagerLightView::SetFocus()
{
}
