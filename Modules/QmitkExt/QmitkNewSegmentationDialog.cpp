/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-04-14 19:45:53 +0200 (Mo, 14 Apr 2008) $
Version:   $Revision: 14081 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkNewSegmentationDialog.h"

#include "mitkOrganTypeProperty.h"

#include <itkRGBPixel.h>

#include <Q3ListBox>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QColorDialog>

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(QWidget* parent)
:QDialog(parent), // true, modal
 selectedOrgan("undefined"),
 newOrganEntry(false)
{
  QDialog::setFixedSize(250, 105);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  mitk::OrganTypeProperty::Pointer organTypes = mitk::OrganTypeProperty::New();

  // to enter a name for the segmentation
  verticalLayout->addWidget( new QLabel( "Name and color of the segmentation", this ) );

  // to choose a color
  btnColor = new QPushButton( tr(""), this, "btnColor" );
  btnColor->setFixedWidth(25);
  btnColor->setAutoFillBackground(true);
  btnColor->setStyleSheet("background-color:rgb(255,0,0)");

  connect( btnColor, SIGNAL(clicked()), this, SLOT(onColorBtnClicked()) );

  edtName = new QLineEdit( "", this, "edtName" );

  QBoxLayout * horizontalLayout2 = new QHBoxLayout(verticalLayout);
  horizontalLayout2->addWidget( btnColor );
  horizontalLayout2->addWidget( edtName );



  // buttons for closing the dialog
  btnOk = new QPushButton( tr("Ok"), this, "btnOk" );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), this, SLOT(accept()) );

  QPushButton* btnCancel = new QPushButton( tr("Cancel"), this, "btnCancel" );
  connect( btnCancel, SIGNAL(clicked()), this, SLOT(reject()) );

  QBoxLayout * horizontalLayout = new QHBoxLayout( verticalLayout );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( btnOk );
  horizontalLayout->addWidget( btnCancel );

  edtName->setFocus();
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

const char* QmitkNewSegmentationDialog::GetSegmentationName()
{
  return edtName->text().ascii();
}

void QmitkNewSegmentationDialog::onOrganSelected(const QString& organ)
{
  selectedOrgan = organ;
  edtName->setText( organ );
}

void QmitkNewSegmentationDialog::onOrganImmediatelySelected(const QString& organ)
{
  selectedOrgan = organ;
  edtName->setText( organ );

  if ( (signed)(lstOrgans->currentItem()) != (signed)(lstOrgans->count()-1) )
  {
    accept(); // close
  }
  else
  {
    // dont close
  }
}


void QmitkNewSegmentationDialog::onOrganSelected(int index)
{
  if ( index == (signed)(lstOrgans->count()-1) )
  {
    // "Other organ..." selected, we shouldn't close the dialog now
    newOrganEntry = true;

    edtNewOrgan->show();
    btnOk->setEnabled( false );
  }
  else
  {
    newOrganEntry = false;
    
    edtNewOrgan->hide();
    btnOk->setEnabled( true );
  }
}

const char* QmitkNewSegmentationDialog::GetOrganType()
{
  return selectedOrgan.ascii();
}


void QmitkNewSegmentationDialog::onNewOrganNameChanged(const QString& newText)
{
  if (!newText.isEmpty())
  {
    btnOk->setEnabled( true );
  }

  selectedOrgan = newText;
  edtName->setText( newText );
}

void QmitkNewSegmentationDialog::onColorBtnClicked()
{
  color = QColorDialog::getColor();
  if (color.spec() == 0)
  {
    color.setRed(255);
    color.setGreen(0);
    color.setBlue(0);
  }
  btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
}
    
void QmitkNewSegmentationDialog::setPrompt( const QString& prompt )
{
  lblPrompt->setText( prompt );
}
    
void QmitkNewSegmentationDialog::setSegmentationName( const QString& name )
{
  edtName->setText( name );
}

mitk::Color QmitkNewSegmentationDialog::GetColorProperty()
{
  mitk::Color colorProperty;
  if (color.spec() == 0)
  {
    colorProperty.SetRed(1);
    colorProperty.SetGreen(0);
    colorProperty.SetBlue(0);
  }
  else
  {
    colorProperty.SetRed(color.redF());
    colorProperty.SetGreen(color.greenF());
    colorProperty.SetBlue(color.blueF());
  }
  return colorProperty;
}
