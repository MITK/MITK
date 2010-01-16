/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYQTSELECTIONPROVIDER_H_
#define BERRYQTSELECTIONPROVIDER_H_

#include "berryUiQtDll.h"

#include <berryISelectionProvider.h>

#include <QItemSelectionModel>
#include <QItemSelection>
#include <QObject>

namespace berry
{

class BERRY_UI_QT QtSelectionProvider: public QObject,
    public ISelectionProvider
{
  Q_OBJECT

public:

  osgiObjectMacro(QtSelectionProvider)

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

#endif /* BERRYQTSELECTIONPROVIDER_H_ */
