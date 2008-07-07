/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 15:14:28 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13135 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkLoadPresetDialog.h"

#include <qlistbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

QmitkLoadPresetDialog::QmitkLoadPresetDialog(QWidget* parent, const char* name, std::list<std::string> presets)
:QDialog(parent, name, true) // true, modal
{
  QDialog::setMinimumSize(250, 300);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  // list of all presets
  lblPrompt = new QLabel( "Which preset do you want to load?", this );
  verticalLayout->addWidget( lblPrompt );
  lstPresets = new QListBox( this, "lstPresets" );
  verticalLayout->addWidget( lstPresets );

  std::list<std::string>::iterator iter;
  for( iter = presets.begin(); iter != presets.end(); iter++ ) 
  {
    std::string preset = *iter;
    lstPresets->insertItem( preset.c_str() );
  }

  lstPresets->setSelected(0, true); // select first Item by default (might turn out to be a stupid descision)

  connect( lstPresets, SIGNAL(selected(const QString&)), this, SLOT(onPresetImmediatelySelected(const QString&)) );
  connect( lstPresets, SIGNAL(highlighted(const QString&)), this, SLOT(onPresetSelected(const QString&)) );

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
}

QmitkLoadPresetDialog::~QmitkLoadPresetDialog()
{
}

const char* QmitkLoadPresetDialog::GetPresetName()
{
  return lstPresets->currentText().ascii();
}

void QmitkLoadPresetDialog::onPresetImmediatelySelected(const QString& preset)
{
  if ( (signed)(lstPresets->currentItem()) != (signed)(lstPresets->count()-1) )
  {
    accept(); // close
  }
  else
  {
    // dont close
  }
}

