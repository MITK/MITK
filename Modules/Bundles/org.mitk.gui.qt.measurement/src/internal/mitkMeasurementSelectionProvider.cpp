/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-11-02 20:38:30 +0100 (Mo, 02 Nov 2009) $
 Version:   $Revision: 19650 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "mitkMeasurementSelectionProvider.h"
#include <mitkDataTreeNodeSelection.h>
#include "cherrySelectionChangedEvent.h"

namespace mitk
{



MeasurementSelectionProvider::MeasurementSelectionProvider()
{
}


void MeasurementSelectionProvider::AddSelectionChangedListener(cherry::ISelectionChangedListener::Pointer listener)
{
  m_RegisteredListeners.AddListener(listener);
  //LOG_INFO << "jemand hat sich registriert!";
}

void MeasurementSelectionProvider::RemoveSelectionChangedListener(cherry::ISelectionChangedListener::Pointer listener)
{
  m_RegisteredListeners.RemoveListener(listener);
 
}

cherry::ISelection::ConstPointer MeasurementSelectionProvider::GetSelection() const
{
  return (cherry::ISelection::ConstPointer)0;
}

void MeasurementSelectionProvider::SetSelection(cherry::ISelection::Pointer selection)
{
  cherry::SelectionChangedEvent::Pointer event(new cherry::SelectionChangedEvent(cherry::ISelectionProvider::Pointer(this), selection));
  m_RegisteredListeners.selectionChanged(event);
}

}
