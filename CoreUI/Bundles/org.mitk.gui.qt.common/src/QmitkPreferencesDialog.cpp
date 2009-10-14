/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-07-08 13:02:46 +0200 (Mi, 08 Jul 2009) $
 Version:   $Revision: 18037 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkPreferencesDialog.h"

#include "cherryPlatform.h"
#include "cherryIPreferencePage.h"
#include "cherryIConfigurationElement.h"
#include "cherryIExtensionPointService.h"
#include "cherryIExtension.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QEvent>
#include <QTreeWidgetItem>
#include <QIcon>
#include <QStackedWidget>
#include <QFileDialog>
#include <QMessageBox>

#include <algorithm>

using namespace std;

QmitkPreferencesDialog::QmitkPreferencesDialog(QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f)
{
  // m_PreferencesService
  m_PreferencesService = 
    cherry::Platform::GetServiceRegistry().GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

  // m_PrefPages
  cherry::IExtensionPointService::Pointer extensionPointService = cherry::Platform::GetExtensionPointService();
  cherry::IConfigurationElement::vector prefPages(extensionPointService->GetConfigurationElementsFor("org.opencherry.ui.preferencePages"));
  cherry::IConfigurationElement::vector keywordExts(extensionPointService->GetConfigurationElementsFor("org.opencherry.ui.keywords"));      
  cherry::IConfigurationElement::vector::iterator prefPagesIt;
  std::string id;
  std::string name;
  std::string category;
  std::string className;
  std::vector<std::string> keywords;
  vector<cherry::IConfigurationElement::Pointer> keywordRefs;
  cherry::IConfigurationElement::vector::iterator keywordRefsIt;
  cherry::IConfigurationElement::vector::iterator keywordExtsIt;
  string keywordRefId;
  string keywordId;
  string keywordLabels;

  for (prefPagesIt = prefPages.begin(); prefPagesIt != prefPages.end(); ++prefPagesIt)
  {
    keywords.clear();
    id.clear();
    name.clear();
    className.clear();
    category.clear();
    keywordRefId.clear();
    keywordId.clear();
    keywordLabels.clear();

    if((*prefPagesIt)->GetAttribute("id", id)
      && (*prefPagesIt)->GetAttribute("name", name)
      && (*prefPagesIt)->GetAttribute("class", className))
    {
      (*prefPagesIt)->GetAttribute("category", category);
      //# collect keywords
      keywordRefs = (*prefPagesIt)->GetChildren("keywordreference"); // get all keyword references
      for (keywordRefsIt = keywordRefs.begin()
        ; keywordRefsIt != keywordRefs.end(); ++keywordRefsIt) // iterate over all refs
      {
        (*keywordRefsIt)->GetAttribute("id", keywordRefId); // get referenced id

        for (keywordExtsIt = keywordExts.begin(); keywordExtsIt != keywordExts.end(); ++keywordExtsIt) // iterate over all keywords
        {
          (*keywordExtsIt)->GetAttribute("id", keywordId); // get keyword id
          if(keywordId == keywordRefId) // if referenced id is equals the current keyword id
          {
            //# collect all keywords from label attribute with a tokenizer
            (*keywordExtsIt)->GetAttribute("label", keywordLabels);

            break; // break here; possibly search for other referenced keywords
          }
        }
      }

      // add information as PrefPage
      m_PrefPages.push_back(PrefPage(id, name, category, className, keywordLabels, cherry::IConfigurationElement::Pointer(*prefPagesIt)));
    }
    
  }

  //#QWidget

  //# m_Keyword
  m_Keyword = new QLineEdit("");
  //m_Keyword->installEventFilter(this);
  QObject::connect(m_Keyword, SIGNAL(editingFinished()), this, SLOT(OnKeywordEditingFinished()));
  QObject::connect(m_Keyword, SIGNAL(textChanged ( const QString & )), this, SLOT(OnKeywordTextChanged(const QString &)));

  //# m_PreferencesTree
  m_PreferencesTree = new QTreeWidget;
  m_PreferencesTree->setHeaderHidden(true);
  QObject::connect(m_PreferencesTree, SIGNAL(itemSelectionChanged()), this, SLOT(OnPreferencesTreeItemSelectionChanged()));

  //# m_LeftLayout
  m_LeftLayout = new QGridLayout;
  m_LeftLayout->setContentsMargins(0,0,0,0);
  m_LeftLayout->setSpacing(0);
  m_LeftLayout->addWidget(m_Keyword, 0, 0);
  m_LeftLayout->addWidget(m_PreferencesTree, 1, 0);

  //# m_LeftPanel
  m_LeftPanel = new QWidget;
  m_LeftPanel->setLayout(m_LeftLayout);

  //# m_Headline
  m_Headline = new QLabel("");
  m_Headline->setStyleSheet("background-color: 	silver; "
    "border-style: solid; border-width: 1px;"
    "border-color: white; font: bold 13px; color: white;");

  //# m_PreferencesPanel
  m_PreferencesPanel = new QStackedWidget;

  //# m_RightLayout
  m_RightLayout = new QGridLayout;
  m_RightLayout->setRowStretch(0, 1);
  m_RightLayout->setRowStretch(1, 20);
  m_RightLayout->setContentsMargins(0,0,0,0);
  m_RightLayout->setSpacing(0);
  m_RightLayout->addWidget(m_Headline, 0, 0);
  m_RightLayout->addWidget(m_PreferencesPanel, 1, 0);

  //# m_RightPanel
  m_RightPanel = new QWidget;
  m_RightPanel->setLayout(m_RightLayout);

  //# m_Splitter
  m_Splitter = new QSplitter(Qt::Horizontal);
  m_Splitter->setContentsMargins(0,0,0,0);
  //m_Splitter->setFrameStyle(QFrame::Plain | QFrame::Sunken);
  m_Splitter->setLineWidth(2);
  m_Splitter->addWidget(m_LeftPanel);
  m_Splitter->addWidget(m_RightPanel);
  //QList<int> sizes; sizes.push_back(this->width()*(1/3)); sizes.push_back(this->width()*(2/3));
  //m_Splitter->setSizes(sizes);
  m_Splitter->setStretchFactor(0,0);
  m_Splitter->setStretchFactor(1,2);

  //# m_ImportButton
  m_ImportButton = new QPushButton(QString(tr("Import ...")));
  QObject::connect(m_ImportButton, SIGNAL(clicked(bool)), this, SLOT(OnImportButtonClicked(bool)));
  m_ImportButton->setIcon(QIcon(":/org.mitk.gui.qt.application/edit-redo.png"));
  m_ImportButton->setFlat(true);
  m_ImportButton->setDefault(false);

  //# m_ExportButton
  m_ExportButton = new QPushButton(QString(tr("Export ...")), this);
  QObject::connect(m_ExportButton, SIGNAL(clicked(bool)), this, SLOT(OnExportButtonClicked(bool)));
  m_ExportButton->setIcon(QIcon(":/org.mitk.gui.qt.application/edit-undo.png"));
  m_ExportButton->setFlat(true);
  m_ExportButton->setDefault(false);

  //# m_ApplyButton
  m_ApplyButton = new QPushButton(QString(tr("Apply")), this);
  QObject::connect(m_ApplyButton, SIGNAL(clicked(bool)), this, SLOT(OnApplyButtonClicked(bool)));
  m_ApplyButton->setIcon(QIcon(":/org.mitk.gui.qt.application/document-save.png"));
  m_ApplyButton->setFlat(true);
  m_ApplyButton->setDefault(false);

  //# m_CloseButton
  m_CloseButton = new QPushButton(QString(tr("Close")), this);
  QObject::connect(m_CloseButton, SIGNAL(clicked(bool)), this, SLOT(OnCloseButtonClicked(bool)));
  m_CloseButton->setIcon(QIcon(":/org.mitk.gui.qt.application/system-log-out.png"));
  m_CloseButton->setFlat(true);
  m_CloseButton->setDefault(true);
  m_CloseButton->setFocus();

  //# m_Layout
  m_Layout = new QGridLayout;
  //m_Layout->setContentsMargins(0,0,0,0);
  //m_Layout->setSpacing(2);
  //m_Layout->setRowStretch(0, 20);
  //m_Layout->setRowStretch(1, 1);
  m_Layout->addWidget(m_Splitter, 0, 0, 1, 4);
  m_Layout->addWidget(m_ImportButton, 2, 0);
  m_Layout->addWidget(m_ExportButton, 2, 1);
  m_Layout->addWidget(m_ApplyButton, 2, 2);
  m_Layout->addWidget(m_CloseButton, 2, 3);

  // this
  this->setModal(true);
  this->setSizeGripEnabled(true);
  this->setWhatsThis("Dialog to set application wide preferences");
  this->setWindowIcon(QIcon(":/org.mitk.gui.qt.application/preferences-system.png"));
  QRect parentGeometry = parent->geometry();
  int w = parentGeometry.width()/2; int h = parentGeometry.height()/2;
  int x = parentGeometry.x()+(parentGeometry.width()-w)/2; 
  int y = parentGeometry.y()+(parentGeometry.height()-h)/2;
  this->setGeometry(x,y,w,h);
  this->setWindowTitle("Preferences");
  this->setLayout(m_Layout);
  this->UpdateTree();
}

