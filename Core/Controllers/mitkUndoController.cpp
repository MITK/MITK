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


#include "mitkUndoController.h"
#include "mitkLimitedLinearUndo.h"
#include "mitkVerboseLimitedLinearUndo.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"

//static member-variables init.
mitk::UndoModel::Pointer mitk::UndoController::m_CurUndoModel;
mitk::UndoController::UndoModelMap mitk::UndoController::m_UndoModelList;
mitk::UndoController::UndoType mitk::UndoController::m_CurUndoType;

//const mitk::UndoController::UndoType mitk::UndoController::DEFAULTUNDOMODEL = LIMITEDLINEARUNDO;
const mitk::UndoController::UndoType mitk::UndoController::DEFAULTUNDOMODEL = VERBOSE_LIMITEDLINEARUNDO;


mitk::UndoController::UndoController(UndoType undoType)
{
  if (SwitchUndoModel(undoType)==false) //existiert noch nicht in static-Liste
  {
    switch (undoType)
    {
    case LIMITEDLINEARUNDO:
      m_CurUndoModel = mitk::LimitedLinearUndo::New();
      m_CurUndoType = undoType;
      m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
      break;
    case VERBOSE_LIMITEDLINEARUNDO:
      m_CurUndoModel = mitk::VerboseLimitedLinearUndo::New();
      m_CurUndoType = undoType;
      m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
      break;
      //case ###
      //insert here, in add- and RemoveUndoModel new sets of UndoModels!
      //break;
    default :
      m_CurUndoModel = VerboseLimitedLinearUndo::New();
      m_CurUndoType = undoType;
      m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
    }
  }
}

bool mitk::UndoController::SetOperationEvent(UndoStackItem* operationEvent)
{
  m_CurUndoModel->SetOperationEvent(operationEvent);
  return true;
}

bool mitk::UndoController::Undo()
{
  return this->Undo(true);
}

bool mitk::UndoController::Undo(bool fine)
{
  bool ret = m_CurUndoModel->Undo(fine);
  
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  
  return ret;
}

bool mitk::UndoController::Redo()
{
  return this->Redo(true);
}

bool mitk::UndoController::Redo(bool fine)
{
  bool ret = m_CurUndoModel->Redo(fine);
  
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  
  return ret;
}

void mitk::UndoController::Clear()
{
  m_CurUndoModel->Clear();
}

void mitk::UndoController::ClearRedoList()
{
  m_CurUndoModel->ClearRedoList();
}

bool mitk::UndoController::RedoListEmpty()
{
  return m_CurUndoModel->RedoListEmpty();
}

//##Documentation
//##Switches the UndoModel to the given Type
//##if there is no equal Type in List, then return false
bool mitk::UndoController::SwitchUndoModel(UndoType undoType)
{
  if (m_CurUndoType == undoType)
  {
    return true;//already switched, don't need to be switched!
  }

  UndoModelMapIter undoModelIter = m_UndoModelList.find(undoType);
  if (undoModelIter == m_UndoModelList.end())
  {//undoType not found in List
    return false;
  }

  //found-> switch to UndoModel
  m_CurUndoModel = (undoModelIter)->second;
  m_CurUndoType = (undoModelIter)->first;
  return true;
}

//##Documentation
//##adds a new kind of UndoModel to the set of UndoModels
//##and switches to that UndoModel
//##if the UndoModel exists already in the List, then nothing is done
bool mitk::UndoController::AddUndoModel(UndoType undoType)
{
  if (m_UndoModelList.find(undoType) != m_UndoModelList.end())
  { //UndoModel already exists
    return false;
  }
  //doesn't already exist in list
  switch (undoType)
  {
  case LIMITEDLINEARUNDO:
    m_CurUndoModel = LimitedLinearUndo::New();
    m_CurUndoType = undoType;
    m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
    break;
  case VERBOSE_LIMITEDLINEARUNDO:
    m_CurUndoModel = VerboseLimitedLinearUndo::New();
    m_CurUndoType = undoType;
    m_UndoModelList.insert(UndoModelMap::value_type(undoType, m_CurUndoModel));
    break;
  default:
    //that undoType is not implemented!
    return false;
  }
  return true;
}

//##Documentation
//##Removes an UndoModel from the set of UndoModels
//##If that UndoModel is currently selected, then the DefaultUndoModel(const) is set. 
//##If the default is not in List, then the first UndoModel is set.
//##UndoList may not be empty, so if the UndoType is the last, then return false;
bool mitk::UndoController::RemoveUndoModel(UndoType undoType)
{
  if (m_UndoModelList.size() < 2)
  {//for no empty m_UndoModelList
    return false;
  }
  //try deleting Element
  int ok = m_UndoModelList.erase(undoType);
  if (ok == 0) 
  {//delete unsucessful; Element of undoType not found
    return false;
  }

  //if m_CurUndoModel is the one removed, then change it to default or to the next or first
  if (m_CurUndoType == undoType)
  {//we have to change m_CurUndoModel and m_CurUndoType to an existing Model

    //if defaultUndoModel exists, then set to default
    UndoModelMapIter undoModelIter = m_UndoModelList.find(DEFAULTUNDOMODEL);
    if (undoModelIter == m_UndoModelList.end())
    {//DefaultUndoModel does not exists in m_CurUndoModelList
      undoModelIter = m_UndoModelList.begin();
    }
    m_CurUndoModel = (undoModelIter)->second;
    m_CurUndoType = (undoModelIter)->first;
    return true;
  }
  //m_CurUndoType was not undoType and is not changed
  return true;
}

int mitk::UndoController::GetLastObjectEventIdInList()
{
  return m_CurUndoModel->GetLastObjectEventIdInList();
}

int mitk::UndoController::GetLastGroupEventIdInList()
{
  return m_CurUndoModel->GetLastGroupEventIdInList();
}


mitk::OperationEvent* mitk::UndoController::GetLastOfType(OperationActor* destination, OperationType opType)
{
  return m_CurUndoModel->GetLastOfType(destination, opType);
}

mitk::UndoModel* mitk::UndoController::GetCurrentUndoModel()
{
  return m_CurUndoModel;
}
