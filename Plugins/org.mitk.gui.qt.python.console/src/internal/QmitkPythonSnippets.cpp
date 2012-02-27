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

  defaultSnippets["itk image processing"] =
      "import itk\n"
      "import mitkCast\n\n"
      "print 'using ITK ' + itk.Version.GetITKVersion()\n"
      "itkImage = mitkCast.node2itk( Pic3D, itk.Image.US3 )\n"
      "kernel = itk.strel( 3, 3 )\n"
      "filter  = itk.GrayscaleDilateImageFilter[itkImage,itkImage,kernel].New( itkImage, Kernel=kernel )\n"
      "filter.Update()\n"
      "Pic3DFiltered = mitkCast.itk2node( filter.GetOutput(), 'Pic3DFiltered' )\n";

  defaultSnippets["itk batch image processing"] =
      "import itk\n"
      "import mitkCast\n\n"
      "print 'using ITK ' + itk.Version.GetITKVersion()\n"
      "itkImage = mitkcast.node2itk( Pic3D, itk.Image.US3 )\n"
      "for s in range(5):\n"
        "kernel = itk.strel( 3, s+1 )\n"
        "filter  = itk.GrayscaleDilateImageFilter[itkImage,itkImage,kernel].New( itkImage, Kernel=kernel )\n"
        "filter.Update()\n"
        "exec( \"newnode_\" + str(s+1) + \" = mitkcast.itk2node( filter.GetOutput(), 'demo_\" + str(s+1) + \"')\" )\n";

  defaultSnippets["Import itk and vtk"] =
      "import itk, vtk";

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
    QString snippet = QString::fromStdString(it->second);
    snippet = snippet.replace("\n", "<br />");
//    std::cout << "adding " << it->first << ": " << snippet.toStdString() << std::endl;
    node->Put( it->first, snippet.toStdString() );
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
//      std::cout << "inserting " << keys.at(i) << std::endl;
      snippet = node->Get( keys.at(i), "" );
      QString qsnippet = QString::fromStdString(snippet);
      qsnippet = qsnippet.replace("<br />", "\n");
//      std::cout << "inserting " << keys.at(i) << ": " << qsnippet.toStdString() << std::endl;
      m_Snippets[keys.at(i)] = qsnippet.toStdString();
    }
  }

  this->Update();

  connect( m_Controls->Name, SIGNAL(currentIndexChanged(int)), this, SLOT(on_Name_currentIndexChanged(int)) );
  connect( m_Controls->AddNewSnippet, SIGNAL(clicked()), this, SLOT(on_AddNewSnippet_clicked()) );
  connect( m_Controls->RemoveSnippet, SIGNAL(clicked()), this, SLOT(on_RemoveSnippet_clicked()) );
  connect( m_Controls->PasteNow, SIGNAL(clicked()), this, SLOT(on_PasteNow_clicked()) );
  connect( m_Controls->Content, SIGNAL(textChanged()), this, SLOT(on_Content_textChanged()) );
  connect( m_Controls->RenameSnippet, SIGNAL(clicked()),
           this, SLOT(on_RenameSnippet_clicked()) );
  connect( m_Controls->RestoreDefaultSnippets, SIGNAL(clicked()),
           this, SLOT(on_RestoreDefaultSnippets_clicked()) );
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

void QmitkPythonSnippets::on_RestoreDefaultSnippets_clicked()
{
  QString question = QString("Really restore default Snippets?");
  int remove = QMessageBox::question( QApplication::topLevelWidgets().at(0),
                                      QString("Confirm restoring"),
                                      question,
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No );
  if( remove == QMessageBox::Yes || remove == QMessageBox::Ok )
  {
    std::map<std::string, std::string>::const_iterator it
        = DEFAULT_SNIPPETS.begin();
    while( it != DEFAULT_SNIPPETS.end() )
    {
      m_Snippets[it->first] = it->second;
      ++it;
    }
    this->Update();
  }

}

void QmitkPythonSnippets::on_AddNewSnippet_clicked()
{
  QString name = "newSnippet";
  CreateUniqueName(name);
  m_Snippets[name.toStdString()] = "";
  m_NameToSelect = name;
  this->Update();
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
    this->Update();
  }
}

void QmitkPythonSnippets::on_PasteNow_clicked()
{
  if( m_Controls->Name->count() == 0 )
    return;

  QString name = m_Controls->Name->currentText();
  QString snippet = QString::fromStdString(m_Snippets[name.toStdString()]);
  QmitkPythonMediator::getInstance()->paste( snippet );
  QmitkPythonMediator::getInstance()->update();
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
                                           m_NameToSelect, &repeat );

    if( newName != m_Controls->Name->currentText() )
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
  if( newName != m_Controls->Name->currentText() )
  {
    std::map<std::string, std::string>::iterator it
        = m_Snippets.find( m_NameToSelect.toStdString() );
    std::string snippet = it->second;
    m_Snippets.erase(it);
    m_Snippets[newName.toStdString()] = snippet;
    m_NameToSelect = newName;
    this->Update();
  }
}

void QmitkPythonSnippets::Update()
{
  if( m_NameToSelect.isEmpty() )
  {
    QString name = m_Controls->Name->currentText();
    m_NameToSelect = name;
  }

  m_Controls->Name->clear();
  std::map<std::string, std::string>::iterator it
      = m_Snippets.begin();

  while( it != m_Snippets.end() )
  {
    m_Controls->Name->addItem( QString::fromStdString( it->first ) );
    ++it;
  }

  // reselect previous or last one if there are elements or nothing if there are no snippets
  std::string current = m_NameToSelect.toStdString();
  int index = -1;
  if( m_Snippets.find(current) != m_Snippets.end() )
  {
    index = m_Controls->Name->findText( m_NameToSelect );
  }
  else if( m_Snippets.size() > 0 )
  {
    index = m_Controls->Name->count()-1;
  }

  if( index >= 0 )
  {
    m_Controls->Name->setCurrentIndex(index);
    current = m_Controls->Name->itemText( index ).toStdString();
    m_Controls->Content->setPlainText( QString::fromStdString(m_Snippets[current]) );
  }

  // clear the current name
  m_NameToSelect.clear();
}
