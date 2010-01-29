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

const QString QmitkNewSegmentationDialog::GetSegmentationName()
{
  return m_SegmentationName;
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
  m_Color = QColorDialog::getColor();
  if (m_Color.spec() == 0)
  {
    m_Color.setRed(255);
    m_Color.setGreen(0);
    m_Color.setBlue(0);
  }
  btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
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

mitk::Color QmitkNewSegmentationDialog::GetColor()
{
  mitk::Color colorProperty;
  if (m_Color.spec() == 0)
  {
    colorProperty.SetRed(1);
    colorProperty.SetGreen(0);
    colorProperty.SetBlue(0);
  }
  else
  {
    colorProperty.SetRed(m_Color.redF());
    colorProperty.SetGreen(m_Color.greenF());
    colorProperty.SetBlue(m_Color.blueF());
  }
  return colorProperty;
}

void QmitkNewSegmentationDialog::SetSuggestionList(QStringList organColorList)
{
  QStringList::iterator iter;
  for (iter = organColorList.begin(); iter != organColorList.end(); ++iter)
  {
    QString& element = *iter;
    QString colorName = element.right(7);
    QColor color(colorName);
    QString organName = element.left(element.size() - 7);

    organList.push_back(organName);
    colorList.push_back(color);
  }

  QStringListModel* completeModel = static_cast<QStringListModel*> (completer->model());
  completeModel->setStringList(organList);
}

void QmitkNewSegmentationDialog::onColorChange(const QString& completedWord)
{
  if (organList.contains(completedWord))
  {
    int j = organList.indexOf(completedWord);
    m_Color = colorList.at(j);
    btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)").arg(m_Color.red()).arg(m_Color.green()).arg(m_Color.blue()));
  }
}
