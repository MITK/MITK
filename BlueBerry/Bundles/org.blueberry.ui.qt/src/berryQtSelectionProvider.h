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

#ifndef BERRYQTSELECTIONPROVIDER_H_
#define BERRYQTSELECTIONPROVIDER_H_

#include <org_blueberry_ui_qt_Export.h>

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

  berryObjectMacro(QtSelectionProvider)

  QtSelectionProvider();

  void AddSelectionChangedListener(ISelectionChangedListener::Pointer listener);

  void RemoveSelectionChangedListener(
      ISelectionChangedListener::Pointer listener);

  ISelection::ConstPointer GetSelection() const;
  void SetSelection(ISelection::ConstPointer selection);

  virtual void SetSelection(ISelection::ConstPointer selection, QItemSelectionModel::SelectionFlags);

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
