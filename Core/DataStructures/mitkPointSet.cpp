#include "mitkPointSet.h"

#include <mitkOperation.h>
#include <mitkOperationActor.h>

#include <qapplication.h>//@todo because of quickimplementation UpdateAllWidgets
#include <qwidgetlist.h>//@todo because of quickimplementation UpdateAllWidgets

#include <mitkPointOperation.h>
#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"

inline void UpdateAllWidgets()//@todo global quickimplementation. is to be changed into Modified Data...
{
    QWidgetList *list = QApplication::allWidgets();
    QWidgetListIt it( *list );         // iterate over the widgets
    QWidget * w;
    while ( (w=it.current()) != 0 ) {  // for each widget...
        ++it;
        w->update();
    }
    delete list;                      // delete the list, not the widgets
  
}

//##ModelId=3F0177E901BD
mitk::PointSet::PointSet()
{
  //init new itk::PointSet
  m_PointList = PointSetType::New();
  m_Geometry3D->Initialize(1);
}

//##ModelId=3F0177E901BE
mitk::PointSet::~PointSet()
{}

//##ModelId=3F0177E901C1
int mitk::PointSet::GetSize()
{
  return m_PointList->GetNumberOfPoints();
}

//##ModelId=3F0177E901CC
const mitk::PointSet::PointSetType::Pointer mitk::PointSet::GetPointList() const
{
  return m_PointList;
}
//##ModelId=3F054CE203B8
const mitk::PointSet::BoolList& mitk::PointSet::GetSelectList() const
{
	return m_SelectList;
}

//##ModelId=3F0177E901DE
//##Documentation
//##@brief searches a point in the list with a given precision
int mitk::PointSet::SearchPoint(ITKPoint3D point, float distance )
{
//in is the given point to search
//out is the point which is checked to be the searched point
//pout is a pointer on out to work with itkPointSet
  PointType in, out, *pout;

  //converting point:
  in[0] = point[0];
  in[1] = point[1];
  in[2] = point[2];
  out.Fill(0);
  pout = &out;
	
  //searching the first point in the List, that is +- distance far away from the given point
  unsigned int i;
 	for (i=0; i < m_PointList->GetNumberOfPoints(); i++)
	{
    m_PointList->GetPoint(i, pout);//changes out
    if (in == out)//if totaly equal
      return i;
    
    //distance calculation 
		if ( ( in[0] >= ( out[0] - distance ) ) && ( in[0] <= ( out[0] + distance ) ) &&
			 ( in[1] >= ( out[1] - distance ) ) && ( in[1] <= ( out[1] + distance ) ) &&
			 ( in[2] >= ( out[2] - distance ) ) && ( in[2] <= ( out[2] + distance ) ) )
      return i;
	}
	return -1;
}

//##ModelId=3F0177E901CE
const mitk::PointSet::PointType mitk::PointSet::GetPoint(int position)
{
	if ((int)m_PointList->GetNumberOfPoints()< position)
	{
    return NULL;
	}
  PointType out;
  out.Fill(0);
  m_PointList->GetPoint(position, &out);
  return out;
}

const mitk::ITKPoint3D mitk::PointSet::GetItkPoint(int position)
{
  if ((int)m_PointList->GetNumberOfPoints()< position)
	{
    return NULL;
	}
	mitk::ITKPoint3D itkout;
  PointType out;
  out.Fill(0);
  m_PointList->GetPoint(position, &out);
  itkout[0] = out[0];
  itkout[1] = out[1];
  itkout[2] = out[2];
  return itkout;
}

//##ModelId=3F0177E901DC
bool mitk::PointSet::GetSelectInfo(int position)
{
  if ( (int)m_SelectList.size() > position )
	{
        return m_SelectList[position];
	}
	else
		return false;
}

//##ModelId=3F05B07B0147
const int mitk::PointSet::GetNumberOfSelected()
{
    int numberOfSelected = 0;
    BoolListConstIter it;
    for (it = m_SelectList.begin(); it != m_SelectList.end(); it++)
    {
        if (*it == true)
            numberOfSelected++;
    }
    return numberOfSelected;
}