QmitkPreferencesDialog::~QmitkPreferencesDialog()
{
}


void QmitkPreferencesDialog::OnImportButtonClicked( bool triggered )
{  
  int answer = QMessageBox::question(this, "Importing Preferences"
    , "All existing preferences will be overwritten!\nAre you sure that you want to import other preferences?", QMessageBox::Yes | QMessageBox::No );
  if(answer == QMessageBox::No)
    return;

  try
  { 
    cherry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
    if(prefService.IsNotNull())
    {
      cherry::ICherryPreferencesService* cherryPrefService
        = dynamic_cast<cherry::ICherryPreferencesService*>(prefService.GetPointer());
      if(cherryPrefService != 0)
      {
        QFileDialog fd(this, "Choose file to import preferences", "", "XML files (*.xml)" );
        fd.setFileMode(QFileDialog::ExistingFile);
        QStringList fileNames;
        if (fd.exec())
        {
          fileNames = fd.selectedFiles();
          if(fileNames.size()>0)
          {
            Poco::File f(fileNames.at(0).toStdString());
            cherryPrefService->ImportPreferences(f, "");
            cherry::IQtPreferencePage* prefPage 
              = dynamic_cast<cherry::IQtPreferencePage*>(m_PreferencesPanel->currentWidget());
            if(prefPage)
              prefPage->Update();

            LOG_INFO("QmitkPreferencesDialog") << "Preferences successfully imported from " << f.path();
          }
        }
      }
    }
  }
  catch (Poco::Exception& pe)
  {
    QMessageBox::critical(this, "Error Importing", pe.message().c_str());
    LOG_ERROR("QmitkPreferencesDialog") << pe.what();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(this, "Error Importing", e.what());
    LOG_ERROR("QmitkPreferencesDialog") << e.what();
  }
}

