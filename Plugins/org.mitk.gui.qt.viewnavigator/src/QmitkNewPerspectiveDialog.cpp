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

#include "QmitkNewPerspectiveDialog.h"

#include "mitkOrganTypeProperty.h"

#include <itkRGBPixel.h>

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QColorDialog>
#include <QStringListModel>
#include <QAbstractItemModel>

#include <mitkDataNodeFactory.h>

QmitkNewPerspectiveDialog::QmitkNewPerspectiveDialog(QWidget* parent)
    :QDialog(parent)
{
    QGridLayout* formGridLayout = new QGridLayout( this );

    QLabel* label = new QLabel( "Perspective name:", this );

    m_PerspectiveNameLineEdit = new QLineEdit( "", this );
    m_PerspectiveNameLineEdit->setFocus();

    m_AcceptNameButton = new QPushButton( tr("Ok"), this );
    m_AcceptNameButton->setDefault(true);
    m_AcceptNameButton->setEnabled(false);

    QPushButton* rejectNameButton = new QPushButton( tr("Cancel"), this );

    formGridLayout->addWidget(label, 0, 0);
    formGridLayout->addWidget(m_PerspectiveNameLineEdit, 0, 1);
    formGridLayout->addWidget(m_AcceptNameButton, 1, 0);
    formGridLayout->addWidget(rejectNameButton, 1, 1);
    setLayout(formGridLayout);

    // create connections
    connect( rejectNameButton, SIGNAL(clicked()), this, SLOT(reject()) );
    connect( m_AcceptNameButton, SIGNAL(clicked()), this, SLOT(OnAcceptClicked()) );
    connect( m_PerspectiveNameLineEdit, SIGNAL(textEdited(const QString&)), this, SLOT(OnPerspectiveNameChanged(const QString&)) );
}

QmitkNewPerspectiveDialog::~QmitkNewPerspectiveDialog()
{
}

void QmitkNewPerspectiveDialog::SetPerspectiveName(QString name)
{
    m_PerspectiveNameLineEdit->setText(name);
    OnPerspectiveNameChanged(name);
}

void QmitkNewPerspectiveDialog::OnAcceptClicked()
{
    m_PerspectiveName = m_PerspectiveNameLineEdit->text();
    this->accept();
}

const QString QmitkNewPerspectiveDialog::GetPerspectiveName()
{
    return m_PerspectiveName;
}

void QmitkNewPerspectiveDialog::OnPerspectiveNameChanged(const QString& newText)
{
    if (!newText.isEmpty())
        m_AcceptNameButton->setEnabled(true);
    else
        m_AcceptNameButton->setEnabled(false);
}
