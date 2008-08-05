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


#ifndef UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79
#define UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79

#include "mitkCommon.h"
#include "mitkUndoModel.h"
#include "mitkOperationEvent.h"
#include <map>


namespace mitk {

//## @ingroup Undo
class MITK_CORE_EXPORT UndoController
{
  public:
  typedef int UndoType;
  typedef std::map<UndoType, UndoModel*> UndoModelMap;
  typedef std::map<UndoType, UndoModel*>::iterator UndoModelMapIter;
  //##Documentation
  //## @brief Default UndoModel to use.
  static const UndoType DEFAULTUNDOMODEL;

  //##Documentation
  //## Constructor; Adds the new UndoType or if undoType exists ,
  //## switches it to undoType; for UndoTypes see definitionmitkInteractionConst.h
  UndoController(UndoType undoType = DEFAULTUNDOMODEL);

  bool SetOperationEvent(UndoStackItem* operationEvent);

  //##Documentation
  //## @brief calls the UndoMechanism to undo the last change
  bool Undo();

  //##Documentation
  //## @brief calls the UndoMechanism to undo the last change
  //##
  //## the UndoMechanism has the possibility to undo the last changes in two different ways:
  //## first it can Undo a group of operations done at last (e.g. build up a new object; Undo leads to deleting that object);
  //## or it can Undo a set of operations, that belong together(statechange with Action),
  //## that way it is possible recall the last set point after you have finished to build up a new object
  //## @param fine: if set to true, then undo all operations with the same objectEventId
  //## if set to false, then undo all operations with the same GroupEventId
  bool Undo(bool fine);

  //##Documentation
  //## @brief calls the RedoMechanism to redo the operations undone
  //##
  //## read the Documentation of Undo!
  bool Redo();

  //##Documentation
  //## @brief calls the RedoMechanism to redo the operations undone
  //##
  //## read the Documentation of Undo!
  //## only with the possibility to fine redo, like fine undo
  bool Redo(bool fine);

  //##Documentation
  //## @brief Clears the Undo and the RedoList
  void Clear();

  //##Documentation
  //## @brief Clears the RedoList
  void ClearRedoList();

  //##Documentation
  //## @brief returns true, if the RedoList is empty
  bool RedoListEmpty();

  bool SwitchUndoModel(UndoType undoType);

  bool AddUndoModel(UndoType undoType);

  bool RemoveUndoModel(UndoType undoType);

  //##Documentation
  //## @brief returns the ObjectEventId of the 
  //## top Element in the OperationHistory of the selected 
  //## UndoModel
  int GetLastObjectEventIdInList();

  //##Documentation
  //## @brief returns the GroupEventId of the 
  //## top Element in the OperationHistory of the selected 
  //## UndoModel
  int GetLastGroupEventIdInList();


  //##Documentation
  //## @brief returns the last specified OperationEvent in Undo-list  
  //## corresponding to the given value; if nothing found, then returns NULL
  OperationEvent* GetLastOfType(OperationActor* destination, OperationType opType);

  //##Documentation
  //## @brief gives access to the currently used UndoModel
  //## Introduced to access special functions of more specific UndoModels,
  //## especially to retrieve text descriptions of the undo/redo stack
  static UndoModel* GetCurrentUndoModel();
  
  private:
  //##Documentation
  //## current selected UndoModel
  static UndoModel *m_CurUndoModel;
  //##Documentation
  //## current selected Type of m_CurUndoModel
  static UndoType m_CurUndoType;
  //##Documentation
  //## different UndoModels to select and activate
  static UndoModelMap m_UndoModelList;

};
}//namespace mitk

#endif /* UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79 */
