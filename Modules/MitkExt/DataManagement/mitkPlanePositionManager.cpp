#include "mitkPlanePositionManager.h"


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

bool mitk::PlanePositionManager::AddNewPosition ( RestorePlanePositionOperation *op )
{
  //std::ofstream file("C:/Users/fetzer/Desktop/equationSystem/geometryfile.txt");
  ///*MITK_INFO*/file <<"X = "<<op->GetXAxis()<<" Y = "<< op->GetYAxis()<<" Pos = "<<op->GetPos()<<" Origin = "<<op->GetOrigin();

  for (unsigned int i = 0; i < m_PositionList.size(); i++)
  {
    ///*MITK_INFO*/file <<"X = "<<m_PositionList.at(i)->GetXAxis()<<" Y = "<< m_PositionList.at(i)->GetYAxis()<<" Pos = "<<m_PositionList.at(i)->GetPos()<<" Origin = "<<m_PositionList.at(i)->GetOrigin();
    if ( m_PositionList.at(i) != 0 && m_PositionList.at(i)->GetPos() == op->GetPos() && m_PositionList.at(i)->GetTransform()->GetMatrix() == op->GetTransform()->GetMatrix()
        &&  m_PositionList.at(i)->GetTransform()->GetOffset() == op->GetTransform()->GetOffset() )
      return false;
  }
  m_PositionList.push_back( op );
  return true;
}

bool mitk::PlanePositionManager::DeletePlanePosition( unsigned int ID )
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
  if ( m_PositionList.at(ID) != 0 )
  {
    return m_PositionList.at(ID);
  }
  else
  {
    MITK_INFO<<"GetPlanePosition returned NULL";
    return 0;
  }
}

unsigned int mitk::PlanePositionManager::GetNumberOfPlanePositions()
{
  return m_PositionList.size();
}

void mitk::PlanePositionManager::DeleteAllMarkers()
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
    unsigned int t = removedNode->GetName().find_last_of("_");
    unsigned int id = atof(removedNode->GetName().substr(t+1).c_str());
    this->DeletePlanePosition(id);
  }
}

  