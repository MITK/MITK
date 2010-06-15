/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkEditPointDialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

struct QmitkEditPointDialogData
{
  mitk::PointSet* m_PointSet;
  mitk::PointSet::PointIdentifier m_PointId;
  QLineEdit* m_XCoord;
  QLineEdit* m_YCoord;
  QLineEdit* m_ZCoord;
  int m_Timestep;
};

QmitkEditPointDialog::QmitkEditPointDialog( QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f)
, d(new QmitkEditPointDialogData)
{  
  this->setWindowTitle("Edit Point Dialog");
  d->m_PointSet = 0;
  d->m_Timestep = 0;
  d->m_XCoord = new QLineEdit;
  d->m_YCoord = new QLineEdit;
  d->m_ZCoord = new QLineEdit;
  QPushButton* _OKButton = new QPushButton("OK");
  connect( _OKButton, SIGNAL(clicked(bool)), this, SLOT(OnOkButtonClicked(bool)) );

  QGridLayout* layout = new QGridLayout;  
  layout->addWidget(new QLabel("X: "), 0,0,1,1);
  layout->addWidget(d->m_XCoord, 0,1,1,1);  
  layout->addWidget(new QLabel("Y: "), 1,0,1,1);
  layout->addWidget(d->m_YCoord, 1,1,1,1);  
  layout->addWidget(new QLabel("Z: "), 2,0,1,1);
  layout->addWidget(d->m_ZCoord, 2,1,1,1);
  layout->addWidget(_OKButton, 3,0,2,1);
  this->setLayout(layout);

}

QmitkEditPointDialog::~QmitkEditPointDialog()
{
  delete d;
}

void QmitkEditPointDialog::SetPoint( mitk::PointSet* _PointSet, mitk::PointSet::PointIdentifier _PointId
                                    , int timestep)
{
  d->m_PointSet = _PointSet;
  d->m_PointId = _PointId;
  d->m_Timestep = timestep;
  mitk::PointSet::PointType p = d->m_PointSet->GetPoint(d->m_PointId, d->m_Timestep);
  d->m_XCoord->setText( QString::number( p.GetElement(0), 'f', 3 ) );
  d->m_YCoord->setText( QString::number( p.GetElement(1), 'f', 3 ) );
  d->m_ZCoord->setText( QString::number( p.GetElement(2), 'f', 3 ) );
}


void QmitkEditPointDialog::OnOkButtonClicked( bool triggered )
{  
  if(d->m_PointSet == 0)
  {
    MITK_WARN << "Pointset is 0.";
    this->reject();
  }

  mitk::PointSet::PointType p = d->m_PointSet->GetPoint(d->m_PointId, d->m_Timestep);
  p.SetElement( 0, d->m_XCoord->text().toDouble() );
  p.SetElement( 1, d->m_YCoord->text().toDouble() );
  p.SetElement( 2, d->m_ZCoord->text().toDouble() );
  d->m_PointSet->SetPoint(d->m_PointId, p);

  this->accept();
}