void QmitkPreferencesDialog::OnExportButtonClicked( bool triggered )
{
  try
  { 
    cherry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
    if(prefService.IsNotNull())
    {
      cherry::ICherryPreferencesService* cherryPrefService
        = dynamic_cast<cherry::ICherryPreferencesService*>(prefService.GetPointer());
      if(cherryPrefService != 0)
      {
        QFileDialog fd(this, "Choose file to import preferences", "", "XML files (*.xml)" );
        fd.setFileMode(QFileDialog::AnyFile);
        QStringList fileNames;
        if (fd.exec())
        {
          fileNames = fd.selectedFiles();
          if(fileNames.size()>0)
          {
            Poco::File f(fileNames.at(0).toStdString());
            cherryPrefService->ExportPreferences(f, "");
            LOG_INFO("QmitkPreferencesDialog") << "Preferences successfully exported to " << f.path();
          }
        }
      }
    }
  }
  catch (Poco::Exception& pe)
  {
    QMessageBox::critical(this, "Error Exporting", pe.message().c_str());
    LOG_ERROR("QmitkPreferencesDialog") << pe.what();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(this, "Error Exporting", e.what());
    LOG_ERROR("QmitkPreferencesDialog") << e.what();
  }
}

void QmitkPreferencesDialog::OnApplyButtonClicked( bool triggered )
{  
  cherry::IQtPreferencePage* prefPage 
    = dynamic_cast<cherry::IQtPreferencePage*>(m_PreferencesPanel->currentWidget());
  if(prefPage)
    if(prefPage->PerformOk())
      this->done(QDialog::Accepted);
}

void QmitkPreferencesDialog::OnCloseButtonClicked( bool triggered )
{
  cherry::IQtPreferencePage* prefPage 
    = dynamic_cast<cherry::IQtPreferencePage*>(m_PreferencesPanel->currentWidget());
  if(prefPage)
    prefPage->PerformCancel();

  this->done(QDialog::Accepted);
}

