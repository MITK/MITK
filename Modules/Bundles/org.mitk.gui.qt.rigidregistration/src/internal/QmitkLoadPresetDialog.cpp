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

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>

QmitkLoadPresetDialog::QmitkLoadPresetDialog(QWidget* parent, Qt::WindowFlags f, const char* name, std::list<std::string> presets)
:QDialog(parent, f)
{
  QDialog::setMinimumSize(250, 300);
  this->setObjectName(name);

  QBoxLayout * verticalLayout = new QVBoxLayout( this );
  verticalLayout->setMargin(5);
  verticalLayout->setSpacing(5);

  // list of all presets
  lblPrompt = new QLabel( "Which preset do you want to load?", this );
  verticalLayout->addWidget( lblPrompt );
  lstPresets = new QListWidget( this );
  verticalLayout->addWidget( lstPresets );

  std::list<std::string>::iterator iter;
  for( iter = presets.begin(); iter != presets.end(); iter++ ) 
  {
    std::string preset = *iter;
    new QListWidgetItem(preset.c_str(), lstPresets);
  }

  lstPresets->setCurrentItem(0); // select first Item by default (might turn out to be a stupid descision)

  connect( lstPresets, SIGNAL(itemDoubleClicked (QListWidgetItem *)), this, SLOT(onPresetImmediatelySelected(QListWidgetItem *)) );

  // buttons for closing the dialog
  btnOk = new QPushButton( tr("Ok"), this);
  btnOk->setObjectName("btnOk" );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), this, SLOT(accept()) );

  QPushButton* btnCancel = new QPushButton( tr("Cancel"), this);
  btnCancel->setObjectName("btnCancel" );
  connect( btnCancel, SIGNAL(clicked()), this, SLOT(reject()) );

  QWidget* buttonWidget = new QWidget(this);
  QBoxLayout * horizontalLayout = new QHBoxLayout( buttonWidget );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( btnOk );
  horizontalLayout->addWidget( btnCancel );
  verticalLayout->addWidget(buttonWidget);
}

QmitkLoadPresetDialog::~QmitkLoadPresetDialog()
{
}

std::string QmitkLoadPresetDialog::GetPresetName()
{
  std::string presetName = lstPresets->currentItem()->text().toLatin1();
  return presetName;
}

void QmitkLoadPresetDialog::onPresetImmediatelySelected(QListWidgetItem * )
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

