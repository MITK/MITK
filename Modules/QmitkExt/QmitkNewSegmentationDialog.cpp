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

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <QColorDialog>
#include <QStringListModel>
#include <QAbstractItemModel>

#include <mitkDataTreeNodeFactory.h>

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
  lblPrompt = new QLabel( "Name and color of the segmentation", this );
  verticalLayout->addWidget( lblPrompt );

  // to choose a color
  btnColor = new QPushButton( "", this );
  btnColor->setFixedWidth(25);
  btnColor->setAutoFillBackground(true);
  btnColor->setStyleSheet("background-color:rgb(255,0,0)");

  connect( btnColor, SIGNAL(clicked()), this, SLOT(onColorBtnClicked()) );

  edtName = new QLineEdit( "", this );
  QStringList completionList;
  completionList << "";
  completer = new QCompleter(completionList);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  edtName->setCompleter(completer);

  connect( completer, SIGNAL(activated(const QString&)), this, SLOT(onColorChange(const QString&)) );

  QBoxLayout * horizontalLayout2 = new QHBoxLayout(this);
  verticalLayout->addLayout(horizontalLayout2);
  horizontalLayout2->addWidget( btnColor );
  horizontalLayout2->addWidget( edtName );



  // buttons for closing the dialog
  btnOk = new QPushButton( tr("Ok"), this );
  btnOk->setDefault(true);
  connect( btnOk, SIGNAL(clicked()), this, SLOT(onAcceptClicked()) );

  QPushButton* btnCancel = new QPushButton( tr("Cancel"), this );
  connect( btnCancel, SIGNAL(clicked()), this, SLOT(reject()) );

  QBoxLayout * horizontalLayout = new QHBoxLayout(this);
  verticalLayout->addLayout( horizontalLayout );
  horizontalLayout->setSpacing(5);
  horizontalLayout->addStretch();
  horizontalLayout->addWidget( btnOk );
  horizontalLayout->addWidget( btnCancel );

  edtName->setFocus();
}

QmitkNewSegmentationDialog::~QmitkNewSegmentationDialog()
{
}

void QmitkNewSegmentationDialog::onAcceptClicked()
{
  m_SegmentationName = edtName->text();
  this->accept();
}

const std::string QmitkNewSegmentationDialog::GetSegmentationName()
{
  return m_SegmentationName.toStdString();
}

const char* QmitkNewSegmentationDialog::GetOrganType()
{
  return selectedOrgan.toLocal8Bit().constData();
}


void QmitkNewSegmentationDialog::onNewOrganNameChanged(const QString& newText)
{
  if (!newText.isEmpty())
  {
    btnOk->setEnabled( true );
  }

  selectedOrgan = newText;
  this->setSegmentationName( newText );

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
  m_SegmentationName = name;
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

void QmitkNewSegmentationDialog::AddSuggestion(const QStringList organColor)
{
  SetSuggestionList(organColor);
}

void QmitkNewSegmentationDialog::SetSuggestionList(QStringList organColorList)
{
  for (int i = 0; i < organColorList.size()/4; i++)
  {
    organList.push_back(organColorList.at(4*i));
    colorList.push_back(organColorList.at(4*i+1));
    colorList.push_back(organColorList.at(4*i+2));
    colorList.push_back(organColorList.at(4*i+3));
  }
  QStringListModel* completeModel = static_cast<QStringListModel*> (completer->model());
  completeModel->setStringList(organList);
}

void QmitkNewSegmentationDialog::onColorChange(const QString& completedWord)
{
  if (organList.contains(completedWord))
  {
    int j = organList.indexOf(completedWord);
    color.setRed(colorList.at(3*j).toInt());
    color.setGreen(colorList.at(3*j+1).toInt());
    color.setBlue(colorList.at(3*j+2).toInt());
    btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
  }
}