void QmitkPreferencesDialog::OnKeywordTextChanged(const QString & s)
{
  // search for text
  this->UpdateTree();

}

void QmitkPreferencesDialog::OnKeywordEditingFinished()
{
}

bool QmitkPreferencesDialog::eventFilter( QObject *obj, QEvent *event )
{
  if(obj = m_Keyword)
  {
    if(event->type() == QEvent::FocusIn && m_Keyword->text() == "search ...")
    {
      m_Keyword->setText("");
      m_Keyword->setStyleSheet("color: black;");
    }
    else if(event->type() == QEvent::FocusOut && m_Keyword->text() == "")
    {
      m_Keyword->setText("search ...");
      m_Keyword->setStyleSheet("color: gray;");
    }
  }
  return true;
}

void QmitkPreferencesDialog::OnPreferencesTreeItemSelectionChanged()
{
  if(m_PreferencesTree == 0)
    return;

  // TODO: create page and show it
  QList<QTreeWidgetItem *> selectedItems = m_PreferencesTree->selectedItems();
  if(selectedItems.size()>0)
  {

    for(vector<PrefPage>::iterator it = m_PrefPages.begin();
      it != m_PrefPages.end(); ++it)
    {
      if(it->treeWidgetItem == selectedItems.at(0))
      {
        m_Headline->setText(QString::fromStdString(it->name));
        if(it->prefPage == 0)
        {
          it->prefPage = it->confElem->CreateExecutableExtension<cherry::IQtPreferencePage>("class");
          m_PreferencesPanel->addWidget(it->prefPage);
        }
        m_PreferencesPanel->setCurrentWidget(it->prefPage);
        
        break;
      }
    }
  }
}

void QmitkPreferencesDialog::UpdateTree()
{
  if(m_PreferencesTree == 0)
    return;

  //m_PreferencesTree->clear();
  string keyword = m_Keyword->text().toStdString();

  map<std::string, QTreeWidgetItem*> items;

  for(vector<PrefPage>::iterator it = m_PrefPages.begin();
    it != m_PrefPages.end(); ++it)
  {
    if(it->treeWidgetItem == 0)
    {

      if(it->category.empty())
      {
        it->treeWidgetItem = new QTreeWidgetItem(m_PreferencesTree);
      }
      else
      {
        it->treeWidgetItem = new QTreeWidgetItem(items[it->category]);
      }
      it->treeWidgetItem->setText(0, QString::fromStdString(it->name));
      items[it->id] = it->treeWidgetItem;
    }

    // hide treeWidgetItem if keyword not matches
    if(!keyword.empty())
    {
      if( it->keywords.find(keyword) == string::npos )
        it->treeWidgetItem->setHidden(true);
      else
      {
        //#make the whole branch visible
        QTreeWidgetItem* treeWidgetParent = it->treeWidgetItem->parent();
        while(treeWidgetParent!=0)
        {
          treeWidgetParent->setHidden(false);
          treeWidgetParent->setExpanded(true);
          treeWidgetParent = treeWidgetParent->parent();
        }

        it->treeWidgetItem->setHidden(false);
        QFont f = it->treeWidgetItem->font(0);
        f.setBold(true);
        it->treeWidgetItem->setFont(0, f);
      }
    }
    else
    {
      QFont f = it->treeWidgetItem->font(0);
      f.setBold(false);
      it->treeWidgetItem->setFont(0, f);
      it->treeWidgetItem->setHidden(false);
    }
  }

  if(m_PrefPages.size()>0)
  {
    if(m_PrefPages.front().treeWidgetItem != 0)
      m_PrefPages.front().treeWidgetItem->setSelected(true);
  }

}

bool QmitkPreferencesDialog::PrefPage::operator==( const PrefPage& other )
{
  return id == other.id;
}

bool QmitkPreferencesDialog::PrefPage::operator<( const PrefPage& other )
{
  return name < other.name;
}

QmitkPreferencesDialog::PrefPage::PrefPage( std::string _id, std::string _name, std::string _category
                                           , std::string _className, std::string _keywords
                                           , cherry::IConfigurationElement::Pointer _confElem )
: id(_id)
, name(_name)
, category(_category)
, className(_className)
, keywords(_keywords)
, prefPage(0)
, confElem(_confElem)
, treeWidgetItem(0)
{

}