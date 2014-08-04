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
#include <mitkCommon.h>
#include <memory>

struct QmitkPythonSnippetsData
{
  QString m_AutoSaveFileName;
  QString m_SaveFileName;

  QAction* m_PasteSnippet;
  QAction* m_RemoveSnippet;
  QAction* m_RenameSnippet;
  QAction* m_AddSnippet;
  QAction* m_RestoreDefaultSnippets;
  QAction* m_LoadSnippets;
  QAction* m_SaveSnippets;

  QToolBar* m_Toolbar;

  QComboBox* m_Name;

  QTextEdit* m_Content;

  QGridLayout* m_Layout;

  QmitkPythonSnippets::QStringMap m_Snippets;
};

const QString QmitkPythonSnippets::DEFAULT_SNIPPET_FILE( ":/mitkPython/PythonSnippets.xml" );
const QString QmitkPythonSnippets::SNIPPETS_ROOT_XML_ELEMENT_NAME( "PythonSnippets" );
const QString QmitkPythonSnippets::SNIPPETS_XML_ELEMENT_NAME( "PythonSnippet" );

QmitkPythonSnippets::QmitkPythonSnippets( const QString& _AutoSaveFileName, QWidget* parent )
: QWidget(parent), d(new QmitkPythonSnippetsData)
{
  d->m_SaveFileName = QDir::currentPath();
  d->m_AutoSaveFileName = _AutoSaveFileName;

  if( !QmitkPythonSnippets::LoadStringMap( d->m_AutoSaveFileName, d->m_Snippets ) )
  {
    QmitkPythonSnippets::LoadStringMap( DEFAULT_SNIPPET_FILE, d->m_Snippets );
  }

  d->m_PasteSnippet = new QAction(this);
  d->m_PasteSnippet->setObjectName(QString::fromUtf8("PasteSnippet"));
  QIcon icon;
  icon.addFile(QString::fromUtf8(":/mitkPython/edit-paste.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_PasteSnippet->setIcon(icon);
  d->m_PasteSnippet->setToolTip("Paste snippet!");
  d->m_PasteSnippet->setEnabled(false);

  d->m_RemoveSnippet = new QAction(this);
  d->m_RemoveSnippet->setObjectName(QString::fromUtf8("RemoveSnippet"));
  QIcon icon1;
  icon1.addFile(QString::fromUtf8(":/mitkPython/edit-delete.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RemoveSnippet->setIcon(icon1);
  d->m_RemoveSnippet->setToolTip("Remove snippet.");
  d->m_RemoveSnippet->setEnabled(false);

  d->m_RenameSnippet = new QAction(this);
  d->m_RenameSnippet->setObjectName(QString::fromUtf8("RenameSnippet"));
  QIcon icon2;
  icon2.addFile(QString::fromUtf8(":/mitkPython/edit-find-replace.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RenameSnippet->setIcon(icon2);
  d->m_RenameSnippet->setToolTip("Rename snippet.");
  d->m_RenameSnippet->setEnabled(false);

  d->m_AddSnippet = new QAction(this);
  d->m_AddSnippet->setObjectName(QString::fromUtf8("AddSnippet"));
  QIcon icon3;
  icon3.addFile(QString::fromUtf8(":/mitkPython/document-new.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_AddSnippet->setIcon(icon3);
  d->m_AddSnippet->setToolTip("Add snippet.");

  d->m_RestoreDefaultSnippets = new QAction(this);
  d->m_RestoreDefaultSnippets->setObjectName(QString::fromUtf8("RestoreDefaultSnippets"));
  QIcon icon4;
  icon4.addFile(QString::fromUtf8(":/mitkPython/edit-clear.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RestoreDefaultSnippets->setIcon(icon4);
  d->m_RestoreDefaultSnippets->setToolTip("Restore default snippets");

  d->m_LoadSnippets = new QAction(this);
  d->m_LoadSnippets->setToolTip("Load Snippets from disk.");
  d->m_LoadSnippets->setObjectName(QString::fromUtf8("LoadSnippets"));
  QIcon icon5;
  icon5.addFile(QString::fromUtf8(":/mitkPython/document-open.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_LoadSnippets->setIcon(icon5);

  d->m_SaveSnippets = new QAction(this);
  d->m_SaveSnippets->setToolTip("Save Snippets to disk.");
  d->m_SaveSnippets->setObjectName(QString::fromUtf8("SaveSnippets"));
  QIcon icon6;
  icon6.addFile(QString::fromUtf8(":/mitkPython/document-save.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_SaveSnippets->setIcon(icon6);
  d->m_SaveSnippets->setEnabled(false);

  d->m_Toolbar = new QToolBar;
  d->m_Toolbar->addAction( d->m_PasteSnippet );
  d->m_Toolbar->addAction( d->m_AddSnippet );
  d->m_Toolbar->addAction( d->m_RemoveSnippet );
  d->m_Toolbar->addAction( d->m_RenameSnippet );
  d->m_Toolbar->addAction( d->m_RestoreDefaultSnippets );
  d->m_Toolbar->addAction( d->m_SaveSnippets );
  d->m_Toolbar->addAction( d->m_LoadSnippets );

  d->m_Name = new QComboBox;
  d->m_Name->setObjectName(QString::fromUtf8("Name"));

  d->m_Content = new QTextEdit(this);
  d->m_Content->setObjectName(QString::fromUtf8("Content"));
  d->m_Content->setEnabled(false);

  QmitkPythonScriptEditorHighlighter* highlighter =
      new QmitkPythonScriptEditorHighlighter( d->m_Content->document() );

  d->m_Layout = new QGridLayout;
  d->m_Layout->addWidget( d->m_Toolbar, 0, 0, 1, 1 );
  d->m_Layout->addWidget( d->m_Name, 1, 0, 1, 1 );
  d->m_Layout->addWidget( d->m_Content, 2, 0, 1, 1 );
  d->m_Layout->setContentsMargins(2,2,2,2);

  this->setLayout(d->m_Layout);
  QMetaObject::connectSlotsByName(this);

  this->Update();
}

QmitkPythonSnippets::~QmitkPythonSnippets()
{
  delete d;
}

void QmitkPythonSnippets::on_PasteSnippet_triggered( bool )
{
  emit PasteCommandRequested( d->m_Content->toPlainText() );
}

void QmitkPythonSnippets::on_RenameSnippet_triggered(bool)
{
  QString oldname = d->m_Name->currentText();
  QString name = oldname;
  bool ok = false;
  while( true )
  {
    name = QInputDialog::getText(this,
                                 tr("Add new snippet"),
                                 tr("Name of snippet:"),
                                 QLineEdit::Normal,
                                 name,
                                 &ok);

    if (ok)
    {
      if ( d->m_Snippets.contains(name) )
      {
        QMessageBox::warning(this,
                             tr("Duplicate name."),
                             tr("The entered name already exists. Enter another one or cancel the operation."),
                             QMessageBox::Ok,
                             QMessageBox::Ok );
      }
      else
      {
        QString tmpSnippet = d->m_Snippets[oldname];
        d->m_Snippets.remove(oldname);
        d->m_Snippets[name] = tmpSnippet;
        this->Update(name);
        this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
        break;
      }
    }
    else
    {
      break;
    }
  }
}

void QmitkPythonSnippets::on_AddSnippet_triggered(bool)
{
  bool ok;
  QString name = QInputDialog::getText(this,
                                       tr("Add new snippet"),
                                       tr("Name of snippet:"),
                                       QLineEdit::Normal,
                                       "newSnippet",
                                       &ok);
  if (ok && !name.isEmpty())
  {
    MITK_DEBUG("QmitkPythonSnippets") << "creating unique name for " << name.toStdString();
    name = this->CreateUniqueName(name);

    MITK_DEBUG("QmitkPythonSnippets") << "creating snippet " << name.toStdString();
    d->m_Snippets[name] = "";
    this->Update(name);
    this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
  }
}

QString QmitkPythonSnippets::CreateUniqueName( const QString& name ) const
{
  QString newName = name;
  size_t i = 2;
  while( d->m_Snippets.contains(name) )
  {
    newName = name + QString("_%1").arg(i);
    ++i;
  }

  return newName;
}

void QmitkPythonSnippets::on_RemoveSnippet_triggered(bool)
{
  QString name = d->m_Name->currentText();
  QString question = QString("Really remove Snippet %1?").arg(name);
  int remove = QMessageBox::question( this,
                                      QString("Confirm removal"),
                                      question,
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No );

  if( remove == QMessageBox::Yes || remove == QMessageBox::Ok )
  {
    d->m_Snippets.remove(name);
    this->Update();
    this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
  }
}

void QmitkPythonSnippets::on_RestoreDefaultSnippets_triggered(bool)
{
  QString question = QString("Really restore default Snippets?");
  int remove = QMessageBox::question( this,
                                      QString("Confirm restoring"),
                                      question,
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No );
  if( remove == QMessageBox::Yes || remove == QMessageBox::Ok )
  {
    QmitkPythonSnippets::LoadStringMap( DEFAULT_SNIPPET_FILE, d->m_Snippets );
    this->Update();
    this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
  }
}

void QmitkPythonSnippets::on_Name_currentIndexChanged(int i)
{
  bool validSelection =  i >= 0 ;

  d->m_PasteSnippet->setEnabled(validSelection);
  d->m_RemoveSnippet->setEnabled(validSelection);
  d->m_RenameSnippet->setEnabled(validSelection);
  d->m_Content->setEnabled(validSelection);
  d->m_SaveSnippets->setEnabled(validSelection);

  if( validSelection )
  {
    QString name = d->m_Name->currentText();
    MITK_DEBUG("QmitkPythonSnippets") << "selected snippet " << name.toStdString();
    d->m_Content->setText( d->m_Snippets[name] );
    MITK_DEBUG("QmitkPythonSnippets") << "selected snippet content " <<  d->m_Snippets[name].toStdString();
  }
}

void QmitkPythonSnippets::SaveStringMap(const QString &filename, const QmitkPythonSnippets::QStringMap &map) const
{
  MITK_DEBUG("QmitkPythonSnippets") << "saving to xml file " << filename.toStdString();

  if( filename.isEmpty() )
  {
    MITK_WARN("QmitkPythonSnippets") << "empty auto save file path given. quit.";
    return;
  }

  QFile file(filename);
  file.open(QIODevice::WriteOnly);
  if( !file.isOpen() )
  {
    MITK_WARN("QmitkPythonSnippets") << "could not open file " << filename.toStdString() << " for writing";
    return;
  }
  QXmlStreamWriter xmlWriter(&file);

  xmlWriter.setAutoFormatting(true);
  xmlWriter.writeStartDocument();
  xmlWriter.writeStartElement(SNIPPETS_ROOT_XML_ELEMENT_NAME);

  QStringMap::const_iterator it = d->m_Snippets.begin();
  while( it != d->m_Snippets.end() )
  {

    {
      MITK_DEBUG("QmitkPythonSnippets") << "SNIPPETS_XML_ELEMENT_NAME " << SNIPPETS_XML_ELEMENT_NAME.toStdString();
      MITK_DEBUG("QmitkPythonSnippets") << "writing item " << it.key().toStdString();
    }

    xmlWriter.writeStartElement(SNIPPETS_XML_ELEMENT_NAME);

    xmlWriter.writeAttribute( "key", it.key() );
    xmlWriter.writeAttribute( "value", it.value() );

    xmlWriter.writeEndElement();

    ++it;
  }

  xmlWriter.writeEndDocument();
  if( file.isOpen() )
    file.close();

  {
    MITK_DEBUG("QmitkPythonSnippets") << "SaveStringMap successful ";
  }

}

bool QmitkPythonSnippets::LoadStringMap( const QString& filename, QmitkPythonSnippets::QStringMap& oldMap )
{
  MITK_DEBUG("QmitkPythonSnippets") << "loading from xml file " << filename.toStdString();
  QStringMap map;

  QXmlStreamReader xmlReader;
  QFile file;
  QByteArray data;
  // resource file
  if( filename.startsWith(":") )
  {
    QResource res( filename );
    data = QByteArray( reinterpret_cast< const char* >( res.data() ), res.size() );
    xmlReader.addData( data );
  }
  else
  {
    file.setFileName( filename );
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        MITK_ERROR << "Error: Cannot read file " << qPrintable(filename)
         << ": " << qPrintable(file.errorString());
        return false;
    }
    xmlReader.setDevice(&file);
  }

  xmlReader.readNext();

  while(!xmlReader.atEnd())
  {
    xmlReader.readNext();

    if(xmlReader.name() == SNIPPETS_XML_ELEMENT_NAME)
    {
      QXmlStreamAttributes attributes = xmlReader.attributes();
      QString key;
      QString value;
      if(attributes.hasAttribute("key"))
      {
        key = attributes.value("key").toString();
      }

      if(attributes.hasAttribute("value"))
      {
        value = attributes.value("value").toString();
      }

      if( !key.isEmpty() )
      {
        MITK_DEBUG("QmitkPythonSnippets") << "loaded snippet " << key.toStdString();
        MITK_DEBUG("QmitkPythonSnippets") << "value " << value.toStdString();
        map[key] = value;
      }
    }
  }

  if (xmlReader.hasError())
  {
    MITK_ERROR << "Error: Failed to parse file "
      << qPrintable(filename) << ": "
      << qPrintable(xmlReader.errorString());
    return false;
  }
  else if (file.error() != QFile::NoError)
  {
    MITK_ERROR << "Error: Cannot read file " << qPrintable(filename)
      << ": " << qPrintable(file.errorString());
    return false;
  }

  if( file.isOpen() )
    file.close();

  oldMap = map;
  return true;
}

void QmitkPythonSnippets::Update(const QString &name)
{
  d->m_Name->clear();
  d->m_Content->clear();

  MITK_DEBUG("QmitkPythonSnippets") << "size of snippets " << d->m_Snippets.size();
  QStringMap::const_iterator it = d->m_Snippets.begin();

  while( it != d->m_Snippets.end() )
  {
    MITK_DEBUG("QmitkPythonSnippets") << "adding item " << it.key().toStdString();
    d->m_Name->addItem( it.key() );
    ++it;
  }

  int index = d->m_Name->findText( name );
  if( index >= 0 )
  {
    MITK_DEBUG("QmitkPythonSnippets") << "selecting index " << index;
    d->m_Name->setCurrentIndex(index);
  }

}

void QmitkPythonSnippets::on_Content_textChanged()
{
  if( d->m_Content->isEnabled() )
  {
    QString name = d->m_Name->currentText();
    QString snippet = d->m_Content->toPlainText();
    d->m_Snippets[name] = snippet;

    this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
    MITK_DEBUG("QmitkPythonSnippets") << "SaveStringMap successful";
  }
}

void QmitkPythonSnippets::on_SaveSnippets_triggered(bool)
{
  QString fileName = QFileDialog::getSaveFileName(this, "Save snippets", d->m_SaveFileName, "XML files (*.xml)");
  if( !fileName.isEmpty() )
  {
    d->m_SaveFileName = fileName;
    this->SaveStringMap( d->m_SaveFileName, d->m_Snippets );
  }
}

void QmitkPythonSnippets::on_LoadSnippets_triggered(bool)
{
  QString fileName = QFileDialog::getOpenFileName(this, "Load snippets", d->m_SaveFileName, "XML files (*.xml)");

  if( !fileName.isEmpty() )
  {
    d->m_SaveFileName = fileName;
    QString question = QString("Your current snippets will be overwritten. Proceed?");
    int overwrite = QMessageBox::warning(this,
                                        QString("Confirm overwrite"),
                                        question,
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No );

    if( overwrite == QMessageBox::Yes )
    {
      this->LoadStringMap( d->m_SaveFileName, d->m_Snippets );
      this->Update( d->m_Name->currentText() );
      this->SaveStringMap( d->m_AutoSaveFileName, d->m_Snippets );
    }
  }

}
