/*=========================================================================

 Program:   openCherry Platform
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

#ifndef CHERRYQTSELECTIONPROVIDER_H_
#define CHERRYQTSELECTIONPROVIDER_H_

#include "cherryUiQtDll.h"

#include <cherryISelectionProvider.h>

#include <QItemSelectionModel>
#include <QItemSelection>
#include <QObject>

namespace cherry
{

class CHERRY_UI_QT QtSelectionProvider: public QObject,
    public ISelectionProvider
{
  Q_OBJECT

public:

  cherryObjectMacro(QtSelectionProvider)

  QtSelectionProvider();

  void AddSelectionChangedListener(ISelectionChangedListener::Pointer listener);

  void RemoveSelectionChangedListener(
      ISelectionChangedListener::Pointer listener);

  ISelection::ConstPointer GetSelection() const;
  void SetSelection(ISelection::Pointer selection);

  QItemSelection GetQItemSelection() const;
  void SetQItemSelection(const QItemSelection& selection);

  QItemSelectionModel* GetItemSelectionModel() const;
  void SetItemSelectionModel(QItemSelectionModel* combo);

protected:
  ISelectionChangedListener::Events selectionEvents;
  QItemSelectionModel* qSelectionModel;

protected slots:

virtual void FireSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

};

}

#endif /* CHERRYQTSELECTIONPROVIDER_H_ */
