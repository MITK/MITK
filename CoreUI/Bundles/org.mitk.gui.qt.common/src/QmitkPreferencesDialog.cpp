/*=========================================================================
 Program:   BlueBerry Platform
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

#include "berryPlatform.h"
#include "berryIConfigurationElement.h"
#include "berryIExtensionPointService.h"
#include "berryIExtension.h"
#include <berryIBerryPreferencesService.h>
#include <berryIQtPreferencePage.h>

#include <ui_QmitkPreferencesDialog.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include <algorithm>

#include <mitkLogMacros.h>

using namespace std;

class QmitkPreferencesDialogPrivate : public Ui::QmitkPreferencesDialog
{
public:

  ///
  /// Just a stub class for holding information on prefpages (metadata)
  ///
  struct PrefPage
  {
    PrefPage(std::string _id, std::string _name, std::string _category
      , std::string _className, std::string _keywords, berry::IConfigurationElement::Pointer _confElem)
      : id(_id), name(_name), category(_category), className(_className), keywords(_keywords),
        prefPage(0), confElem(_confElem), treeWidgetItem(0)
    {}

    bool operator==(const PrefPage& other)
    { return id == other.id; }

    bool operator<(const PrefPage& other)
    { return name < other.name; }

    std::string id;
    std::string name;
    std::string category;
    std::string className;
    std::string keywords;
    berry::IQtPreferencePage* prefPage;
    berry::IConfigurationElement::Pointer confElem;
    QTreeWidgetItem* treeWidgetItem;
  };

  QmitkPreferencesDialogPrivate()
    : m_CurrentPage(0)
  {
    // m_PreferencesService
    m_PreferencesService =
      berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

    // m_PrefPages
    berry::IExtensionPointService::Pointer extensionPointService = berry::Platform::GetExtensionPointService();
    berry::IConfigurationElement::vector prefPages(extensionPointService->GetConfigurationElementsFor("org.blueberry.ui.preferencePages"));
    berry::IConfigurationElement::vector keywordExts(extensionPointService->GetConfigurationElementsFor("org.blueberry.ui.keywords"));
    berry::IConfigurationElement::vector::iterator prefPagesIt;
    std::string id;
    std::string name;
    std::string category;
    std::string className;
    std::vector<std::string> keywords;
    vector<berry::IConfigurationElement::Pointer> keywordRefs;
    berry::IConfigurationElement::vector::iterator keywordRefsIt;
    berry::IConfigurationElement::vector::iterator keywordExtsIt;
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
              std::string currLabel;
              (*keywordExtsIt)->GetAttribute("label", currLabel);
              std::transform(currLabel.begin(), currLabel.end(), currLabel.begin(), ::tolower);
              if (!currLabel.empty()) keywordLabels += std::string(" ") + currLabel;

              //break; // break here; possibly search for other referenced keywords
            }
          }
        }

        // add information as PrefPage
        m_PrefPages.push_back(PrefPage(id, name, category, className, keywordLabels, berry::IConfigurationElement::Pointer(*prefPagesIt)));
      }

    }
  }

  ///
  /// The Preferences Service to retrieve and store preferences.
  ///
  berry::IPreferencesService::WeakPtr m_PreferencesService;

  ///
  /// Saves all treewidgetitems in a map, the key is the id of the preferencepage.
  ///
  std::vector<PrefPage> m_PrefPages;
  std::size_t m_CurrentPage;
};

QmitkPreferencesDialog::QmitkPreferencesDialog(QWidget * parent, Qt::WindowFlags f)
  : QDialog(parent, f), d(new QmitkPreferencesDialogPrivate)
{
  d->setupUi(this);

  QObject::connect(d->m_Keyword, SIGNAL(editingFinished()), this, SLOT(OnKeywordEditingFinished()));
  QObject::connect(d->m_Keyword, SIGNAL(textChanged(QString)), this, SLOT(OnKeywordTextChanged(QString)));

  QObject::connect(d->m_PreferencesTree, SIGNAL(itemSelectionChanged()), this, SLOT(OnPreferencesTreeItemSelectionChanged()));

  QPushButton* importButton = d->buttonBox->addButton("Import...", QDialogButtonBox::ActionRole);
  QObject::connect(importButton, SIGNAL(clicked()), this, SLOT(OnImportButtonClicked()));

  QPushButton* exportButton = d->buttonBox->addButton("Export...", QDialogButtonBox::ActionRole);
  QObject::connect(exportButton, SIGNAL(clicked()), this, SLOT(OnExportButtonClicked()));

  QObject::connect(this, SIGNAL(accepted()), this, SLOT(OnDialogAccepted()));
  QObject::connect(this, SIGNAL(rejected()), this, SLOT(OnDialogRejected()));

  d->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

  this->UpdateTree();
}

QmitkPreferencesDialog::~QmitkPreferencesDialog()
{
}

void QmitkPreferencesDialog::SetSelectedPage(const std::string& id)
{
  for(vector<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
  {
    if(it->id == id)
    {
      d->m_PreferencesTree->setCurrentItem(it->treeWidgetItem);
      break;
    }
  }
}

void QmitkPreferencesDialog::OnImportButtonClicked()
{  
  int answer = QMessageBox::question(this, "Importing Preferences"
    , "All existing preferences will be overwritten!\nAre you sure that you want to import other preferences?", QMessageBox::Yes | QMessageBox::No );
  if(answer == QMessageBox::No)
    return;

  try
  { 
    berry::IPreferencesService::Pointer prefService = d->m_PreferencesService.Lock();
    if(prefService.IsNotNull())
    {
      berry::IBerryPreferencesService::Pointer berryPrefService = prefService.Cast<berry::IBerryPreferencesService>();
      if(berryPrefService != 0)
      {
        static QString importDir = "";
        QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file to import preferences"),
                                                        importDir, tr("XML files (*.xml)"));

        if(!fileName.isEmpty())
        {
          importDir = QFileInfo(fileName).absoluteDir().path();
          Poco::File f(fileName.toLocal8Bit().data());
          berryPrefService->ImportPreferences(f, "");
          berry::IQtPreferencePage* prefPage = d->m_PrefPages[d->m_CurrentPage].prefPage;
          if(prefPage)
            prefPage->Update();

          MITK_INFO("QmitkPreferencesDialog") << "Preferences successfully imported from " << f.path();
        }
      }
    }
  }
  catch (Poco::Exception& pe)
  {
    QMessageBox::critical(this, "Error Importing", pe.message().c_str());
    MITK_ERROR("QmitkPreferencesDialog") << pe.what();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(this, "Error Importing", e.what());
    MITK_ERROR("QmitkPreferencesDialog") << e.what();
  }
}

void QmitkPreferencesDialog::OnExportButtonClicked()
{
  try
  { 
    berry::IPreferencesService::Pointer prefService = d->m_PreferencesService.Lock();
    if(prefService.IsNotNull())
    {
      berry::IBerryPreferencesService::Pointer berryPrefService = prefService.Cast<berry::IBerryPreferencesService>();
      if(berryPrefService != 0)
      {
        SavePreferences();
        static QString exportDir = "";
        QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file to export preferences"),
                                                        exportDir, tr("XML files (*.xml)"));

        if(!fileName.isEmpty())
        {
          if(QFileInfo(fileName).suffix() != ".xml")
          {
            fileName += ".xml";
          }
          exportDir = QFileInfo(fileName).absoluteDir().path();
          Poco::File f(fileName.toLocal8Bit().data());
          berryPrefService->ExportPreferences(f, "");
          MITK_INFO("QmitkPreferencesDialog") << "Preferences successfully exported to " << f.path();
        }
      }
    }
  }
  catch (Poco::Exception& pe)
  {
    QMessageBox::critical(this, "Error Exporting", pe.message().c_str());
    MITK_ERROR("QmitkPreferencesDialog") << pe.what();
  }
  catch (std::exception& e)
  {
    QMessageBox::critical(this, "Error Exporting", e.what());
    MITK_ERROR("QmitkPreferencesDialog") << e.what();
  }
}

void QmitkPreferencesDialog::SavePreferences()
{
  berry::IQtPreferencePage* prefPage = 0;

  for(vector<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
  {
    prefPage = it->prefPage;
    if(prefPage) {
      prefPage->PerformOk();
    }
  }

  /**
   * Every preference page has its own preferences, which should stay the same after a system restart. <br>
   * Therefore this method flushes all the preferences, every time a change in the preferences is <br>
   * performed and confirmed.
   *
   */
  berry::IPreferencesService::Pointer prefService = d->m_PreferencesService.Lock();
  if (prefService)
  {
    prefService->GetSystemPreferences()->Flush();
  }
}

