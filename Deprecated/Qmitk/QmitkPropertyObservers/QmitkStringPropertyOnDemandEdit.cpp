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
#include <QmitkStringPropertyOnDemandEdit.h>
#include <qinputdialog.h>

QmitkStringPropertyOnDemandEdit::QmitkStringPropertyOnDemandEdit( mitk::StringProperty* property, QWidget* parent, const char* name )
: QFrame( parent, name ),
  PropertyEditor( property ),
  m_StringProperty(property)
{
  setFrameStyle( QFrame::NoFrame );
  setLineWidth(0);
  
  // create HBoxLayout with two buttons
  m_layout = new QHBoxLayout(this);
  m_layout->setMargin(0);
 
  m_label = new QLabel(this);
  m_layout->addWidget(m_label);
  
  m_toolbutton = new QClickableLabel2(this);
  m_toolbutton->setText("...");
  m_layout->addWidget(m_toolbutton);
  
  m_layout->addStretch(10);

  connect( m_toolbutton, SIGNAL( clicked() ) , this, SLOT(onToolButtonClicked()) );

  polish();
  adjustSize();

  PropertyChanged();
}

QmitkStringPropertyOnDemandEdit::~QmitkStringPropertyOnDemandEdit()
{
}

void QmitkStringPropertyOnDemandEdit::PropertyChanged()
{
  if ( m_Property )
    m_label->setText( m_StringProperty->GetValue() );
}

void QmitkStringPropertyOnDemandEdit::PropertyRemoved()
{
  m_Property = NULL;
  m_StringProperty = NULL;
  m_label->setText("n/a");
}

void QmitkStringPropertyOnDemandEdit::onToolButtonClicked()
{
  bool ok(false);
  QString newText = QInputDialog::getText("Change text", 
                                          "You can change the displayed text here", 
                                          QLineEdit::Normal, 
                                          m_label->text(),
                                          &ok);

  if (ok)
  {
    BeginModifyProperty();  // deregister from events
  
    m_StringProperty->SetValue(newText.ascii());
    m_label->setText(newText);
  
    EndModifyProperty();  // again register for events
  }
}

void QmitkStringPropertyOnDemandEdit::setPalette( const QPalette& p )
{
  m_label->setPalette(p);
  m_toolbutton->setPalette(p);
  QFrame::setPalette(p);
}

void QmitkStringPropertyOnDemandEdit::setBackgroundMode (BackgroundMode m)
{
  m_label->setBackgroundMode(m);
  m_toolbutton->setBackgroundMode(m);
  QFrame::setBackgroundMode(m);
}
