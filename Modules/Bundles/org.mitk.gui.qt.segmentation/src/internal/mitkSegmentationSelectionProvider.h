/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef MITKSEGMENTATIONSELECTIONPROVIDER_H_
#define MITKSEGMENTATIONSELECTIONPROVIDER_H_

//#include "berryUiQtDll.h"

#include <berryISelection.h>
#include <berryISelectionProvider.h>
#include <berryISelectionChangedListener.h>
#include <berryISelectionListener.h>
#include <mitkDataTreeNodeSelection.h>

/*
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QObject>
*/

namespace mitk
{

class SegmentationSelectionProvider: public berry::ISelectionProvider
{
  

public:

  berryObjectMacro(SegmentationSelectionProvider)

  SegmentationSelectionProvider();

  void AddSelectionChangedListener(berry::ISelectionChangedListener::Pointer listener);

  void RemoveSelectionChangedListener(berry::ISelectionChangedListener::Pointer listener);

  berry::ISelection::ConstPointer GetSelection() const;
  
  void SetSelection(berry::ISelection::Pointer selection);
  
  void FireSelectionChanged(mitk::DataTreeNodeSelection::Pointer selection);
  void FireSelectionChanged(mitk::DataTreeNode::Pointer selectedNode);

  //QItemSelection GetQItemSelection() const;
  
  //void SetQItemSelection(const QItemSelection& selection);

  //QItemSelectionModel* GetItemSelectionModel() const;
  
  //void SetItemSelectionModel(QItemSelectionModel* combo);

  protected:

   DataTreeNodeSelection::Pointer m_CurrentSelection;
   berry::ISelectionChangedListener::Events m_RegisteredListeners;
};

}

#endif /* MITKSEGMENTATIONSELECTIONPROVIDER_H_ */
