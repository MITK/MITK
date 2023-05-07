/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAlgorithmInfoSelectionProvider_h
#define mitkAlgorithmInfoSelectionProvider_h

#include <org_mitk_gui_qt_matchpoint_algorithm_browser_Export.h>

#include <berryISelectionProvider.h>

#include <QObject>

#include "mitkMAPAlgorithmInfoSelection.h"

namespace mitk
{

class AlgorithmInfoSelectionProvider: public QObject,
  public berry::ISelectionProvider
{
  Q_OBJECT

public:

  berryObjectMacro(AlgorithmInfoSelectionProvider);

  AlgorithmInfoSelectionProvider();

  void AddSelectionChangedListener(berry::ISelectionChangedListener* listener) override;

  void RemoveSelectionChangedListener( berry::ISelectionChangedListener* listener) override;

  berry::ISelection::ConstPointer GetSelection() const override;
  void SetSelection(const berry::ISelection::ConstPointer& selection) override;

  MAPAlgorithmInfoSelection::ConstPointer GetInfoSelection() const;
  void SetInfoSelection(MAPAlgorithmInfoSelection::ConstPointer selection);

protected:
  berry::ISelectionChangedListener::Events _selectionEvents;
  MAPAlgorithmInfoSelection::ConstPointer _selection;
};

}

#endif