//##ModelId=3F0177E901BF
void mitk::PointSet::ExecuteOperation(Operation* operation)
/*Notes
	a new point is set to unselected. if it is to be selected, then define in StateMachine as a additional sideeffect
*/
{
	mitk::PointOperation *pointOp = dynamic_cast<mitk::PointOperation *>(operation);
	if (pointOp == NULL)
	{
		(StatusBar::GetInstance())->DisplayText("Recieved wrong type of operation!See mitkPointSetInteractor.cpp", 10000);
		return;
	}

	switch (operation->GetOperationType())
	{
	case OpNOTHING:
		break;
	case OpADD://adds the point to the end
		{
      unsigned long idoffset = m_PointList->GetNumberOfPoints();

      mitk::ITKPoint3D a0 = pointOp->GetPoint();
      PointType a1;
      a1[0] = a0[0];
      a1[1] = a0[1];
      a1[2] = a0[2];

      m_PointList->SetPoint(idoffset, a1);

      m_SelectList.push_back(false);
		}
		break;
	case OpINSERT://inserts the point at the given position pointOperation.index
		{
      PointsContainer::Pointer itkPoints = m_PointList->GetPoints();
      
			if (m_PointList->GetNumberOfPoints() >= ((unsigned)(pointOp->GetIndex())))
			{
        PointType pt;
        pt.CastFrom(pointOp->GetPoint());

        itkPoints->InsertElement(pointOp->GetIndex(), pt);
        m_PointList->SetPoints( itkPoints );

        BoolListIter selPosition = m_SelectList.begin();
        selPosition+=pointOp->GetIndex();
				selPosition = m_SelectList.insert(selPosition, false);
			}
		}
		break;
	case OpMOVE://moves the point given by index
		{//check if working!
      unsigned int index = pointOp->GetIndex();
      PointsContainer::Pointer itkPoints = m_PointList->GetPoints();
			if (index < m_PointList->GetNumberOfPoints() )//checking, cause .at is not supported by older compilers
			{
        PointType pt;
        pt.CastFrom(pointOp->GetPoint());
        m_PointList->SetPoint(index, pt);
			}
		}
		break;
	case OpDELETE://deletes the last point in list
		{
      m_PointList->GetPoints()->pop_back();
			m_SelectList.pop_back();			
		}
		break;
	case OpREMOVE://removes the point at position pointOperation.index
		{
      unsigned int index = (unsigned)pointOp->GetIndex();
			if (index < m_PointList->GetNumberOfPoints() )//checking, cause .at is not supported by older compilers
			{
        //PointSet doesn't delete the Index! So use the std::vector behind it!
        //pc->DeleteIndex(index);//doesn't work by ITK!!!

#ifdef INTERDEBUG
        std::cout<<"Numbers in List before remove:"<<m_PointList->GetNumberOfPoints();
#endif
        PointsContainer::Pointer pc = m_PointList->GetPoints();
        itk::Point<mitk::ScalarType, 3>* position = &((*pc)[index]);
        pc->erase(position);

#ifdef INTERDEBUG
        std::cout<<"Numbers in List after remove:"<<m_PointList->GetNumberOfPoints()<<std::endl;
#endif

        BoolListIter selPosition = m_SelectList.begin();
        selPosition+=pointOp->GetIndex();
        selPosition = m_SelectList.erase(selPosition);
			}
		}
		break;
	case OpSELECTPOINT://select the given point
		{
			m_SelectList[pointOp->GetIndex()] = true;
		}
		break;
	case OpDESELECTPOINT://unselect the given point
		{
			m_SelectList[pointOp->GetIndex()] = false;
		}
		break;
	default:
		NULL;
	}
	//to tell the mappers, that the data is modifierd and has to be updated
	this->Modified();

    ((const itk::Object*)this)->InvokeEvent(itk::EndEvent());

    UpdateAllWidgets();
}

//##ModelId=3F0177E901EE
void mitk::PointSet::UpdateOutputInformation()
{
  m_Geometry3D->SetBoundingBox(m_PointList->GetBoundingBox());
}

//##ModelId=3F0177E901FB
void mitk::PointSet::SetRequestedRegionToLargestPossibleRegion()
{
}
//##ModelId=3F0177E901FD
bool mitk::PointSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}
//##ModelId=3F0177E901FF
bool mitk::PointSet::VerifyRequestedRegion()
{
    return false;
}
//##ModelId=3F0177E9020B
void mitk::PointSet::SetRequestedRegion(itk::DataObject*)
{
}