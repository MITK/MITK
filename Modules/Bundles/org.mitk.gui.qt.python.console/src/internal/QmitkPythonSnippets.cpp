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

#include "QmitkPythonSnippets.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkPythonConsoleView.h"

#include "mitkDataStorageEditorInput.h"

// berry Includes
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>
#include <berryConstants.h>
#include <berryIViewPart.h>

#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>
#include <mitkImageCaster.h>
#include <QApplication>
#include "QmitkPythonMediator.h"
#include "QmitkPythonScriptEditorHighlighter.h"

const std::string QmitkPythonSnippets::VIEW_ID = "org.mitk.views.pythonsnippets";

std::map<std::string, std::string> QmitkPythonSnippets::CreateDefaultSnippets()
{
  std::map<std::string, std::string> defaultSnippets;

  defaultSnippets["Cast mitk to itk image"] =
      "itk.Image itkImage;"
      "mitk.ImageCaster.Cast(mitkImage, itkImage);";

  return defaultSnippets;
}

const std::map<std::string, std::string> QmitkPythonSnippets::DEFAULT_SNIPPETS
  = QmitkPythonSnippets::CreateDefaultSnippets();

QmitkPythonSnippets::QmitkPythonSnippets()
: m_MultiWidget(NULL)
, m_Controls(0)
{
}

QmitkPythonSnippets::~QmitkPythonSnippets()
{
  berry::IPreferencesService::Pointer _PreferencesService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(
        berry::IPreferencesService::ID);
  berry::IPreferences::Pointer node = _PreferencesService->GetSystemPreferences()->Node(VIEW_ID);
  std::map<std::string, std::string>::iterator it
      = m_Snippets.begin();

  node->Clear();
  while( it != m_Snippets.end() )
  {
//    std::cout << "adding " << it->first << std::endl;
    node->Put( it->first, it->second );
    ++it;
  }
  node->Flush();
}

void QmitkPythonSnippets::CreateQtPartControl(QWidget *parent)
{
  if( !m_Controls )
  {
    m_Controls = new Ui::QmitkPythonSnippets;
    m_Controls->setupUi( parent );
    QmitkPythonScriptEditorHighlighter *highlighter =
        new QmitkPythonScriptEditorHighlighter(m_Controls->Content->document());
  }

  // get all snippets
  berry::IPreferencesService::Pointer _PreferencesService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(
        berry::IPreferencesService::ID);

  // insert default snippets if there were no prefs before
  if( !_PreferencesService->GetSystemPreferences()->NodeExists(VIEW_ID) )
  {
    m_Snippets = DEFAULT_SNIPPETS;
  }
  else
  {
    berry::IPreferences::Pointer node = _PreferencesService->GetSystemPreferences()->Node(VIEW_ID);
    std::vector<std::string> keys = node->Keys();
    std::string snippet;
    for( size_t i = 0; i < keys.size(); ++i)
    {
      snippet = node->Get( keys.at(i), "" );
      m_Snippets[keys.at(i)] = snippet;
    }
  }

  connect( m_Controls->Name, SIGNAL(currentIndexChanged(int)), this, SLOT(on_Name_currentIndexChanged(int)) );
  std::map<std::string, std::string>::iterator it
      = m_Snippets.begin();

  while( it != m_Snippets.end() )
  {
    m_Controls->Name->addItem( QString::fromStdString( it->first ) );
    ++it;
  }
  m_Controls->Name->setCurrentIndex( m_Controls->Name->count()-1 );
  connect( m_Controls->AddNewSnippet, SIGNAL(clicked()), this, SLOT(on_AddNewSnippet_clicked()) );
  connect( m_Controls->RemoveSnippet, SIGNAL(clicked()), this, SLOT(on_RemoveSnippet_clicked()) );
  connect( m_Controls->PasteNow, SIGNAL(clicked()), this, SLOT(on_PasteNow_clicked()) );
  connect( m_Controls->Content, SIGNAL(textChanged()), this, SLOT(on_Content_textChanged()) );
  connect( m_Controls->RenameSnippet, SIGNAL(clicked()),
           this, SLOT(on_RenameSnippet_clicked()) );
}

