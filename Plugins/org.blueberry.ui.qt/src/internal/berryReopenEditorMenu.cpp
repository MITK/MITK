/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryReopenEditorMenu.h"

#include <berryIPreferences.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>

#include <berryMenuManager.h>
#include <berrySafeRunner.h>

#include "berryEditorHistory.h"
#include "berryEditorHistoryItem.h"
#include "berryWorkbench.h"
#include "berryWorkbenchPlugin.h"
#include "berryPreferenceConstants.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>

namespace berry {

ReopenEditorMenu::ReopenEditorMenu(IWorkbenchWindow* window, const QString& id, bool showSeparator)
  : ContributionItem(id)
  , window(window)
  , history(nullptr)
  , showSeparator(showSeparator)
  , dirty(true)
{
  IWorkbench* workbench = window->GetWorkbench();
  if (Workbench* w = dynamic_cast<Workbench*>(workbench))
  {
    history = w->GetEditorHistory();
  }
}

QString ReopenEditorMenu::CalcText(int index, const QString& name, const QString& toolTip, bool rtl)
{
  QString sb;

  int mnemonic = index + 1;
  QString nm = QString::number(mnemonic);
  if (mnemonic <= MAX_MNEMONIC_SIZE)
  {
    nm.prepend('&');
  }

  QString fileName = name;
  QString pathName = toolTip;
  if (pathName == fileName)
  {
    // tool tip text isn't necessarily a path;
    // sometimes it's the same as name, so it shouldn't be treated as a path then
    pathName.clear();
  }
  QFileInfo path(pathName);
  // if last segment in path is the fileName, remove it
  if (path.fileName() == fileName)
  {
    path.setFile(path.path());
    pathName = path.absoluteFilePath();
  }

  if ((fileName.size() + pathName.size()) <= (MAX_TEXT_LENGTH - 4))
  {
    // entire item name fits within maximum length
    sb += fileName;
    if (!pathName.isEmpty())
    {
      sb += "  [" + pathName + "]";
    }
  }
  else
  {
    // need to shorten the item name
    int length = fileName.size();
    if (length > MAX_TEXT_LENGTH)
    {
      // file name does not fit within length, truncate it
      sb += fileName.left(MAX_TEXT_LENGTH - 3);
      sb += "...";
    }
    else if (length > MAX_TEXT_LENGTH - 7)
    {
      sb += fileName;
    }
    else
    {
      sb += fileName;
      QStringList pathSegments = path.absoluteFilePath().split('/', QString::SkipEmptyParts);
      int segmentCount = pathSegments.size();
      if (segmentCount > 0)
      {
        length += 7; // 7 chars are taken for "  [...]"

        sb += "  [";

        // Add first n segments that fit
        int i = 0;
        while (i < segmentCount && length < MAX_TEXT_LENGTH)
        {
          const QString& segment = pathSegments[i];
          if (length + segment.size() < MAX_TEXT_LENGTH)
          {
            sb += segment + QDir::separator();
            length += segment.size() + 1;
            i++;
          }
          else if (i == 0)
          {
            // append at least part of the first segment
            sb += segment.left(MAX_TEXT_LENGTH - length);
            length = MAX_TEXT_LENGTH;
            break;
          }
          else
          {
            break;
          }
        }

        sb += "...";

        i = segmentCount - 1;
        // Add last n segments that fit
        while (i > 0 && length < MAX_TEXT_LENGTH)
        {
          const QString& segment = pathSegments[i];
          if (length + segment.size() < MAX_TEXT_LENGTH)
          {
            sb += QDir::separator();
            sb += segment;
            length += segment.size() + 1;
            i--;
          }
          else
          {
            break;
          }
        }

        sb.append("]");
      }
    }
  }
  QString process;
  if (rtl)
  {
    process = sb + " " + nm;
  }
  else
  {
    process = nm + " " + sb;
  }
  //return TextProcessor.process(process, TextProcessor.getDefaultDelimiters() + "[]");
  return process;
}

void ReopenEditorMenu::Fill(QMenu* menu, QAction* before)
{
  if (window->GetActivePage() == nullptr
      || window->GetActivePage()->GetPerspective().IsNull())
  {
    return;
  }

  if (MenuManager* mm = dynamic_cast<MenuManager*>(this->GetParent()))
  {
    mm->connect(mm, SIGNAL(AboutToShow(IMenuManager*)), this, SLOT(MenuAboutToShow(IMenuManager*)));
  }

  int itemsToShow = WorkbenchPlugin::GetDefault()->GetPreferences()->GetInt(PreferenceConstants::RECENT_FILES, 6);
  if (itemsToShow == 0 || history == nullptr)
  {
    return;
  }

  // Get items.
  QList<EditorHistoryItem::Pointer> historyItems = history->GetItems();

  int n = std::min(itemsToShow, historyItems.size());
  if (n <= 0)
  {
    return;
  }

  if (showSeparator)
  {
    menu->addSeparator();
  }

  struct _SafeRunnable : public ISafeRunnable
  {
    QMenu* menu;
    QAction* before;
    EditorHistoryItem::Pointer item;
    const int historyIndex;

    _SafeRunnable(QMenu* menu, QAction* before, EditorHistoryItem::Pointer item, int index)
      : menu(menu), before(before), item(item), historyIndex(index) {}

    void Run() override
    {
      QString text = ReopenEditorMenu::CalcText(historyIndex, item);
      auto   mi = new QAction(text, nullptr);
      menu->insertAction(before, mi);
      mi->setData(QVariant::fromValue(item));
    }

    void HandleException(const ctkException& e) override
    {
      // just skip the item if there's an error,
      // e.g. in the calculation of the shortened name
      WorkbenchPlugin::Log(this->GetClassName(), "Fill", e);
    }
  };

  for (int i = 0; i < n; i++)
  {
    EditorHistoryItem::Pointer item = historyItems[i];
    ISafeRunnable::Pointer runnable(new _SafeRunnable(menu, before, item, i));
    SafeRunner::Run(runnable);
  }
  dirty = false;
}

bool ReopenEditorMenu::IsDirty() const
{
  return dirty;
}

bool ReopenEditorMenu::IsDynamic() const
{
  return true;
}

void ReopenEditorMenu::Open(const EditorHistoryItem::Pointer& item)
{
  IWorkbenchPage::Pointer page = window->GetActivePage();
  if (page.IsNotNull())
  {
    try
    {
      QString itemName = item->GetName();
      if (!item->IsRestored())
      {
        item->RestoreState();
      }
      IEditorInput::Pointer input = item->GetInput();
      IEditorDescriptor::Pointer desc = item->GetDescriptor();
      if (!input || !desc)
      {
        QString title = "Problems opening editor";
        QString msg = QString("Unable to open %1.").arg(itemName);
        QMessageBox::warning(window->GetShell()->GetControl(), title, msg);
        history->Remove(item);
      }
      else
      {
        page->OpenEditor(input, desc->GetId());
      }
    }
    catch (const PartInitException& e2)
    {
      QString title = "Problems opening editor";
      QMessageBox::warning(window->GetShell()->GetControl(), title, e2.what());
      history->Remove(item);
    }
  }
}

QString ReopenEditorMenu::CalcText(int index, const EditorHistoryItem::Pointer& item)
{
  // IMPORTANT: avoid accessing the item's input since
  // this can require activating plugins.
  // Instead, ask the item for the info, which can
  // consult its memento if it is not restored yet.
  return CalcText(index, item->GetName(), item->GetToolTipText(), false);
                  // Window::GetDefaultOrientation() == SWT.RIGHT_TO_LEFT);
}

void ReopenEditorMenu::MenuAboutToShow(IMenuManager* manager)
{
  manager->MarkDirty();
  dirty = true;
}

}
