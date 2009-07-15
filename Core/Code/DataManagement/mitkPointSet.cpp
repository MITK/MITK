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


#include "mitkPointSet.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"

#include <itkSmartPointerForwardReference.txx>


mitk::PointSet::PointSet()
{
  this->InitializeEmpty();
}

mitk::PointSet::~PointSet()
{
  this->ClearData();
}

void mitk::PointSet::ClearData()
{
  m_PointSetSeries.clear();
  Superclass::ClearData();
}

void mitk::PointSet::InitializeEmpty()
{
  m_PointSetSeries.resize( 1 );

  m_PointSetSeries[0] = DataType::New();
  PointDataContainer::Pointer pointData = PointDataContainer::New();
  m_PointSetSeries[0]->SetPointData( pointData );
  m_CalculateBoundingBox = false;

  Superclass::InitializeTimeSlicedGeometry(1);
  m_Initialized = true;
}

bool mitk::PointSet::IsEmpty(unsigned int t) const
{
  return IsInitialized() && (GetSize(t) == 0);
}

void mitk::PointSet::Expand( unsigned int timeSteps )
{
  // Check if the vector is long enough to contain the new element
  // at the given position. If not, expand it with sufficient pre-initialized
  // elements.
  //
  // NOTE: This method will never REDUCE the vector size; it should only
  // be used to make sure that the vector has enough elements to include the
  // specified time step.

  unsigned int oldSize = m_PointSetSeries.size();

  if ( timeSteps > oldSize )
  {
    Superclass::Expand( timeSteps );

    m_PointSetSeries.resize( timeSteps );
    for ( unsigned int i = oldSize; i < timeSteps; ++i )
    {
      m_PointSetSeries[i] = DataType::New();
      PointDataContainer::Pointer pointData = PointDataContainer::New();
      m_PointSetSeries[i]->SetPointData( pointData );
    }

    //if the size changes, then compute the bounding box
    m_CalculateBoundingBox = true;

    this->InvokeEvent( PointSetExtendTimeRangeEvent() );
  }
}


unsigned int mitk::PointSet::GetPointSetSeriesSize() const
{
  return m_PointSetSeries.size();
}


int mitk::PointSet::GetSize( unsigned int t ) const
{
  if ( t < m_PointSetSeries.size() )
  {
    return m_PointSetSeries[t]->GetNumberOfPoints();
  }
  else
  {
    return 0;
  }
}

mitk::PointSet::DataType::Pointer mitk::PointSet::GetPointSet( int t ) const
{
  if ( t < (int)m_PointSetSeries.size() )
  {
    return m_PointSetSeries[t];
  }
  else
  {
    return NULL;
  }
}

int mitk::PointSet::SearchPoint( Point3D point, float distance, int t  ) const
{
  if ( t >= (int)m_PointSetSeries.size() )
  {
    return -1;
  }
  
  // Out is the point which is checked to be the searched point
  PointType out;
  out.Fill( 0 );
  PointType indexPoint;

  this->GetGeometry( t )->WorldToIndex(point, indexPoint);

  // Searching the first point in the Set, that is +- distance far away fro
  // the given point
  unsigned int i;
  PointsContainer::Iterator it, end;
  end = m_PointSetSeries[t]->GetPoints()->End();
  int bestIndex = -1;
  distance = distance * distance;
  
  // To correct errors from converting index to world and world to index
  if (distance == 0.0)
  {
    distance = 0.000001;
  }

  ScalarType bestDist = distance;
  ScalarType dist, tmp;

  for ( it = m_PointSetSeries[t]->GetPoints()->Begin(), i = 0;
        it != end; 
        ++it, ++i )
  {
    bool ok = m_PointSetSeries[t]->GetPoints()
      ->GetElementIfIndexExists( it->Index(), &out );

    if ( !ok )
    {
      return -1;
    }
    else if ( indexPoint == out ) //if totally equal
    {
      return it->Index();
    }

    //distance calculation
    tmp = out[0] - indexPoint[0]; dist  = tmp * tmp;
    tmp = out[1] - indexPoint[1]; dist += tmp * tmp;
    tmp = out[2] - indexPoint[2]; dist += tmp * tmp;

    if ( dist < bestDist )
    {
      bestIndex = it->Index();
      bestDist  = dist;
    }
  }
  return bestIndex;
}

