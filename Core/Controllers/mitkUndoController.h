#ifndef UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79
#define UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79

#include "mitkCommon.h"
#include "UndoModel.h"
#include "OperationEvent.h"
#include <map>


namespace mitk {

//##ModelId=3EDCAEC901C3
typedef int UndoType;

//##ModelId=3E9C45D5007E
typedef std::map<UndoType, UndoModel*> UndoModelMap;

//##ModelId=3E9C45D50092
typedef std::map<UndoType, UndoModel*>::iterator UndoModelMapIter;

static const mitk::UndoType DEFAULTUNDOMODEL;

//##ModelId=3E5F53AA026C
class UndoController
{
  public:
	//##ModelId=3E956E3B03A8
	//##Documentation
    //## Constructor; Adds the new UndoType or if undoType exists ,
	//## switches it to undoType; for UndoTypes see definitionmitkInteractionConst.h
    UndoController(UndoType undoType = DEFAULTUNDOMODEL);

    //##ModelId=3E5F543402C5
    bool SetOperationEvent(OperationEvent* operationEvent);

    //##ModelId=3E5F55C80207
    bool Undo();

    //##ModelId=3E5F55E6003E
    bool Redo();


    //##ModelId=3E5F56EB017A
    bool SwitchUndoModel(UndoType undoType);

    //##ModelId=3E9C45D502B9
	bool AddUndoModel(UndoType undoType);

    //##ModelId=3E9C45D502EB
	bool RemoveUndoModel(UndoType undoType);

  private:
    //##ModelId=3EAEBEBD02DC
	//##Documentation
    //## current selected UndoModel
	static UndoModel *m_CurUndoModel;
    //##ModelId=3EDCAECC0176
	//##Documentation
    //## current selected Type of m_CurUndoModel
	static UndoType m_CurUndoType;
    //##ModelId=3E9C45D5026A
	//##Documentation
    //## different UndoModels to select and activate
	static UndoModelMap m_UndoModelList;

};
}//namespace mitk

#endif /* UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79 */