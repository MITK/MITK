/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYSHOWVIEWMENU_H
#define BERRYSHOWVIEWMENU_H

#include <berryContributionItem.h>

#include <QPair>

namespace berry {

struct IWorkbenchPage;
struct IWorkbenchWindow;
struct IMenuManager;

class CommandContributionItemParameter;

/**
 * A <code>ShowViewMenu</code> is used to populate a menu manager with Show
 * View actions. The visible views are determined by user preference from the
 * Perspective Customize dialog.
 */
class ShowViewMenu : public QObject, public ContributionItem
{
  Q_OBJECT

public:

  /**
   * Creates a Show View menu.
   *
   * @param window
   *            the window containing the menu
   * @param id
   *            the id
   */
  ShowViewMenu(IWorkbenchWindow* window, const QString& id);

  bool IsDirty() const;

  /**
   * Overridden to always return true and force dynamic menu building.
   */
  bool IsDynamic() const;

  using ContributionItem::Fill;

  void Fill(QMenu* menu, QAction *before);

private:

  bool dirty;

  IWorkbenchWindow* window;

  static const QString NO_TARGETS_MSG;

  /**
   * Fills the menu with Show View actions.
   */
  void FillMenu(IMenuManager* innerMgr);

  QSet<QPair<QString, QString> > GetShortcuts(IWorkbenchPage* page) const;

  IContributionItem::Pointer showDlgItem;

//  bool makeFast;

  Q_SLOT void AboutToShow(IMenuManager* manager);

  SmartPointer<CommandContributionItemParameter> GetItem(const QString& viewId, const QString& secondaryId) const;

  QSet<QPair<QString,QString> > AddOpenedViews(IWorkbenchPage* page, QSet<QPair<QString,QString> >& actions) const;

  QSet<QPair<QString,QString> > GetParts(IWorkbenchPage* page) const;


  /**
   * @param commandService
   * @param makeFast
   */
//  ParameterizedCommand GetCommand(ICommandService* commandService,
//                                  bool makeFast)
//  {
//    Command c = commandService.getCommand(IWorkbenchCommandConstants.VIEWS_SHOW_VIEW);
//    Parameterization[] parms = null;
//    if (makeFast) {
//      try {
//        IParameter parmDef = c
//            .getParameter(IWorkbenchCommandConstants.VIEWS_SHOW_VIEW_PARM_FASTVIEW);
//        parms = new Parameterization[] { new Parameterization(parmDef,
//            "true") //$NON-NLS-1$
//        };
//      } catch (NotDefinedException e) {
//        // this should never happen
//      }
//    }
//    return new ParameterizedCommand(c, parms);
//  }
};

}

#endif // BERRYSHOWVIEWMENU_H
