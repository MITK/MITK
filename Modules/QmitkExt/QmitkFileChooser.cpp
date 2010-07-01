/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkFileChooser.h"

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QApplication>

QmitkFileChooser::QmitkFileChooser(bool horizontalLayout, bool selectDir,
                                   bool fileMustExist
                                   , QWidget* parent, Qt::WindowFlags f )
: QWidget( parent, f )
, m_SelectDir( selectDir )
, m_FileMustExist(fileMustExist)
{
  m_File = new QLineEdit;
  m_File->setReadOnly( true );
  m_SelectFile = new QPushButton("Select File");
  connect( m_SelectFile, SIGNAL(clicked(bool)), this, SLOT( OnSelectFileClicked( bool ) ) );

  this->SetHorizotalLayout(horizontalLayout);
}

void QmitkFileChooser::SetHorizotalLayout(bool horizontalLayout)
{
  QBoxLayout* layout = 0;
  if(horizontalLayout)
    layout = new QHBoxLayout;
   else
    layout = new QVBoxLayout;

  layout->setContentsMargins(0,0,0,0);
  layout->addWidget( m_File );
  layout->addWidget( m_SelectFile );

  this->setLayout( layout );
}

void QmitkFileChooser::SetSelectDir( bool selectDir  )
{
  m_SelectDir = selectDir;
}

void QmitkFileChooser::SetFileMustExist( bool fileMustExist )
{
  m_FileMustExist = fileMustExist;
}

void QmitkFileChooser::SetFile( const std::string& file )
{
  m_File->setText( QString::fromStdString(file) );
}

void QmitkFileChooser::SetFilePattern( const std::string& filepattern )
{
  m_FilePattern = QString::fromStdString(filepattern);
}

bool QmitkFileChooser::IsValidFile() const
{
  return m_ValidFile;
}

std::string QmitkFileChooser::GetFile() const
{
  return m_File->text().toStdString();
}

void QmitkFileChooser::OnSelectFileClicked( bool checked/*=false */ )
{

  QString filename;
  if( m_SelectDir )
    filename = QFileDialog::getExistingDirectory( QApplication::activeWindow()
      , "Open directory", m_File->text() );
  else
  {
    if (m_FileMustExist)
      filename = QFileDialog::getOpenFileName( QApplication::activeWindow()
        , "Open file", m_File->text(), m_FilePattern );
    else
      filename = QFileDialog::getSaveFileName( QApplication::activeWindow()
        , "Open file", m_File->text(), m_FilePattern );
  }

  if(!filename.isEmpty())
    m_File->setText( filename );
}