void QmitkPreferencesDialog::OnDialogAccepted()
{  
  this->SavePreferences();
}

void QmitkPreferencesDialog::OnDialogRejected()
{
  berry::IQtPreferencePage* prefPage = d->m_PrefPages[d->m_CurrentPage].prefPage;
  if(prefPage)
    prefPage->PerformCancel();
}

void QmitkPreferencesDialog::OnKeywordTextChanged(const QString &  /*s*/)
{
  // search for text
  this->UpdateTree();
}

void QmitkPreferencesDialog::OnKeywordEditingFinished()
{
}

//bool QmitkPreferencesDialog::eventFilter( QObject *obj, QEvent *event )
//{
//  if(obj == d->m_Keyword)
//  {
//    if(event->type() == QEvent::FocusIn && d->m_Keyword->text() == "search ...")
//    {
//      d->m_Keyword->setText("");
//      d->m_Keyword->setStyleSheet("color: black;");
//    }
//    else if(event->type() == QEvent::FocusOut && d->m_Keyword->text() == "")
//    {
//      d->m_Keyword->setText("search ...");
//      d->m_Keyword->setStyleSheet("color: gray;");
//    }
//  }
//  return true;
//}

void QmitkPreferencesDialog::OnPreferencesTreeItemSelectionChanged()
{
  if(d->m_PreferencesTree == 0)
    return;

  // TODO: create page and show it
  QList<QTreeWidgetItem *> selectedItems = d->m_PreferencesTree->selectedItems();
  if(selectedItems.size()>0)
  {

    d->m_CurrentPage = 0;
    for(vector<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it, ++d->m_CurrentPage)
    {
      if(it->treeWidgetItem == selectedItems.at(0))
      {
        d->m_Headline->setText(QString::fromStdString(it->name));
        if(it->prefPage == 0)
        {
          berry::IPreferencePage* page = it->confElem->CreateExecutableExtension<berry::IPreferencePage>("class");
          if (page == 0)
          {
            // support legacy BlueBerry extensions
            page = it->confElem->CreateExecutableExtension<berry::IPreferencePage>("class", berry::IPreferencePage::GetManifestName());
          }
          it->prefPage = dynamic_cast<berry::IQtPreferencePage*>(page);
          it->prefPage->CreateQtControl(d->m_PreferencesPanel);
          d->m_PreferencesPanel->addWidget(it->prefPage->GetQtControl());
        }
        d->m_PreferencesPanel->setCurrentWidget(it->prefPage->GetQtControl());
        
        break;
      }
    }
  }
}

void QmitkPreferencesDialog::UpdateTree()
{
  if(d->m_PreferencesTree == 0)
    return;

  //m_PreferencesTree->clear();
  string keyword = d->m_Keyword->text().toLower().toStdString();

  map<std::string, QTreeWidgetItem*> items;

  for(vector<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin();
      it != d->m_PrefPages.end(); ++it)
  {
    if(it->treeWidgetItem == 0)
    {

      if(it->category.empty())
      {
        it->treeWidgetItem = new QTreeWidgetItem(d->m_PreferencesTree);
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

  if(d->m_PrefPages.size()>0)
  {
    if(d->m_PrefPages.front().treeWidgetItem != 0)
      d->m_PrefPages.front().treeWidgetItem->setSelected(true);
  }

}