void QmitkPythonSnippets::SetFocus ()
{

}

int QmitkPythonSnippets::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkPythonSnippets::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
  {
    return 160;
  }
  else
  {
    return preferredResult;
  }
}

void QmitkPythonSnippets::on_Name_currentIndexChanged(int i)
{
//  std::cout << "on_Name_currentIndexChanged" << std::endl;
  if( m_Controls->Name->count() == 0 )
    return;
  QString name = m_Controls->Name->currentText();
  m_Controls->Content->setText( QString::fromStdString(m_Snippets[name.toStdString()]) );
  m_CurrentName = name;
}

void QmitkPythonSnippets::CreateUniqueName( QString& name )
{
  QString basename = name;
  size_t i = 2;
  while( m_Snippets.find(name.toStdString()) != m_Snippets.end() )
  {
    name = basename + QString("_%1").arg(i);
    ++i;
  }
}

void QmitkPythonSnippets::on_AddNewSnippet_clicked()
{
  QString name = "newSnippet";
  CreateUniqueName(name);
  m_Snippets[name.toStdString()] = "";
  m_Controls->Name->insertItem( m_Controls->Name->count(), name );
  m_Controls->Name->setCurrentIndex( m_Controls->Name->count()-1 );
  this->on_RenameSnippet_clicked();
}

void QmitkPythonSnippets::on_RemoveSnippet_clicked()
{
  if( m_Controls->Name->count() == 0 )
    return;
  QString name = m_Controls->Name->currentText();
  QString question = QString("Really remove Snippet %1?").arg(name);
  int remove = QMessageBox::question( QApplication::topLevelWidgets().at(0),
                                      QString("Confirm removal"),
                                      question,
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No );
  if( remove == QMessageBox::Yes || remove == QMessageBox::Ok )
  {
    std::map<std::string, std::string>::iterator it
        = m_Snippets.find( name.toStdString() );
    if( it!= m_Snippets.end() )
    {
//      std::cout << "removing " << it->first << std::endl;
      m_Snippets.erase(it);
    }
    m_Controls->Content->setText("");
    m_Controls->Name->removeItem( m_Controls->Name->currentIndex() );
  }
}

void QmitkPythonSnippets::on_PasteNow_clicked()
{
  if( m_Controls->Name->count() == 0 )
    return;

  QString name = m_Controls->Name->currentText();
  QString snippet = QString::fromStdString(m_Snippets[name.toStdString()]);
  QmitkPythonMediator::getInstance()->paste( snippet );
}

void QmitkPythonSnippets::on_Content_textChanged()
{
  if( m_Controls->Name->count() == 0 )
    return;

  std::string name = (m_Controls->Name->currentText()).toStdString();
  std::string snippet = m_Controls->Content->toPlainText().toStdString();

  if( m_Snippets.find(name) != m_Snippets.end() )
    m_Snippets[name] = snippet;
}

void QmitkPythonSnippets::on_RenameSnippet_clicked()
{
//  std::cout << "on_Name_editTextChanged" << std::endl;
  if( m_Controls->Name->count() == 0 )
    return;

  QString newName;
  bool repeat = false;
  do
  {
    newName = QInputDialog::getText( QApplication::topLevelWidgets().at(0),
                                           "Name the Snippet", "Name:", QLineEdit::Normal,
                                           m_CurrentName, &repeat );
    if( newName != m_CurrentName )
    {
      repeat = m_Snippets.find(newName.toStdString()) != m_Snippets.end()
          || newName.isEmpty();
      if( repeat )
        QMessageBox::information( QApplication::topLevelWidgets().at(0)
                                            , QString("Invalid Name"),
                                  "Invalid name. Please enter another one");
    }
    else
      repeat = false;
  }
  while( repeat );


  // name changed
  if( newName != m_CurrentName )
  {
    std::map<std::string, std::string>::iterator it
        = m_Snippets.find( m_CurrentName.toStdString() );
    std::string snippet = it->second;
    m_Snippets.erase(it);
    m_Snippets[newName.toStdString()] = snippet;
    m_CurrentName = newName;
    m_Controls->Name->setItemText( m_Controls->Name->currentIndex(), newName );
  }
}
