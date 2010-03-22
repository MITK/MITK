/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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
#include <QPushButton>
#include <QLineEdit>
#include <QApplication>

QmitkFileChooser::QmitkFileChooser( bool selectDir, QWidget* parent, Qt::WindowFlags f )
: QWidget( parent, f )
, m_SelectDir( selectDir )
{
  m_File = new QLineEdit;
  m_File->setReadOnly( true );
  m_SelectFile = new QPushButton;

  QVBoxLayout* layout = new QVBoxLayout; 
  layout->addWidget( m_File );
  layout->addWidget( m_SelectFile );

  this->setLayout( layout );
}

void QmitkFileChooser::SetSelectDir( bool selectDir )
{
  m_SelectDir = selectDir;
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
    filename = QFileDialog::getOpenFileName( QApplication::activeWindow()
      , "Open file", m_File->text(), m_FilePattern );

  m_File->setText( filename );  
}