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
//##ModelId=3E5F53AA026C

//## @ingroup Undo
class MITK_CORE_EXPORT UndoController
{
  public:
  //##ModelId=3F01770900C3
  typedef int UndoType;
  //##ModelId=3F01770900D2
  typedef std::map<UndoType, UndoModel*> UndoModelMap;
  //##ModelId=3F01770900E2
  typedef std::map<UndoType, UndoModel*>::iterator UndoModelMapIter;
  //##ModelId=3F01770A0170
  //##Documentation
  //## @brief Default UndoModel to use.
  static const UndoType DEFAULTUNDOMODEL;

  //##ModelId=3E956E3B03A8
  //##Documentation
  //## Constructor; Adds the new UndoType or if undoType exists ,
  //## switches it to undoType; for UndoTypes see definitionmitkInteractionConst.h
  UndoController(UndoType undoType = DEFAULTUNDOMODEL);

  //##ModelId=3E5F543402C5
  bool SetOperationEvent(UndoStackItem* operationEvent);

  //##ModelId=3E5F55C80207
  //##Documentation
  //## @brief calls the UndoMechanism to undo the last change
  bool Undo();

  //##ModelId=3F045196036A
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

  //##ModelId=3E5F55E6003E
  //##Documentation
  //## @brief calls the RedoMechanism to redo the operations undone
  //##
  //## read the Documentation of Undo!
  bool Redo();

  //##ModelId=3F0451960398
  //##Documentation
  //## @brief calls the RedoMechanism to redo the operations undone
  //##
  //## read the Documentation of Undo!
  //## only with the possibility to fine redo, like fine undo
  bool Redo(bool fine);

  //##ModelId=3F04519603B8
  //##Documentation
  //## @brief Clears the Undo and the RedoList
  void Clear();

  //##ModelId=3F04519603C7
  //##Documentation
  //## @brief Clears the RedoList
  void ClearRedoList();

  //##ModelId=3F04519603D7
  //##Documentation
  //## @brief returns true, if the RedoList is empty
  bool RedoListEmpty();

  //##ModelId=3E5F56EB017A
  bool SwitchUndoModel(UndoType undoType);

  //##ModelId=3E9C45D502B9
  bool AddUndoModel(UndoType undoType);

  //##ModelId=3E9C45D502EB
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
  //##ModelId=3EAEBEBD02DC
  //##Documentation
  //## current selected UndoModel
  static UndoModel *m_CurUndoModel;
  //##ModelId=3F0185C90085
  //##Documentation
  //## current selected Type of m_CurUndoModel
  static UndoType m_CurUndoType;
  //##ModelId=3F0185C900A5
  //##Documentation
  //## different UndoModels to select and activate
  static UndoModelMap m_UndoModelList;

};
}//namespace mitk

#endif /* UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79 */
