#ifndef MITKPointSet_H_HEADER_INCLUDED
#define MITKPointSet_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include <mitkBaseData.h>
#include <itkPoint.h>
#include <itkPointSet.h>
#include <itkCovariantVector.h>
#include <itkMesh.h>
#include <itkDefaultDynamicMeshTraits.h>
  
const unsigned int Dimension = 3;

namespace mitk {

//##ModelId=3F0177E803A1
//##Documentation
//##@brief DataStructure which stores a list of Points 
//## @ingroup Geometry
//##
//## Each Entry (Point) in the the PointSet has a additional value (bool) to store
//## the selected/unselected state of the Entry(Point).
class PointSet : public BaseData
{
public:
  mitkClassMacro(PointSet, BaseData);

  itkNewMacro(Self);

  typedef mitk::ScalarType PixelType;
  typedef itk::DefaultDynamicMeshTraits<bool, 3, 3, mitk::ScalarType> MeshTraits;
  typedef itk::Mesh<PixelType, Dimension, MeshTraits> MeshType;  
  typedef MeshType DataType;

  typedef DataType::PointType PointType;
  typedef DataType::PointsContainer PointsContainer;
  typedef DataType::PointsContainerIterator PointsIterator;
  typedef DataType::PointDataContainer PointDataContainer;
  typedef DataType::PointDataContainerIterator PointDataIterator;
  

  //##ModelId=3F0177E901BF
  //##Documentation
  //## @brief executes the given Operation
	virtual void ExecuteOperation(Operation* operation);

  //##ModelId=3F0177E901C1
  //##Documentation
	//## @brief returns the current size of the point-list
	virtual int GetSize();

  //##ModelId=3F0177E901CC
  //##Documentation
	//## @brief returns the point-list with points and selected Information
  DataType* GetPointSet() const;

  //##ModelId=3F0177E901CE
  //##Documentation
	//## @brief Get the point on the given position
  //##
  //## check if index exists. If it doesn't exist, then return 0,0,0
	const PointType GetPoint(int position);

  //##Documentation
	//## @brief Get the point on the given position in itkPoint3D
  //##
  //## check if index exists. If it doesn't exist, then return 0,0,0
  const mitk::ITKPoint3D GetItkPoint(int position);

  //##Documentation
	//## @brief returns true if a point exists at this position
  virtual bool IndexExists(int position);

  //##ModelId=3F0177E901DC
	//##Documentation
	//## @brief to get the state selected/unselected of the point on the position
	virtual bool GetSelectInfo(int position);

  //##ModelId=3F05B07B0147
  //##Documentation
	//## @brief returns the number of selected points
	virtual const int GetNumberOfSelected();

  //##ModelId=3F0177E901DE
	//##Documentation
	//## @brief searches a point in the List == point +/- distance
	//##
	//## returns -1 if no point is found
	//## or the position in the list of the first match
	int SearchPoint(ITKPoint3D point, float distance);

	//virtual methods, that need to be implemented
  //##ModelId=3F0177E901EE
	virtual void UpdateOutputInformation();
  //##ModelId=3F0177E901FB
	virtual void SetRequestedRegionToLargestPossibleRegion();
  //##ModelId=3F0177E901FD
	virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  //##ModelId=3F0177E901FF
	virtual bool VerifyRequestedRegion();
  //##ModelId=3F0177E9020B
	virtual void SetRequestedRegion(itk::DataObject *data);

protected:
  //##ModelId=3F0177E901BD
	PointSet();

  //##ModelId=3F0177E901BE
	virtual ~PointSet();

  //##ModelId=3F0177E90190
	//##Documentation
	//## @brief Data from ITK; List of Points; the object, the operations are ment for
	DataType::Pointer m_ItkData;
};

} // namespace mitk



#endif /* MITKPointSet_H_HEADER_INCLUDED */
