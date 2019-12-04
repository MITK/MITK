/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(QtSelectionProvider);

  QtSelectionProvider();

  void AddSelectionChangedListener(ISelectionChangedListener* listener) override;

  void RemoveSelectionChangedListener(ISelectionChangedListener* listener) override;

  ISelection::ConstPointer GetSelection() const override;
  void SetSelection(const ISelection::ConstPointer& selection) override;

  virtual void SetSelection(const ISelection::ConstPointer& selection,
                            QItemSelectionModel::SelectionFlags);

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
