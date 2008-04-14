/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkPolygonInteractor.h"
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>
#include <mitkLineOperation.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>


mitk::PolygonInteractor::PolygonInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode)
{
  m_PrimStripInteractor = mitk::PrimStripInteractor::New("primstripinteractor",NULL);
  this->AddInteractor((Interactor::Pointer)m_PrimStripInteractor);
}

mitk::PolygonInteractor::~PolygonInteractor()
{
  //delete m_PrimStripInteractor;
}

void mitk::PolygonInteractor::DeselectAllCells()  
{
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return;

  //deselect cells
  mitk::Mesh::DataType *itkMesh = mesh->GetMesh(); 
  mitk::Mesh::CellDataIterator cellDataIt, cellDataEnd;
  cellDataEnd = itkMesh->GetCellData()->End();
  for (cellDataIt = itkMesh->GetCellData()->Begin(); cellDataIt != cellDataEnd; cellDataIt++)
  {
    if ( cellDataIt->Value().selected )
    {
      mitk::LineOperation* doOp = new mitk::LineOperation(OpDESELECTCELL, cellDataIt->Index());
      if (m_UndoEnabled)
      {
        mitk::LineOperation* undoOp = new mitk::LineOperation(OpSELECTCELL, cellDataIt->Index());
        OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      mesh->ExecuteOperation(doOp);
    }
  }
}

float mitk::PolygonInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0;

  float lowerJurisdiction = this->CalculateLowerJurisdiction(stateEvent);
  
  if (returnValue < lowerJurisdiction)
    returnValue = lowerJurisdiction;

  return returnValue;
}


bool mitk::PolygonInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool
  
  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return false;

  switch (action->GetActionId())
  {
  case AcINITNEWOBJECT:
    //Set a Property so that a BoundingBox is drawn arround the Polygon if selected
    {
      mitk::BoolProperty::Pointer showBoundingBox;
      showBoundingBox = mitk::BoolProperty::New(true);
      m_DataTreeNode->GetPropertyList()->SetProperty("showBoundingBox", showBoundingBox);
      ok = true;
      break;
    }
  case AcFINISHOBJECT:
    //finish the creation of the polygon
    {
      int cellId = mesh->SearchSelectedCell();
      LineOperation* lineDoOp = new mitk::LineOperation(OpCLOSECELL, cellId);
      if (m_UndoEnabled)
      {
        LineOperation* lineUndoOp = new mitk::LineOperation(OpOPENCELL, cellId);
        OperationEvent *operationEvent = new OperationEvent(mesh, lineDoOp, lineUndoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      mesh->ExecuteOperation(lineDoOp );
    }
    ok = true;
    break;
  default:
    return Superclass::ExecuteAction(action, stateEvent);
  }
  return ok;
}
