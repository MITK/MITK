#ifndef UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79
#define UNDOCONTROLLER_H_HEADER_INCLUDED_C16EFF79

#include "mitkCommon.h"
#include "UndoModel.h"
#include "OperationEvent.h"
#include <map>


namespace mitk {

	enum UndoType
	{
		//NULL = 0,
		LIMITEDLINEARUNDO = 1,
		TREEUNDO = 2
	};

typedef std::map<UndoType, UndoModel*> UndoModelMap;

typedef std::map<UndoType, UndoModel*>::iterator UndoModelMapIter;


//##ModelId=3E5F53AA026C
class UndoController
{
  public:
	//##ModelId=3E956E3B03A8
	//##Documentation
    //## Constructor; Adds the new UndoType or if undoType exists , 
	//## switches it to undoType
    UndoController(UndoType undoType);
	
    //##ModelId=3EA585FD0261
	//##Documentation
	//##plain constructor
    UndoController();

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
    //##ModelId=3EA93FC40068
	//##Documentation
    //## current selected UndoModel
	static UndoModel *m_CurUndoModel;
    //##ModelId=3E9C45D50256
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
