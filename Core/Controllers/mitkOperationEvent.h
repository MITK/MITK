#ifndef OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC
#define OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC

#include "mitkCommon.h"
#include "Operation.h"
#include "OperationActor.h"
#include "UndoModel.h"



namespace mitk {
//##ModelId=3E5F60F301A4
//##Documentation
//## @brief represents a pair of operations: Undo and the according Redo.
//## @ingroup Undo
//## includes a generation of Group and ObjectEventId's, to seperate different OperationEvents
//## and to undo them in a fine or in a raw mode.
class OperationEvent
{		
  public:
    //##ModelId=3E957AE700E6
    OperationEvent(OperationActor* destination, Operation* operation, Operation* undoOperation, int objectEventId, int groupEventId );

    //##ModelId=3F0451960212
	~OperationEvent();

    //##ModelId=3E5F610D00BB
    Operation* GetOperation();

    //##ModelId=3E9B07B50220
	//##Documentation
	//## @brief for combining operations in Groups
	//##
	//## This ID is used in the Undo-Mechanism.
	//## For separation of the seperate operations
	//## If the GroupEventId of two OperationEvents is equal, 
	//## then they share one group and will be undone in case of Undo(fine==false)
	static int GetCurrGroupEventId();

	//##ModelId=3E9B07B501A7
	//##Documentation
	//## @brief for combining operations in Objects
	//##
	//## This ID is used in the Undo-Mechanism.
	//## For separation of the seperate operations
	//## If the ObjectEventId of two OperationEvents is equal, 
	//## then they share one Object and will be undone in all cases of Undo(true and false).
	//## they shal not be seperated, because they were produced to realize one object-change.
	//## for example: OE_statechange and OE_addlastpoint
	static int GetCurrObjectEventId();
    
    //##ModelId=3E9B07B502AC
	//## brief returns the destination of the operations
	OperationActor* GetDestination();

    //##ModelId=3EF099E90249
    //##Documentation
    //## @brief Returns the GroupEventId for this object
	int GetGroupEventId();
	
    //##ModelId=3EF099E90259
    //##Documentation
    //## @brief Returns the ObjectEventId for this object
	int GetObjectEventId();
	


	friend class UndoModel;	
	friend class StateMachine;//for IncCurrGroupEventId
	friend class EventMapper;//for IncCurrObjectEventId

  protected:
	//##ModelId=3E957C1102E3
	//##Documentation
	//## @brief swaps the two operations and sets a flag, 
	//## that it has been swapped and do is undo and undo is do
    void SwapOperations();
    
    //##ModelId=3EF099E90269
	//##Documentation
	//## @brief increments the current GroupEventId and returns the new value
	static int IncCurrGroupEventId();
	
    //##ModelId=3EF099E90289
	//##Documentation
	//## @brief increments the current ObjectEventId and returns the new value
	static int IncCurrObjectEventId();

	  
  private:
    //##ModelId=3E9B07B40374
	static int m_CurrObjectEventId;

    //##ModelId=3E9B07B5002B
	static int m_CurrGroupEventId;

    //##ModelId=3E5F61DB00D6
    OperationActor* m_Destination;

    //##ModelId=3E5F6B1E0107
    Operation* m_Operation;

    //##ModelId=3E5F6B2E0060
    Operation* m_UndoOperation;

    //##ModelId=3E7F488E022B
    int m_ObjectEventId;

    //##ModelId=3E7F48C60335
    int m_GroupEventId;

    //##ModelId=3E9B07B500D5
	//##Documentation
	//## @brief true, if operation and undooperation have been swaped/changed
	bool m_Swaped;
};
} //namespace mitk

#endif /* OPERATIONEVENT_H_HEADER_INCLUDED_C16E83FC */