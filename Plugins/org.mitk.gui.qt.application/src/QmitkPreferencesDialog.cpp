/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPreferencesDialog.h"

#include "berryPlatform.h"
#include "berryPlatformUI.h"
#include "berryIWorkbench.h"
#include "berryIConfigurationElement.h"
#include "berryIExtensionRegistry.h"
#include "berryIExtension.h"
#include <berryIBerryPreferencesService.h>
#include <berryIQtPreferencePage.h>

#include "internal/org_mitk_gui_qt_application_Activator.h"

#include <ui_QmitkPreferencesDialog.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

#include <algorithm>

#include <mitkLogMacros.h>

using namespace std;

static std::vector<std::string> splitString(const std::string &s, char delim=' ')
{
  std::vector < std::string > elems;
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim))
  {
    elems.push_back(item);
  }
  return elems;
}

class QmitkPreferencesDialogPrivate : public Ui::QmitkPreferencesDialog
{
public:

  ///
  /// Just a stub class for holding information on prefpages (metadata)
  ///
  struct PrefPage
  {
    PrefPage(QString _id, QString _name, QString _category
             , QString _className, QString _keywords, berry::IConfigurationElement::Pointer _confElem)
      : id(_id), name(_name), category(_category), className(_className), keywords(_keywords),
        prefPage(nullptr), confElem(_confElem), treeWidgetItem(nullptr)
    {}

    bool operator==(const PrefPage& other)
    { return id == other.id; }

    bool operator<(const PrefPage& other)
    { return name < other.name; }

    QString id;
    QString name;
    QString category;
    QString className;
    QString keywords;
    berry::IQtPreferencePage* prefPage;
    berry::IConfigurationElement::Pointer confElem;
    QTreeWidgetItem* treeWidgetItem;
  };

