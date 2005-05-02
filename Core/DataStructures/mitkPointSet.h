/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKPointSet_H_HEADER_INCLUDED
#define MITKPointSet_H_HEADER_INCLUDED

#include <mitkCommon.h>
#include <mitkVector.h>
#include <mitkBaseData.h>
#include <itkPoint.h>
#include <itkPointSet.h>
#include <itkCovariantVector.h>
#include <itkMesh.h>
#include <itkDefaultDynamicMeshTraits.h>
#include <itkPolygonCell.h>
#include <itkEventObject.h>

const unsigned int PointDimension = 3;
const unsigned int MaxTopologicalDimension = 3;


namespace mitk {

//##ModelId=3F0177E803A1
//##Documentation
//##@brief DataStructure which stores a set of points. Superclass of mitk::Mesh.
//## @ingroup Data
//##
//## This class supports 3D point definition.
//## An entry is divided into the pointcoordinates and pointdata.
//## The pointdata includes information about the pointId, the selection state of the point and an information about the type of the point.
//## For further information about different types of a point see mitk::PointSpecificationType in mitkVector.h;
//## Inserting a point is accompanied by an event, containing an index. The new point is set into ths list 
//## at the indexed position. At the same time an internal ID is set in data of the point.
//##
//## The class uses an itkMesh internaly, because mitk::Mesh is derived from mitk::PointSet and needs the itk::Mesh structure which is also derived from itk::PointSet.
//## Thus several typedefs, that seem to be in wrong place, are declared here (for example SelectedLinesType).
class PointSet : public BaseData
{
public:
  mitkClassMacro(PointSet, BaseData);

  itkNewMacro(Self);

  
  typedef mitk::ScalarType CoordinateType;
  typedef mitk::ScalarType InterpolationWeightType;
  //##Documentation
  //##@brief struct for data of a point
  typedef struct PointDataType 
  {
    unsigned int id;  //to give the point a special ID
    bool selected;  //information about if the point is selected
    mitk::PointSpecificationType pointSpec;  //specifies the type of the point
  };
  //##Documentation
  //##@brief cellDataType, that stores all indexes of the lines, that are selected
  //## e.g.: points A,B and C.Between A and B there is a line with index 0.
  //## if vector of cellData contains 1 and 2, then the lines between B and C and C and A is selected.
  typedef std::vector<unsigned int> SelectedLinesType;
  typedef SelectedLinesType::iterator SelectedLinesIter;
  typedef struct CellDataType 
  {
    bool selected; //used to set the whole cell on selected
    SelectedLinesType selectedLines;//indexes of selected lines. 0 is between pointId 0 and 1
    bool closed; //is the polygon already finished and closed
  };

  typedef itk::DefaultDynamicMeshTraits<
    PointDataType, PointDimension, MaxTopologicalDimension, 
    CoordinateType, InterpolationWeightType, CellDataType > MeshTraits;
  typedef itk::Mesh<PointDataType, PointDimension, MeshTraits> MeshType;  

  typedef MeshType DataType;
  typedef DataType::PointType PointType;
  typedef DataType::PointsContainer PointsContainer;
  typedef DataType::PointsContainerIterator PointsIterator;
  typedef DataType::PointsContainer::ConstIterator PointsConstIterator;
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
  //## @brief returns the pointset
  virtual DataType::Pointer GetPointSet() const;

  //##ModelId=3F0177E901CE
  //##Documentation
  //## @brief Get the point on the given position
  //##
  //## check if index exists. If it doesn't exist, then return 0,0,0
  PointType GetPoint(int position) const;

  //##Documentation
  //## @brief searches a selected point and returns the id of that point. if no point is found, then -1 is returned
  virtual int SearchSelectedPoint();

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
  int SearchPoint(Point3D point, float distance);

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

  itkEventMacro( NewPointEvent, itk::AnyEvent );
  itkEventMacro( RemovedPointEvent, itk::AnyEvent );
} // namespace mitk



#endif /* MITKPointSet_H_HEADER_INCLUDED */