mitk::PointSet::PointType 
mitk::PointSet::GetPoint( int position, int t ) const
{
  PointType out;
  out.Fill(0);

  if ( (unsigned int) t >= m_PointSetSeries.size() )
  {
    return out;
  }

  if ( m_PointSetSeries[t]->GetPoints()->IndexExists(position) )
  {
    m_PointSetSeries[t]->GetPoint( position, &out );

    this->GetGeometry(t)->IndexToWorld( out, out );

    return out;
  }
  else
  {
    return out;
  }
}


bool 
mitk::PointSet
::GetPointIfExists( PointIdentifier id, PointType* point, int t ) const
{
  if ( (unsigned int) t >= m_PointSetSeries.size() )
  {
    return false;
  }

  if ( m_PointSetSeries[t]->GetPoints()->GetElementIfIndexExists(id, point) )
  {
    this->GetGeometry( t )->IndexToWorld( *point, *point );
    return true;
  }
  else
  {
    return false;
  }
}


void mitk::PointSet::SetPoint( PointIdentifier id, PointType point, int t )
{
  // Adapt the size of the data vector if necessary
  this->Expand( t+1 );

  mitk::Point3D indexPoint;
  this->GetGeometry( t )->WorldToIndex( point, indexPoint );
  m_PointSetSeries[t]->SetPoint( id, indexPoint );

  //boundingbox has to be computed anyway
  m_CalculateBoundingBox = true;
  this->Modified();
}


void mitk::PointSet::InsertPoint( PointIdentifier id, PointType point, int t )
{
  if ( (unsigned int) t < m_PointSetSeries.size() )
  {
    mitk::Point3D indexPoint;
    mitk::Geometry3D* tempGeometry = this->GetGeometry( t );
    if (tempGeometry == NULL)
    {
      LOG_INFO<< __FILE__ << ", l." << __LINE__ << ": GetGeometry of "<< t <<" returned NULL!" << std::endl;
      return;
    }
    tempGeometry->WorldToIndex( point, indexPoint );
    m_PointSetSeries[t]->GetPoints()->InsertElement( id, indexPoint );
    
    //boundingbox has to be computed anyway
    m_CalculateBoundingBox = true;
    this->Modified();
  }
}

void mitk::PointSet::SwapPointPosition( PointIdentifier id, bool moveUpwards, int t )
{
  if(GetSize(t) > 1 && GetNumberOfSelected(t) == 1)
  {
    PointType point = GetPoint(id,t);

    if(moveUpwards)
    {//up
      if(IndexExists(id-1,t))
      {
        m_PointSetSeries[t]->GetPoints()->DeleteIndex(id);
        InsertPoint(id-1,point,t);
      }
    }
    else
    {//down
      if(IndexExists(id+1,t))
      {
        m_PointSetSeries[t]->GetPoints()->DeleteIndex(id);
        InsertPoint(id,point,t);
      }
    }
    this->Modified();
  }
}

bool mitk::PointSet::IndexExists( int position, int t ) const
{
  if ( (unsigned int) t < m_PointSetSeries.size() )
  {
    return m_PointSetSeries[t]->GetPoints()->IndexExists( position );
  }
  else
  {
    return false;
  }
}

bool mitk::PointSet::GetSelectInfo( int position, int t ) const
{
  if ( this->IndexExists( position, t ) )
  {
    PointDataType pointData = { 0, false, PTUNDEFINED };
    m_PointSetSeries[t]->GetPointData( position, &pointData );
    return pointData.selected;
  }
  else
  {
    return false;
  }
}
  
void mitk::PointSet::SetSelectInfo( int position, bool selected, int t )
{
  if ( this->IndexExists( position, t ) )
  {
    // timeStep to ms
    ScalarType timeInMS = this->GetTimeSlicedGeometry()->TimeStepToMS( t );

    // point
    Point3D point = this->GetPoint( position, t );

    PointOperation* op;
    if (selected)
    {
      op = new mitk::PointOperation(OpSELECTPOINT, timeInMS, point, position );
    }
    else
    {
      op = new mitk::PointOperation(OpDESELECTPOINT, timeInMS, point, position );
    }
      
    this->ExecuteOperation( op );
  }
}



int mitk::PointSet::GetNumberOfSelected( int t ) const
{
  if ( (unsigned int) t >= m_PointSetSeries.size() )
  {
    return 0;
  }

  int numberOfSelected = 0;
  PointDataIterator it;
  for ( it = m_PointSetSeries[t]->GetPointData()->Begin();
        it != m_PointSetSeries[t]->GetPointData()->End();
        it++ )
  {
    if (it->Value().selected == true)
    {
      ++numberOfSelected;
    }
  }

  return numberOfSelected;
}


