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

#include <deque>
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
    PrefPage(QString _id, QString _name, QString _category
      , QString _className, QString _keywords, berry::IConfigurationElement::Pointer _confElem)
      : id(_id), name(_name), category(_category), className(_className), keywords(_keywords),
        prefPage(nullptr), confElem(_confElem), treeWidgetItem(nullptr)
    {}

    bool operator==(const PrefPage& other) const
    { return id == other.id; }

    bool operator<(const PrefPage& other) const
    {
      bool res = (name < other.name);
      return res;
    }

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
        PrefPage page(id, name, category, className, keywordLabels, berry::IConfigurationElement::Pointer(*prefPagesIt));
        if (category.isEmpty())
        {
          m_PrefPages.push_front(page);
        }
        else
        {
          m_PrefPages.push_back(page);
        }
      }
    }
  }

  ///
  /// Saves all treewidgetitems in a map, the key is the id of the preferencepage.
  ///
  std::deque<PrefPage> m_PrefPages;
  int m_CurrentPage;

};

QmitkPreferencesDialog::QmitkPreferencesDialog(QWidget * parent, Qt::WindowFlags f)
  : QDialog(parent, f), d(new QmitkPreferencesDialogPrivate)
{
  d->setupUi(this);

  QPalette currentPalette = palette();
  d->m_Headline->setStyleSheet(QString(
    "background-color: [background-color]; "
    "border-style: solid; "
    "border-width: 1px; "
    "border-color: [border-color]; "
    "color: [text-color]; "
  ).replace("[background-color]", currentPalette.color(QPalette::Mid).name())
   .replace("[border-color]", currentPalette.color(QPalette::Light).name())
   .replace("[text-color]", currentPalette.color(QPalette::BrightText).name())
  );

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

void QmitkPreferencesDialog::SetSelectedPage(const QString& id)
{
  for(std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
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

  for(std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it)
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
  QString currentText = d->m_Keyword->text();
  currentText = currentText.toUpper();
  
  for (unsigned int i = 0; i < d->m_PrefPages.size(); ++i)
  {
    QString text = d->m_PrefPages[i].name;
    text = text.toUpper();
    
    if (d->m_PrefPages[i].treeWidgetItem == nullptr)
    {
      continue;
    }
    
    if (currentText.isEmpty())
    {
      d->m_PrefPages[i].treeWidgetItem->setHidden(false);
      continue;
    }
    
    if (text.contains(currentText))
    {
      d->m_PrefPages[i].treeWidgetItem->setHidden(false);
    }
    else
    {
      d->m_PrefPages[i].treeWidgetItem->setHidden(true);
    }
  }
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
    for(std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator it = d->m_PrefPages.begin(); it != d->m_PrefPages.end(); ++it, ++d->m_CurrentPage)
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
  {
    return;
  }

  std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator endRoot = d->m_PrefPages.end();
  
  std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator sortIter = d->m_PrefPages.begin();
  for (; sortIter != d->m_PrefPages.end(); ++sortIter)
  {
    if (sortIter->category.isEmpty())
    {
      endRoot = sortIter;
    }
  }
  
  if (endRoot != d->m_PrefPages.end())
  {
    std::sort(d->m_PrefPages.begin(), endRoot + 1);
  }
  
  std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator iter = d->m_PrefPages.begin();
  for (; iter != d->m_PrefPages.end(); ++iter)
  {
    if (iter->id == "org.mitk.gui.qt.application.EnvironmentPreferencePage")
    {
      std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator iter2 = d->m_PrefPages.begin();
      std::iter_swap(iter, iter2);
    }
    else if (iter->id == "org.mitk.gui.qt.application.PacsPreferencePage")
    {
      std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator iter2 = d->m_PrefPages.begin() + 2;
      std::iter_swap(iter, iter2);
    }
    else if (iter->id == "org.mitk.GeneralPreferencePage")
    {
      std::deque<QmitkPreferencesDialogPrivate::PrefPage>::iterator iter2 = d->m_PrefPages.begin() + 1;
      std::iter_swap(iter, iter2);      
    }
  }
  
  for (unsigned int i = 0; i < d->m_PrefPages.size(); ++i)
  {
    d->m_PrefPages[i].treeWidgetItem = new QTreeWidgetItem();
    d->m_PrefPages[i].treeWidgetItem->setText(0, d->m_PrefPages[i].name);
    
    if (d->m_PrefPages[i].category.isEmpty())
    {
      d->m_PreferencesTree->addTopLevelItem(d->m_PrefPages[i].treeWidgetItem);
    }
    else
    {
      for (unsigned int j = 0; j < d->m_PrefPages.size(); ++j)
      {
        if (d->m_PrefPages[i].category == d->m_PrefPages[j].id)
        { 
          d->m_PrefPages[j].treeWidgetItem->addChild(d->m_PrefPages[i].treeWidgetItem);
          
          break;
        }
      }
    }
  }
}
