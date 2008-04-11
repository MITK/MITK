/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkNewSegmentationDialog.h"

#include "mitkOrganTypeProperty.h"

#include <qlistbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

QmitkNewSegmentationDialog::QmitkNewSegmentationDialog(QWidget* parent, const char* name)
:QDialog(parent, name, true), // true, modal
 selectedOrgan("undefined"),
 newOrganEntry(false)
{
  QDialog::setMinimumSize(250, 300);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  // list of all known organs
  lblPrompt = new QLabel( "What do you want to segment?", this );
  verticalLayout->addWidget( lblPrompt );
  lstOrgans = new QListBox( this, "lstOrgans" );
  verticalLayout->addWidget( lstOrgans );

  mitk::OrganTypeProperty::Pointer organTypes = new mitk::OrganTypeProperty();
  for ( mitk::EnumerationProperty::EnumConstIterator iter = organTypes->Begin();
        iter != organTypes->End();
        ++iter )
  {
    std::string organ = iter->second;

    if (organ != "undefined")
    {
      lstOrgans->insertItem( organ.c_str() );
    }
  }

  lstOrgans->setSelected(0, true); // select first Item by default (might turn out to be a stupid descision)

  // one special item for new organs
  lstOrgans->insertItem( "Other organ..." );

  connect( lstOrgans, SIGNAL(selected(const QString&)), this, SLOT(onOrganImmediatelySelected(const QString&)) );
  connect( lstOrgans, SIGNAL(highlighted(const QString&)), this, SLOT(onOrganSelected(const QString&)) );
  connect( lstOrgans, SIGNAL(highlighted(int)), this, SLOT(onOrganSelected(int)) );

  // to enter the definition of a new organ 
  edtNewOrgan = new QLineEdit( "Enter organ name here", this, "edtNewOrgan" );
  edtNewOrgan->setPaletteForegroundColor( Qt::red );
  verticalLayout->addWidget( edtNewOrgan );
  edtNewOrgan->hide();
  connect( edtNewOrgan, SIGNAL(textChanged(const QString&)), this, SLOT(onNewOrganNameChanged(const QString&)) );


  // to enter a name for the segmentation
  verticalLayout->addWidget( new QLabel( "Visible name of the segmentation", this ) );
  QString firstOrgan( lstOrgans->currentText() );
  edtName = new QLineEdit( firstOrgan, this, "edtName" );
  verticalLayout->addWidget( edtName );

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

  edtNewOrgan->setFocus();
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
    
void QmitkNewSegmentationDialog::setPrompt( const QString& prompt )
{
  lblPrompt->setText( prompt );
}
    
void QmitkNewSegmentationDialog::setSegmentationName( const QString& name )
{
  edtName->setText( name );
}


