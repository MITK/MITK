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
  m_PointSet = PointSetType::New();
  PointDataContainer::Pointer pointData = PointDataContainer::New();
  m_PointSet->SetPointData(pointData);
  m_Geometry3D->Initialize();
}

//##ModelId=3F0177E901BE
mitk::PointSet::~PointSet()
{}

//##ModelId=3F0177E901C1
int mitk::PointSet::GetSize()
{
  return m_PointSet->GetNumberOfPoints();
}

//##ModelId=3F0177E901CC
const mitk::PointSet::PointSetType::Pointer mitk::PointSet::GetPointList() const
{
  return m_PointSet;
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
 	for (i=0; i < m_PointSet->GetNumberOfPoints(); i++)
	{
    m_PointSet->GetPoint(i, pout);//changes out
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
	if ((int)m_PointSet->GetNumberOfPoints()< position)
	{
    return NULL;
	}
  PointType out;
  out.Fill(0);
  m_PointSet->GetPoint(position, &out);
  return out;
}

const mitk::ITKPoint3D mitk::PointSet::GetItkPoint(int position)
{
  if ((int)m_PointSet->GetNumberOfPoints()< position)
	{
    return NULL;
	}
	mitk::ITKPoint3D itkout;
  PointType out;
  out.Fill(0);
  m_PointSet->GetPoint(position, &out);
  itkout[0] = out[0];
  itkout[1] = out[1];
  itkout[2] = out[2];
  return itkout;
}

//##ModelId=3F0177E901DC
bool mitk::PointSet::GetSelectInfo(int position)
{
  if ( m_PointSet->GetNumberOfPoints() > position )
	{
    bool selected = false;
    m_PointSet->GetPointData(position, &selected);
    return selected;
	}
	else
		return false;
}

//##ModelId=3F05B07B0147
const int mitk::PointSet::GetNumberOfSelected()
{
    int numberOfSelected = 0;
    for (PointDataIterator it = m_PointSet->GetPointData()->Begin(); it != m_PointSet->GetPointData()->End(); it++)
    {
        if (it->Value() == true)
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
	case OpADD://!!!!!#### shall not be used! Use Insert instead to know the position for undo-operation ####!!!!!
		{
      unsigned long idoffset = m_PointSet->GetNumberOfPoints();
      mitk::ITKPoint3D a0 = pointOp->GetPoint();
      PointType a1;
      a1[0] = a0[0];
      a1[1] = a0[1];
      a1[2] = a0[2];
      m_PointSet->SetPoint(idoffset, a1);
      m_PointSet->SetPointData(idoffset, false);
		}
		break;
	case OpINSERT://inserts the point at the given position 
		{
      int position = pointOp->GetIndex();

      if (m_PointSet->GetNumberOfPoints() >= ((unsigned)(position)))
			{
        PointType pt;
        pt.CastFrom(pointOp->GetPoint());

        m_PointSet->GetPoints()->InsertElement(position, pt);
        m_PointSet->GetPointData()->InsertElement(position, false);
			}
		}
		break;
	case OpMOVE://moves the point given by index
		{//check if working!
      unsigned int index = pointOp->GetIndex();
      PointsContainer::Pointer itkPoints = m_PointSet->GetPoints();
			if (index < m_PointSet->GetNumberOfPoints() )//checking, cause .at is not supported by older compilers
			{
        PointType pt;
        pt.CastFrom(pointOp->GetPoint());
        m_PointSet->SetPoint(index, pt);
			}
		}
		break;
	case OpDELETE://!!!!####shall not be used anymore! Use remove(index) instead to know the position for undo-operation####!!!!
		{
   //   PointsContainer::Pointer position = m_PointSet->GetPoints();
   //   position->DeleteIndex(position->size()-1);
			//m_SelectList.pop_back();

      m_PointSet->GetPoints()->DeleteIndex(m_PointSet->GetNumberOfPoints()-1);
		}
		break;
	case OpREMOVE://removes the point at given by position 
		{
      unsigned int index = (unsigned)pointOp->GetIndex();
			if (index < m_PointSet->GetNumberOfPoints() )
			{
        m_PointSet->GetPoints()->DeleteIndex(index);
			}
		}
		break;
	case OpSELECTPOINT://select the given point
		{
      m_PointSet->SetPointData(pointOp->GetIndex(), true);
		}
		break;
	case OpDESELECTPOINT://unselect the given point
		{
		  m_PointSet->SetPointData(pointOp->GetIndex(), false);
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
  const PointSetType::BoundingBoxType *bb = m_PointSet->GetBoundingBox();
  BoundingBox::BoundsArrayType itkBounds = bb->GetBounds();
  float mitkBounds[6];

  //for assignment see Geometry3d::SetBoundingBox(const float bounds)
  mitkBounds[0] = itkBounds[0];
  mitkBounds[1] = itkBounds[1];
  mitkBounds[2] = itkBounds[2];
  mitkBounds[3] = itkBounds[3];
  mitkBounds[4] = itkBounds[4];
  mitkBounds[5] = itkBounds[5];
//@TODO check if order is correct 012345 or 024135; Bounds/BoundingBox and so on.

  m_Geometry3D->SetBoundingBox(mitkBounds);
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

