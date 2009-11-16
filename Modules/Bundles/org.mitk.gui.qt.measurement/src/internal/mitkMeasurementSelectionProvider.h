/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-05-26 13:47:06 +0200 (Di, 26 Mai 2009) $
 Version:   $Revision: 17408 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef MITKMeasurementSelectionProvider_H_
#define MITKMeasurementSelectionProvider_H_

//#include "cherryUiQtDll.h"

#include <cherryISelection.h>
#include <cherryISelectionProvider.h>
#include <cherryISelectionChangedListener.h>
#include <cherryISelectionListener.h>
#include <mitkDataTreeNodeSelection.h>

/*
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QObject>
*/
namespace mitk
{

class MeasurementSelectionProvider: public cherry::ISelectionProvider
{
  

public:

  cherryObjectMacro(MeasurementSelectionProvider)

  MeasurementSelectionProvider();

  void AddSelectionChangedListener(cherry::ISelectionChangedListener::Pointer listener);

  void RemoveSelectionChangedListener(cherry::ISelectionChangedListener::Pointer listener);

  cherry::ISelection::ConstPointer GetSelection() const;
  
  void SetSelection(cherry::ISelection::Pointer selection);

  void FireSelectionChanged(DataTreeNodeSelection::Pointer selection);
  
  void FireSelectionChanged(DataTreeNode::Pointer selectedNode);

  protected:
    DataTreeNodeSelection::Pointer m_CurrentSelection;
    cherry::ISelectionChangedListener::Events m_RegisteredListeners;
  

};

}

#endif /* MITKMeasurementSelectionProvider_H_ */