  QmitkPreferencesDialogPrivate()
    : m_CurrentPage(0)
  {
    berry::IExtensionRegistry* xpService = berry::Platform::GetExtensionRegistry();

    // m_PrefPages
    QList<berry::IConfigurationElement::Pointer> prefPages(xpService->GetConfigurationElementsFor("org.blueberry.ui.preferencePages"));
    QList<berry::IConfigurationElement::Pointer> keywordExts(xpService->GetConfigurationElementsFor("org.blueberry.ui.keywords"));
    QList<berry::IConfigurationElement::Pointer>::iterator prefPagesIt;

    QList<berry::IConfigurationElement::Pointer>::iterator keywordRefsIt;

    for (prefPagesIt = prefPages.begin(); prefPagesIt != prefPages.end(); ++prefPagesIt)
    {
      QString id = (*prefPagesIt)->GetAttribute("id");
      QString name = (*prefPagesIt)->GetAttribute("name");
      QString className = (*prefPagesIt)->GetAttribute("class");
      if(!id.isEmpty() && !name.isEmpty() && !className.isEmpty())
      {
        QString keywordLabels;

        QString category = (*prefPagesIt)->GetAttribute("category");
        //# collect keywords
        QList<berry::IConfigurationElement::Pointer> keywordRefs = (*prefPagesIt)->GetChildren("keywordreference"); // get all keyword references
        for (keywordRefsIt = keywordRefs.begin()
             ; keywordRefsIt != keywordRefs.end(); ++keywordRefsIt) // iterate over all refs
        {
          QString keywordRefId = (*keywordRefsIt)->GetAttribute("id"); // get referenced id

          for (QList<berry::IConfigurationElement::Pointer>::iterator keywordExtsIt = keywordExts.begin();
               keywordExtsIt != keywordExts.end(); ++keywordExtsIt) // iterate over all keywords
          {
            QString keywordId = (*keywordExtsIt)->GetAttribute("id"); // get keyword id
            if(keywordId == keywordRefId) // if referenced id is equals the current keyword id
            {
              //# collect all keywords from label attribute with a tokenizer
              QString currLabel = (*keywordExtsIt)->GetAttribute("label");
              currLabel = currLabel.toLower();
              if (!currLabel.isEmpty()) keywordLabels += QString(" ") + currLabel;

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
  /// Saves all treewidgetitems in a map, the key is the id of the preferencepage.
  ///
  QList<PrefPage> m_PrefPages;
  int m_CurrentPage;

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

  this->UpdateTree();
}

QmitkPreferencesDialog::~QmitkPreferencesDialog()
{
}

void QmitkPreferencesDialog::SetSelectedPage(const QString& id)
{
  for(QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
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
    berry::IBerryPreferencesService* berryPrefService =
        dynamic_cast<berry::IBerryPreferencesService*>(berry::Platform::GetPreferencesService());
    if(berryPrefService != nullptr)
    {
      static QString importDir = "";
      QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file to import preferences"),
                                                      importDir, tr("XML files (*.xml)"));

      if(!fileName.isEmpty())
      {
        importDir = QFileInfo(fileName).absoluteDir().path();
        berryPrefService->ImportPreferences(fileName, "");
        berry::IQtPreferencePage* prefPage = d->m_PrefPages[d->m_CurrentPage].prefPage;
        if(prefPage)
          prefPage->Update();

        MITK_INFO("QmitkPreferencesDialog") << "Preferences successfully imported from " << fileName;
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
    berry::IBerryPreferencesService* berryPrefService =
        dynamic_cast<berry::IBerryPreferencesService*>(berry::Platform::GetPreferencesService());
    if(berryPrefService != nullptr)
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
        berryPrefService->ExportPreferences(fileName, "");
        MITK_INFO("QmitkPreferencesDialog") << "Preferences successfully exported to " << fileName;
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
  berry::IQtPreferencePage* prefPage = nullptr;

  for(QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
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
  berry::Platform::GetPreferencesService()->GetSystemPreferences()->Flush();
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
  if(d->m_PreferencesTree == nullptr)
    return;

  // TODO: create page and show it
  QList<QTreeWidgetItem *> selectedItems = d->m_PreferencesTree->selectedItems();
  if(selectedItems.size()>0)
  {

    d->m_CurrentPage = 0;
    berry::IWorkbench* workbench = berry::PlatformUI::GetWorkbench();
    for(QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it, ++d->m_CurrentPage)
    {
      if(it->treeWidgetItem == selectedItems.at(0))
      {
        d->m_Headline->setText(it->name);
        if(it->prefPage == nullptr)
        {
          berry::IPreferencePage* page = it->confElem->CreateExecutableExtension<berry::IPreferencePage>("class");
          it->prefPage = dynamic_cast<berry::IQtPreferencePage*>(page);
          it->prefPage->Init(berry::IWorkbench::Pointer(workbench));
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
  if(d->m_PreferencesTree == nullptr)
    return;

  d->m_PreferencesTree->sortByColumn(0, Qt::AscendingOrder);

  //m_PreferencesTree->clear();
  QString keyword = d->m_Keyword->text().toLower();

  map<QString, QTreeWidgetItem*> items;
  std::list< QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator > deferredItems;

  for (QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin();
       it != d->m_PrefPages.end(); ++it)
  {
    if (it->treeWidgetItem == nullptr)
    {
      if (it->category.isEmpty())
      {
        it->treeWidgetItem = new QTreeWidgetItem(d->m_PreferencesTree);
        it->treeWidgetItem->setText(0, it->name);
        items[it->id] = it->treeWidgetItem;
      }
      else
      {
        // the parent might not be created yet
        deferredItems.push_back(it);
      }
    }
  }

  // deal with deferred items. We do not know how many levels
  // of parents need to be created
  auto currentItem = deferredItems.begin();

  while (currentItem != deferredItems.end())
  {
    auto currentItemContent = *currentItem;
    if (items[currentItemContent->category] != nullptr)
    {
      currentItemContent->treeWidgetItem = new QTreeWidgetItem(items[currentItemContent->category]);
      currentItemContent->treeWidgetItem->setText(0, currentItemContent->name);
      items[currentItemContent->id] = currentItemContent->treeWidgetItem;

      deferredItems.erase(currentItem);
      currentItem = deferredItems.begin();
    }
    else
    {
      ++currentItem;
    }
  }

  if (!deferredItems.empty())
  {
    MITK_ERROR << "Unknown preference category. They are added top-level.";

    for (const auto &currentItemContent : deferredItems)
    {
      currentItemContent->treeWidgetItem = new QTreeWidgetItem(d->m_PreferencesTree);
      currentItemContent->treeWidgetItem->setText(0, currentItemContent->name);
      items[currentItemContent->id] = currentItemContent->treeWidgetItem;
    }
  }

  //First, set all pages to hidden to avoid prolems
  //when iterating over the child page first which contains a specific keyword
  //and sets the parent to visible
  //but after this the parent is searched for the keyword. However, the keyword might not be available
  //so the parent and all of its children will be set to hidden.
  if(!keyword.isEmpty())
  {
    for (QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin();
         it != d->m_PrefPages.end(); ++it)
    {
      it->treeWidgetItem->setHidden(true);
    }
  }
  // we have to iterate over the list a second time, as only
  // now all parents and items are guaranteed to be created
  for (QList<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin();
       it != d->m_PrefPages.end(); ++it)
  {
    // hide treeWidgetItem if keyword not matches
    if(!keyword.isEmpty())
    {
      //Split text of search box into multiple single strings
      vector<string> keywordsAvailable = splitString(keyword.toUtf8().constData());
      bool foundAll = true;

      //perform an AND-search
      for(unsigned int i = 0; i < keywordsAvailable.size(); i++)
      {
        if (it->keywords.indexOf(QString::fromStdString(keywordsAvailable[i])) == -1)
        {
          foundAll = false;
          break;
        }
      }

      if(foundAll)
      {
        //#make the whole branch visible
        QTreeWidgetItem* treeWidgetParent = it->treeWidgetItem->parent();
        while(treeWidgetParent!=nullptr)
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
    if(d->m_PrefPages.front().treeWidgetItem != nullptr)
      d->m_PrefPages.front().treeWidgetItem->setSelected(true);
  }

  d->m_PreferencesTree->expandAll();
}
