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
//#include <itkPoint.h>
#include <itkPointSet.h>
//#include <itkCovariantVector.h>
#include <itkMesh.h>
#include <itkDefaultDynamicMeshTraits.h>
#include <itkCommand.h> //for the PointSetObserver
//#include <itkPolygonCell.h>
//#include <itkEventObject.h>

const unsigned int PointDimension = 3;
const unsigned int MaxTopologicalDimension = 3;


namespace mitk {

//##Documentation
//##@brief DataStructure which stores a set of points. Superclass of mitk::Mesh.
//##
//## This class supports 3D point definition (not yet 3D+t).
//## An entry is divided into the pointcoordinates and pointdata.
//## The pointdata includes information about the pointId, the selection state of the point and an information about the type of the point.
//## For further information about different types of a point see mitk::PointSpecificationType in mitkVector.h;
//## Inserting a point is accompanied by an event, containing an index. The new point is set into the list 
//## at the indexed position. At the same time an internal ID is set in data of the point.
//##
//## The points of itk::PointSet stores the points in a pointContainer = MapContainer. To read the points, do it with an constIteractor of MapContainer and not with size.
//## That way all the indexed points are iterated and the points, that are deleted by deleting the index aren't shown.
//##
//## The class uses an itkMesh internaly, because mitk::Mesh is derived from mitk::PointSet and needs the itk::Mesh structure which is also derived from itk::PointSet.
//## Thus several typedefs, that seem to be in wrong place, are declared here (for example SelectedLinesType).
//##
//## \subsection mitkPointSetDisplayOptions
//## 
//## The default mappers for this data structure are mitk::PointSetMapper2D and mitk::PointSetVtkMapper3D. See these classes for display options, that can be
//## set via properties.
//##
//## @ingroup Data
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
  typedef DataType::PointIdentifier PointIdentifier;
  typedef DataType::PointsContainer PointsContainer;
  typedef DataType::PointsContainerIterator PointsIterator;
  typedef DataType::PointsContainer::ConstIterator PointsConstIterator;
  typedef DataType::PointDataContainer PointDataContainer;
  typedef DataType::PointDataContainerIterator PointDataIterator;

  //##Documentation
  //## @brief executes the given Operation
  virtual void ExecuteOperation(Operation* operation);

  //##Documentation
  //## @brief returns the current size of the point-list
  virtual int GetSize() const;

  //##Documentation
  //## @brief returns the pointset
  virtual DataType::Pointer GetPointSet() const;

  //##Documentation
  //## @brief Get the point on the given position in world coordinates
  //##
  //## check if index exists. If it doesn't exist, then return 0,0,0
  PointType GetPoint(int position) const;

  /**
  * @brief If the Id exists in mitkData, then point is set and true is returned in world coordinates
  **/
  bool GetPointIfExists(PointIdentifier id, PointType* point);

  /**
  * @brief Set the given point in world coordinate system into the itkPointSet.
  **/
  void SetPoint(PointIdentifier id, PointType point);

  /**
  * @brief Set the given point in world coordinate system into the itkPointSet.
  **/
  void InsertPoint(PointIdentifier id, PointType point);


  //##Documentation
  //## @brief searches a selected point and returns the id of that point. 
  //## If no point is found, then -1 is returned
  virtual int SearchSelectedPoint();

  //##Documentation
  //## @brief returns true if a point exists at this position
  virtual bool IndexExists(int position);

  //##Documentation
  //## @brief to get the state selected/unselected of the point on the position
  virtual bool GetSelectInfo(int position);

  //##Documentation
  //## @brief returns the number of selected points
  virtual const int GetNumberOfSelected();

  //##Documentation
  //## @brief searches a point in the list == point +/- distance
  //##
  //## @param point is in world coordinates.
  //## @param distance is in mm.
  //## returns -1 if no point is found
  //## or the position in the list of the first match
  //'' Beware when using the position to 
  int SearchPoint(Point3D point, float distance);

  //virtual methods, that need to be implemented
  virtual void UpdateOutputInformation();
  virtual void SetRequestedRegionToLargestPossibleRegion();
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  virtual void SetRequestedRegion(itk::DataObject *data);
  virtual bool WriteXMLData( XMLWriter& xmlWriter );
  virtual bool ReadXMLData( XMLReader& xmlReader );

  //Method for subclasses
  virtual void OnPointSetChange(){};

protected:
  PointSet();

  virtual ~PointSet();

  //##Documentation
  //## @brief Data from ITK; List of Points; the object, the operations are ment for
  DataType::Pointer m_ItkData;


};

  itkEventMacro( NewPointEvent, itk::AnyEvent );
  itkEventMacro( RemovedPointEvent, itk::AnyEvent );



/**
* @brief Class to realize an observer, that listens to an event called by mitkPointSet
* A pointer to a method of an other object, not derived from itk can be connected and called 
**/
template <class T> 
class TPointSetObserver : public itk::Command 
  {
  public:
    typedef  TPointSetObserver        Self;
    typedef  itk::Command             Superclass;
    typedef  itk::SmartPointer<Self>  Pointer;

    itkNewMacro( Self );

  protected:
    TPointSetObserver(){}
    ~TPointSetObserver(){}

  public:
    /**
    * @brief Set the object and its methods, that have to be called if an event is recieved
    **/
    void Set( T* object, void (T::*memberFunctionPointerNewPointEvent)(), void (T::*memberFunctionPointerRemovedPointEvent)() ) 
    {
      m_Object = object; 
      m_MemberFunctionPointerNewPointEvent = memberFunctionPointerNewPointEvent;
      m_MemberFunctionPointerRemovedPointEvent = memberFunctionPointerRemovedPointEvent;
    }
    /**
    * @brief Set the parent, which methods are called
    **/
    void SetParent( T* object) {m_Object = object;}
    /**
    * @brief Set the method to be called if the pointset throughs a NewPointEvent()
    **/
    void SetNewPointEventMethod( void (T::*memberFunctionPointerNewPointEvent)() ) {m_MemberFunctionPointerNewPointEvent = memberFunctionPointerNewPointEvent;}
    /**
    * @brief Set the method to be called if the pointset throughs a RemovedPointEvent()
    **/
    void SetRemovedPointEventMethod( void (T::*memberFunctionPointerRemovedPointEvent)() ) {m_MemberFunctionPointerRemovedPointEvent = memberFunctionPointerRemovedPointEvent;}
    /**
    * @brief Derive methods to call the registered method if an event is revieved
    **/
    void Execute(itk::Object *object, const itk::EventObject & event)
    {Execute( (const itk::Object*) object, event );}
    void Execute(const itk::Object * object, const itk::EventObject & event)
    {
      if ( typeid(event) == typeid(mitk::NewPointEvent) )
        (*m_Object.*m_MemberFunctionPointerNewPointEvent)();
      if ( typeid(event) == typeid(mitk::RemovedPointEvent) )
        (*m_Object.*m_MemberFunctionPointerRemovedPointEvent)();
    }
  private: 
    T* m_Object;
    void (T::*m_MemberFunctionPointerNewPointEvent)();
    void (T::*m_MemberFunctionPointerRemovedPointEvent)();
  };

} // namespace mitk

#endif /* MITKPointSet_H_HEADER_INCLUDED */
