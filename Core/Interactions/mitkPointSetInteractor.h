#ifndef MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF
#define MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF

#include "mitkCommon.h"
#include "mitkVector.h"
#include <mitkInteractor.h>

namespace mitk
{
class DataTreeNode;
//##ModelId=3F017AFD00C3
//##Documentation
//## @brief Interaction with a set of points.
//## @ingroup Interaction
//## Points can be added, removed and moved.
class PointSetInteractor : public Interactor
{
  public:
    //##ModelId=3F017B3200F2
    mitkClassMacro(PointSetInteractor, Interactor);

    //##ModelId=3F017B3200F5
    //##Documentation
    //##@brief Constructor with Param n for limited Set of Points
    //##
    //## if no n is set, then the number of points is unlimited*
    PointSetInteractor(const char * type, DataTreeNode* dataTreeNode, int n = -1);

    //##ModelId=3F017B320103
  	virtual ~PointSetInteractor();

	  //##Documentation
	  //## @brief clears all the elements in the list with undo-functionality
	  //##
	  //## moved due to use in functionalities
	  void Clear();

 	  //##Documentation
	  //## @brief Sets the amount of precision
    void SetPrecision(unsigned int precision);

  protected:
    //##ModelId=3F017B320105
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId);

    //##ModelId=3F017B320121
    //##Documentation
    //## @brief deselects the Points in the PointSet.
    //## supports Undo if enabled
    //## @params needed for declaring operations
    void UnselectAll(int objectEventId, int groupEventId);

    //##ModelId=3F05C0700185
    //##Documentation
    //## @brief Selects the point.
    //## supports Undo if enabled.
    //## @params needed for declaring operations
    void SelectPoint(int position, int objectEventId, int groupEventId);

  private:

    //##ModelId=3F017B3200E3
	  //##Documentation
	  //## @brief the number of possible points in this object
    //##
    //## if -1, then no limit set
	  int m_N;

    //##ModelId=3F0AF6CF00C2
    //##Documentation
    //## @brief to calculate a direction vector from last point and actual point
    ITKPoint3D m_LastPoint;

    //##Documentation
    //## @brief summ-vector for Movement
    ITKVector3D m_SumVec;

    //##Documentation
    //## @brief to store the value of precision to pick a point
    unsigned int m_Precision;
};
}
#endif /* MITKPOINTSETINTERACTOR_H_HEADER_INCLUDED_C11202FF */