int mitk::PointSet::SearchSelectedPoint( int t ) const
{
  if ( (unsigned int) t >= m_PointSetSeries.size() )
  {
    return -1;
  }

  PointDataIterator it;
  for ( it = m_PointSetSeries[t]->GetPointData()->Begin(); 
        it != m_PointSetSeries[t]->GetPointData()->End();
        it++ )
  {
    if ( it->Value().selected == true )
    {
      return it->Index();
    }
  }
  return -1;
}

void mitk::PointSet::ExecuteOperation( Operation* operation )
{
  int timeStep = -1;

  mitkCheckOperationTypeMacro(PointOperation, operation, pointOp);

  if ( pointOp )
  {
    timeStep = this->GetTimeSlicedGeometry()
      ->MSToTimeStep( pointOp->GetTimeInMS() );
  }

  if ( timeStep < 0 )
  {
    LOG_ERROR << "Time step (" << timeStep << ") outside of PointSet time bounds" << std::endl;
    return;
  }

  switch (operation->GetOperationType())
  {
  case OpNOTHING:
    break;

  case OpINSERT://inserts the point at the given position and selects it. 
    {
      int position = pointOp->GetIndex();

      PointType pt;
      pt.CastFrom(pointOp->GetPoint());

      //transfer from world to index coordinates 
      mitk::Geometry3D* geometry = this->GetGeometry( timeStep );
      if (geometry == NULL)
      {
        LOG_INFO<<"mitkPointSet.cpp::L362: GetGeometry returned NULL!\n";
        return;
      }
      geometry->WorldToIndex(pt, pt);

      m_PointSetSeries[timeStep]->GetPoints()->InsertElement(position, pt);

      PointDataType pointData = 
      {
        pointOp->GetIndex(), 
        pointOp->GetSelected(), 
        pointOp->GetPointType()
      };

      m_PointSetSeries[timeStep]->GetPointData()
        ->InsertElement(position, pointData);

      this->Modified();
      
      //boundingbox has to be computed
      m_CalculateBoundingBox = true;

      this->InvokeEvent( PointSetAddEvent() );
      this->OnPointSetChange();
    }
    break;

  case OpMOVE://moves the point given by index
    {
      PointType pt;
      pt.CastFrom(pointOp->GetPoint());
      
      //transfer from world to index coordinates 
      this->GetGeometry( timeStep )->WorldToIndex(pt, pt);

      m_PointSetSeries[timeStep]->SetPoint(pointOp->GetIndex(), pt);

      this->OnPointSetChange();

      this->Modified();

      //boundingbox has to be computed anyway
      m_CalculateBoundingBox = true;

      this->InvokeEvent( PointSetMoveEvent() );
    }
    break;

  case OpREMOVE://removes the point at given by position 
    {
      m_PointSetSeries[timeStep]->GetPoints()
        ->DeleteIndex((unsigned)pointOp->GetIndex());
      m_PointSetSeries[timeStep]->GetPointData()
        ->DeleteIndex((unsigned)pointOp->GetIndex());

      this->OnPointSetChange();

      this->Modified();
      //boundingbox has to be computed anyway
      m_CalculateBoundingBox = true;

      this->InvokeEvent( PointSetRemoveEvent() );
    }
    break;

  case OpSELECTPOINT://select the given point
    {
      PointDataType pointData = {0, false, PTUNDEFINED};
      m_PointSetSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
      pointData.selected = true;
      m_PointSetSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
      this->Modified();
    }
    break;

  case OpDESELECTPOINT://unselect the given point
    {
      PointDataType pointData = {0, false, PTUNDEFINED};
      m_PointSetSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
      pointData.selected = false;
      m_PointSetSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
      this->Modified();
    }
    break;

  case OpSETPOINTTYPE:
    {
      PointDataType pointData = {0, false, PTUNDEFINED};
      m_PointSetSeries[timeStep]->GetPointData(pointOp->GetIndex(), &pointData);
      pointData.pointSpec = pointOp->GetPointType();
      m_PointSetSeries[timeStep]->SetPointData(pointOp->GetIndex(), pointData);
      this->Modified();
    }
    break;

  case OpMOVEPOINTUP: // move point position within the pointset 
    {
      int position = SearchPoint(pointOp->GetPoint(), 0.0, timeStep);
      mitk::Point3D point1 = pointOp->GetPoint();
        
      if(position >= 1)
      {        
        mitk::Point3D point2 = m_PointSetSeries[timeStep]->GetPoints()->GetElement(position-1);
        m_PointSetSeries[timeStep]->GetPoints()->InsertElement(position-1,point1);
        m_PointSetSeries[timeStep]->GetPoints()->InsertElement(position,point2);
        this->Modified();
      }
      
    }
    break;
  case OpMOVEPOINTDOWN: // move point position within the pointset 
    {
      int position = SearchPoint(pointOp->GetPoint(), 0.0, timeStep);
      mitk::Point3D point1 = pointOp->GetPoint();

      if(position >= 0 && GetSize(timeStep) > position)
      {        
        mitk::Point3D point2 = m_PointSetSeries[timeStep]->GetPoints()->GetElement(position+1);
        m_PointSetSeries[timeStep]->GetPoints()->InsertElement(position+1,point1);
        m_PointSetSeries[timeStep]->GetPoints()->InsertElement(position,point2);
        this->Modified();
      }
    }
    break;

  default:
    itkWarningMacro("mitkPointSet could not understrand the operation. Please check!");
    break;
  }
  
  //to tell the mappers, that the data is modifierd and has to be updated 
  //only call modified if anything is done, so call in cases
  //this->Modified();

  mitk::OperationEndEvent endevent(operation);
  ((const itk::Object*)this)->InvokeEvent(endevent);

  //*todo has to be done here, cause of update-pipeline not working yet
  // As discussed lately, don't mess with the rendering from inside data structures
  //mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void mitk::PointSet::UpdateOutputInformation()
{
  if ( this->GetSource( ) )
  {
      this->GetSource( )->UpdateOutputInformation( );
  }

  //
  // first make sure, that the associated time sliced geometry has
  // the same number of geometry 3d's as PointSets are present
  //
  mitk::TimeSlicedGeometry* timeGeometry = GetTimeSlicedGeometry();
  if ( timeGeometry->GetTimeSteps() != m_PointSetSeries.size() )
  {
    itkExceptionMacro(<<"timeGeometry->GetTimeSteps() != m_PointSetSeries.size() -- use Initialize(timeSteps) with correct number of timeSteps!");
  }

  //
  // Iterate over the PointSets and update the Geometry
  // information of each of the items.
  //
  if (m_CalculateBoundingBox)
  {
    for ( unsigned int i = 0 ; i < m_PointSetSeries.size() ; ++i )
    {
      const DataType::BoundingBoxType *bb = m_PointSetSeries[i]->GetBoundingBox();
      BoundingBox::BoundsArrayType itkBounds = bb->GetBounds();
      
      // Ensure minimal bounds of 1.0 in each dimension
      for ( unsigned int j = 0; j < 3; ++j )
      {
        if ( itkBounds[j*2+1] - itkBounds[j*2] < 1.0 )
        {
          BoundingBox::CoordRepType center = 
            (itkBounds[j*2] + itkBounds[j*2+1]) / 2.0;
          itkBounds[j*2] = center - 0.5;
          itkBounds[j*2+1] = center + 0.5;
        }
      }
      this->GetGeometry(i)->SetBounds(itkBounds);
    }
    m_CalculateBoundingBox = false;
  }
  this->GetTimeSlicedGeometry()->UpdateInformation();
}

void mitk::PointSet::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::PointSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}

bool mitk::PointSet::VerifyRequestedRegion()
{
    return true;
}

void mitk::PointSet::SetRequestedRegion( itk::DataObject * )
{
}


void mitk::PointSet::PrintSelf( std::ostream& os, itk::Indent indent ) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Number timesteps: " << m_PointSetSeries.size() << "\n";
  unsigned int i = 0;
  for (PointSetSeries::const_iterator it = m_PointSetSeries.begin(); it != m_PointSetSeries.end(); ++it)
  {
    os << indent << "Timestep " << i++ << ": \n";
    MeshType::Pointer ps = *it;
    itk::Indent nextIndent = indent.GetNextIndent();
    ps->Print(os, nextIndent);
    MeshType::PointsContainer* points = ps->GetPoints();
    for (MeshType::PointsContainer::iterator it2 = points->begin(); it2 != points->end(); ++it2)
    {
      os << nextIndent << "Point " << it2->first << ": [";
      os << it2->second.GetElement(0);
      for (unsigned int i = 1; i < PointType::GetPointDimension(); ++i)
      {
        os << ", " << it2->second.GetElement(i);
      }
      os << "]\n";
    }
  }
}
