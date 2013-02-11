/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkPythonSnippets.h"
#include "QmitkPythonScriptEditorHighlighter.h"
#include <QtGui>

QmitkPythonSnippets::QStringMap QmitkPythonSnippets::CreateDefaultSnippets()
{
  QStringMap defaultSnippets;

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

struct QmitkPythonSnippetsData
{
  QString m_FilePath;

  QAction* m_PasteSnippet;
  QAction* m_RemoveSnippet;
  QAction* m_RenameSnippet;
  QAction* m_AddSnippet;
  QAction* m_RestoreDefaultSnippets;
  QToolBar* m_Toolbar;

  QComboBox* m_Name;

  QTextEdit* m_Content;

  QGridLayout* m_Layout;
};

const QmitkPythonSnippets::QStringMap QmitkPythonSnippets::DEFAULT_SNIPPETS
  = QmitkPythonSnippets::CreateDefaultSnippets();

QmitkPythonSnippets::QmitkPythonSnippets(QWidget* parent)
: QWidget(parent), d(new QmitkPythonSnippetsData)
{

  d->m_PasteSnippet = new QAction(this);
  d->m_PasteSnippet->setObjectName(QString::fromUtf8("PasteSnippet"));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/mitkPython/edit-paste.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_PasteSnippet->setIcon(icon);

  d->m_RemoveSnippet = new QAction(this);
  d->m_RemoveSnippet->setObjectName(QString::fromUtf8("RemoveSnippet"));
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/mitkPython/edit-delete.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RemoveSnippet->setIcon(icon1);

  d->m_RenameSnippet = new QAction(this);
  d->m_RenameSnippet->setObjectName(QString::fromUtf8("RenameSnippet"));
  QIcon icon2;
  icon2.addFile(QString::fromUtf8(":/mitkPython/edit-find-replace.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RenameSnippet->setIcon(icon2);

  d->m_AddSnippet = new QAction(this);
  d->m_AddSnippet->setObjectName(QString::fromUtf8("AddSnippet"));
  QIcon icon3;
  icon3.addFile(QString::fromUtf8(":/mitkPython/document-new.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_AddSnippet->setIcon(icon3);

  d->m_RestoreDefaultSnippets = new QAction(this);
  d->m_RestoreDefaultSnippets->setObjectName(QString::fromUtf8("RestoreDefaultSnippets"));
  QIcon icon4;
  icon4.addFile(QString::fromUtf8(":/mitkPython/edit-clear.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RestoreDefaultSnippets->setIcon(icon4);

  d->m_Toolbar = new QToolBar;
  d->m_Toolbar->addAction( d->m_PasteSnippet );
  d->m_Toolbar->addAction( d->m_AddSnippet );
  d->m_Toolbar->addAction( d->m_RemoveSnippet );
  d->m_Toolbar->addAction( d->m_RenameSnippet );
  d->m_Toolbar->addAction( d->m_RestoreDefaultSnippets );

  d->m_Name = new QComboBox;
  d->m_Name->setObjectName(QString::fromUtf8("Name"));
  d->m_Name->setEditable(true);

  d->m_Content = new QTextEdit(this);
  d->m_Content->setObjectName(QString::fromUtf8("Content"));

  d->m_Layout = new QGridLayout;
  d->m_Layout->addWidget( d->m_Toolbar, 0, 0, 1, 1 );
  d->m_Layout->addWidget( d->m_Name, 1, 0, 1, 1 );
  d->m_Layout->addWidget( d->m_Content, 2, 0, 1, 1 );
  d->m_Layout->setContentsMargins(2,2,2,2);

  this->setLayout(d->m_Layout);
  QMetaObject::connectSlotsByName(this);
}

QmitkPythonSnippets::~QmitkPythonSnippets()
{
  delete d;
}

void QmitkPythonSnippets::SetFilePath(const QString &filePath)
{
  d->m_FilePath = filePath;
}

void QmitkPythonSnippets::on_PasteSnippet_triggered( bool )
{
  emit PasteCommandRequested( d->m_Content->toPlainText() );
}

/*
void QmitkPythonSnippets::on_Name_currentIndexChanged(int i)
{
//  std::cout << "on_Name_currentIndexChanged" << std::endl;
  if( m_Controls->Name->count() == 0 )
    return;
  QString name = m_Controls->Name->currentText();
  m_Controls->Content->setText( QString::fromStdString(m_Snippets[name.toStdString()]) );
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
    QStringMap::const_iterator it
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
    QStringMap::iterator it
        = m_Snippets.find( name.toStdString() );
    if( it!= m_Snippets.end() )
    {
//      std::cout << "removing " << it->first << std::endl;
      m_Snippets.erase(it);
    }
    this->Update();
  }
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
    QStringMap::iterator it
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
  QStringMap::iterator it
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
*/
