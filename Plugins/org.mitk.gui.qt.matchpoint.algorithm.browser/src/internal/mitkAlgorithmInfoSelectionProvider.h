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

#ifndef _MITK_ALGORITHM_SELECTION_PROVIDER_H
#define _MITK_ALGORITHM_SELECTION_PROVIDER_H

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

  berryObjectMacro(AlgorithmInfoSelectionProvider)

  AlgorithmInfoSelectionProvider();

  void AddSelectionChangedListener(berry::ISelectionChangedListener* listener);

  void RemoveSelectionChangedListener( berry::ISelectionChangedListener* listener);

  berry::ISelection::ConstPointer GetSelection() const;
  void SetSelection(const berry::ISelection::ConstPointer& selection);

  MAPAlgorithmInfoSelection::ConstPointer GetInfoSelection() const;
  void SetInfoSelection(MAPAlgorithmInfoSelection::ConstPointer selection);

protected:
  berry::ISelectionChangedListener::Events _selectionEvents;
  MAPAlgorithmInfoSelection::ConstPointer _selection;
};

}

#endif /* BERRYQTSELECTIONPROVIDER_H_ */
