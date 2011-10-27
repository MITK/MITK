#include "mitkPlanePositionManager.h"
#include "mitkInteractionConst.h"


mitk::PlanePositionManager::PlanePositionManager()
: m_ID (0)
{
}

mitk::PlanePositionManager::~PlanePositionManager()
{
  for (unsigned int i = 0; i < m_PositionList.size(); i++)
  {
    delete m_PositionList.at(i);
  }
}

mitk::PlanePositionManager* mitk::PlanePositionManager::GetInstance()
{
  static mitk::PlanePositionManager* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new PlanePositionManager();
  }

  return m_Instance;
}

unsigned int mitk::PlanePositionManager::AddNewPlanePosition ( const Geometry2D* plane, unsigned int sliceIndex )
{
  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  Matrix3D matrix;
  matrix.GetVnlMatrix().set_column(0, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(0));
  matrix.GetVnlMatrix().set_column(1, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(1));
  matrix.GetVnlMatrix().set_column(2, plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2));
  transform->SetMatrix(matrix);
  transform->SetOffset(plane->GetIndexToWorldTransform()->GetOffset());

  mitk::Vector3D direction;
  direction[0] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[0];
  direction[1] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[1];
  direction[2] = plane->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(2)[2];
  direction.Normalize();

  for (unsigned int i = 0; i < m_PositionList.size(); i++)
  {
    if (m_PositionList.at(i) != 0)
    {
      itk::Matrix<float> diffM = plane->GetIndexToWorldTransform()->GetMatrix()-m_PositionList.at(i)->GetTransform()->GetMatrix();
      bool isSameMatrix(true);
      for (unsigned int j = 0; j < 3; j++)
      {
        if (fabs(diffM[j][0]) > 0.00001 || fabs(diffM[j][1]) > 0.00001 || fabs(diffM[j][2]) > 0.00001)
        {
          isSameMatrix = false;
          break;
        }
      }
      itk::Vector<float> diffV = m_PositionList.at(i)->GetTransform()->GetOffset()-transform->GetOffset();
      if ( isSameMatrix && m_PositionList.at(i)->GetPos() == sliceIndex && (fabs(diffV[0]) < 0.00001 && fabs(diffV[1]) < 0.00001 && fabs(diffV[2]) < 0.00001) )
        return i;
    }
    
  }

  mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, plane->GetExtent(0), 
    plane->GetExtent(1), plane->GetSpacing(), sliceIndex, direction, transform);

  m_PositionList.push_back( newOp );
  return GetNumberOfPlanePositions()-1;
}

bool mitk::PlanePositionManager::RemovePlanePosition( unsigned int ID )
{
  if (m_PositionList.size() > ID)
  {
    m_PositionList.at(ID) = 0;
    return true;
  }
  else
  {
    return false;
  }
}

mitk::RestorePlanePositionOperation* mitk::PlanePositionManager::GetPlanePosition ( unsigned int ID )
{
  if ( ID < m_PositionList.size() )
  {
    return m_PositionList.at(ID);
  }
  else
  {
      MITK_WARN<<"GetPlanePosition returned NULL!";
      return 0;
  }
}

unsigned int mitk::PlanePositionManager::GetNumberOfPlanePositions()
{
  return m_PositionList.size();
}

void mitk::PlanePositionManager::RemoveAllPlanePositions()
{
  m_PositionList.clear();
}

void mitk::PlanePositionManager::SetDataStorage( mitk::DataStorage* ds )
{
  if (ds == NULL)
    return;

  /* remove listeners of old DataStorage */
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->RemoveNodeEvent.RemoveListener(MessageDelegate1<PlanePositionManager, const mitk::DataNode*>( this, &PlanePositionManager::DataStorageRemovedNode ));
  }
  /* register listener for new DataStorage */
  m_DataStorage = ds;  // register 
  m_DataStorage->RemoveNodeEvent.AddListener(MessageDelegate1<PlanePositionManager, const mitk::DataNode*>( this, &PlanePositionManager::DataStorageRemovedNode ));
}

void mitk::PlanePositionManager::DataStorageRemovedNode(const mitk::DataNode* removedNode)
{
  bool isContourMarker (false);
  if (removedNode->GetBoolProperty("isContourMarker", isContourMarker))
  {
    unsigned int t = removedNode->GetName().find_last_of(" ");
    unsigned int id = atof(removedNode->GetName().substr(t+1).c_str());
    this->RemovePlanePosition(id-1);
  }
}

  
