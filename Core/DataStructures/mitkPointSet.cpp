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
  m_ItkData = DataType::New();
  PointDataContainer::Pointer pointData = PointDataContainer::New();
  m_ItkData->SetPointData(pointData);
  m_Geometry3D->Initialize();
}

//##ModelId=3F0177E901BE
mitk::PointSet::~PointSet()
{}

//##ModelId=3F0177E901C1
int mitk::PointSet::GetSize()
{
  return m_ItkData->GetNumberOfPoints();
}

//##ModelId=3F0177E901CC
const mitk::PointSet::DataType::Pointer mitk::PointSet::GetPointSet() const
{
  return m_ItkData;
}

//##ModelId=3F0177E901DE
//##Documentation
//##@brief searches a point in the list with a given precision
int mitk::PointSet::SearchPoint(ITKPoint3D point, float distance )
{
//in is the given point to search
//out is the point which is checked to be the searched point
//pout is a pointer on out to work with itkPointSet
  PointType in, out;

  //converting point:
  in[0] = point[0];
  in[1] = point[1];
  in[2] = point[2];
  out.Fill(0);

  //searching the first point in the Set, that is +- distance far away from the given point
  unsigned int i;
  PointsContainer::Iterator it, end;
  end = m_ItkData->GetPoints()->End();
 	for (it = m_ItkData->GetPoints()->Begin(), i=0; it != end; it++, i++)
	{
    bool ok = m_ItkData->GetPoints()->GetElementIfIndexExists(it->Index(), &out);//could be optimized
    if (!ok)
      return -1;
    else if (in == out)//if totaly equal
      return it->Index();

    //distance calculation
		if ( ( in[0] >= ( out[0] - distance ) ) && ( in[0] <= ( out[0] + distance ) ) &&
			 ( in[1] >= ( out[1] - distance ) ) && ( in[1] <= ( out[1] + distance ) ) &&
			 ( in[2] >= ( out[2] - distance ) ) && ( in[2] <= ( out[2] + distance ) ) )
      return it->Index();
	}
	return -1;
}

//##ModelId=3F0177E901CE
//##Documentation
//##@brief check if index exists. If it doesn't exist, then return 0,0,0
const mitk::PointSet::PointType mitk::PointSet::GetPoint(int position)
{
  PointType out;
  out.Fill(0);

  if (m_ItkData->GetPoints()->IndexExists(position))
  {
    m_ItkData->GetPoint(position, &out);
    return out;
  }
  else
    return out;
}

const mitk::ITKPoint3D mitk::PointSet::GetItkPoint(int position)
{
	mitk::ITKPoint3D itkout;
  PointType out;
  out.Fill(0);
  
  //if the Index exists... if not, then return 0,0,0
  if (m_ItkData->GetPoints()->IndexExists(position))
  {
    m_ItkData->GetPoint(position, &out);
  }

  itkout[0] = out[0];
  itkout[1] = out[1];
  itkout[2] = out[2];
  return itkout;
}

bool mitk::PointSet::IndexExists(int position)
{
  return m_ItkData->GetPoints()->IndexExists(position);
}

//##ModelId=3F0177E901DC
bool mitk::PointSet::GetSelectInfo(int position)
{
  if (m_ItkData->GetPoints()->IndexExists(position))
	{
    bool selected = false;
    m_ItkData->GetPointData(position, &selected);
    return selected;
	}
	else
		return false;
}

//##ModelId=3F05B07B0147
const int mitk::PointSet::GetNumberOfSelected()
{
    int numberOfSelected = 0;
    for (PointDataIterator it = m_ItkData->GetPointData()->Begin(); it != m_ItkData->GetPointData()->End(); it++)
    {
        if (it->Value() == true)
            numberOfSelected++;
    }
    return numberOfSelected;
}

//##ModelId=3F0177E901BF
//##Documentation
//## @brief executes the given Operation
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
	case OpINSERT://inserts the point at the given position and select it. In the most cases the point is wanted to be selected.
		{
      int position = pointOp->GetIndex();

      PointType pt;
      pt.CastFrom(pointOp->GetPoint());

      m_ItkData->GetPoints()->InsertElement(position, pt);
      m_ItkData->GetPointData()->InsertElement(position, true);
		}
		break;
	case OpMOVE://moves the point given by index
		{
			PointType pt;
      pt.CastFrom(pointOp->GetPoint());
      m_ItkData->SetPoint(pointOp->GetIndex(), pt);
		}
		break;
	case OpREMOVE://removes the point at given by position 
		{
      m_ItkData->GetPoints()->DeleteIndex((unsigned)pointOp->GetIndex());
      m_ItkData->GetPointData()->DeleteIndex((unsigned)pointOp->GetIndex());
		}
		break;
	case OpSELECTPOINT://select the given point
		{
      m_ItkData->SetPointData(pointOp->GetIndex(), true);
		}
		break;
	case OpDESELECTPOINT://unselect the given point
		{
		  m_ItkData->SetPointData(pointOp->GetIndex(), false);
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
  const DataType::BoundingBoxType *bb = m_ItkData->GetBoundingBox();
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

