/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include <itkDataObject.h>

#include "mitkBaseProcess.h"
#include "mitkTimeGeometry.h"
#include <MitkCoreExports.h>
#include "mitkOperationActor.h"
#include "mitkPropertyList.h"

namespace mitk {

//class BaseProcess;

//##Documentation
//## @brief Base of all data objects
//##
//## Base of all data objects, e.g., images, contours, surfaces etc. Inherits
//## from itk::DataObject and thus can be included in a pipeline.
//## Inherits also from OperationActor and can be used as a destination for Undo
//## @ingroup Data
class MITK_CORE_EXPORT BaseData : public itk::DataObject, public OperationActor
{
public:
  mitkClassMacro(BaseData,itk::DataObject)

  /**
  * \brief Return the TimeGeometry of the data as const pointer.
  *
  * \warning No update will be called. Use GetUpdatedGeometry() if you cannot
  * be sure that the geometry is up-to-date.
  *
  * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
  */
  const mitk::TimeGeometry* GetTimeGeometry() const
  {
    return m_TimeGeometry.GetPointer();
  }

  /**
  * \brief Return the TimeGeometry of the data as const pointer.
  *
  * \warning No update will be called. Use GetUpdatedGeometry() if you cannot
  * be sure that the geometry is up-to-date.
  *
  * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
   * \deprecatedSince{2013_09} Please use GetTimeGeometry instead: For additional information see  http://www.mitk.org/Development/Refactoring%20of%20the%20Geometry%20Classes%20-%20Part%201
  */
  DEPRECATED(const mitk::TimeGeometry* GetTimeSlicedGeometry() const)
  {
    return GetTimeGeometry();
  }

  /**
  * @brief Return the TimeGeometry of the data as pointer.
  *
  * \warning No update will be called. Use GetUpdatedGeometry() if you cannot
  * be sure that the geometry is up-to-date.
  *
  * Normally used in GenerateOutputInformation of subclasses of BaseProcess.
  */
  mitk::TimeGeometry* GetTimeGeometry()
  {
    return m_TimeGeometry.GetPointer();
  }

  /**
  * @brief Return the TimeGeometry of the data.
  *
  * The method does not simply return the value of the m_TimeGeometry
  * member. Before doing this, it makes sure that the TimeGeometry
  * is up-to-date (by setting the update extent to largest possible and
  * calling UpdateOutputInformation).
  */
  const mitk::TimeGeometry* GetUpdatedTimeGeometry();

  /**
  * @brief Return the TimeGeometry of the data.
  *
  * The method does not simply return the value of the m_TimeGeometry
  * member. Before doing this, it makes sure that the TimeGeometry
  * is up-to-date (by setting the update extent to largest possible and
  * calling UpdateOutputInformation).
  * \deprecatedSince{2013_09} Please use GetUpdatedTimeGeometry instead: For additional information see http://www.mitk.org/Development/Refactoring%20of%20the%20Geometry%20Classes%20-%20Part%201
  */
  DEPRECATED(const mitk::TimeGeometry* GetUpdatedTimeSliceGeometry())
  {
    return GetUpdatedTimeGeometry();
  }

  /**
  * \brief Expands the TimeGeometry to a number of TimeSteps.
  *
  * The method expands the TimeGeometry to the given number of TimeSteps,
  * filling newly created elements with empty geometries. Sub-classes should override
  * this method to handle the elongation of their data vectors, too.
  * Note that a shrinking is neither possible nor intended.
  */
  virtual void Expand( unsigned int timeSteps );

  /**
  * \brief Return the BaseGeometry of the data at time \a t.
  *
  * The method does not simply return
  * m_TimeGeometry->GetGeometry(t).
  * Before doing this, it makes sure that the BaseGeometry is up-to-date
  * (by setting the update extent appropriately and calling
  * UpdateOutputInformation).
  *
  * @todo Appropriate setting of the update extent is missing.
  */
  const mitk::BaseGeometry* GetUpdatedGeometry(int t=0);

  //##Documentation
  //## @brief Return the geometry, which is a TimeGeometry, of the data
  //## as non-const pointer.
  //##
  //## \warning No update will be called. Use GetUpdatedGeometry() if you cannot
  //## be sure that the geometry is up-to-date.
  //##
  //## Normally used in GenerateOutputInformation of subclasses of BaseProcess.
  mitk::BaseGeometry* GetGeometry(int t=0) const
  {
    if(m_TimeGeometry.IsNull())
      return NULL;
    return m_TimeGeometry->GetGeometryForTimeStep(t);
  }

  //##Documentation
  //## @brief Update the information for this BaseData (the geometry in particular)
  //## so that it can be used as an output of a BaseProcess.
  //##
  //## This method is used in the pipeline mechanism to propagate information and
  //## initialize the meta data associated with a BaseData. Any implementation
  //## of this method in a derived class is assumed to call its source's
  //## BaseProcess::UpdateOutputInformation() which determines modified
  //## times, LargestPossibleRegions, and any extra meta data like spacing,
  //## origin, etc. Default implementation simply call's it's source's
  //## UpdateOutputInformation().
  //## \note Implementations of this methods in derived classes must take care
  //## that the geometry is updated by calling
  //## GetTimeGeometry()->UpdateInformation()
  //## \em after calling its source's BaseProcess::UpdateOutputInformation().
  void UpdateOutputInformation();

  //##Documentation
  //## @brief Set the RequestedRegion to the LargestPossibleRegion.
  //##
  //## This forces a filter to produce all of the output in one execution
  //## (i.e. not streaming) on the next call to Update().
  virtual void SetRequestedRegionToLargestPossibleRegion()=0;

  //##Documentation
  //## @brief Determine whether the RequestedRegion is outside of the BufferedRegion.
  //##
  //## This method returns true if the RequestedRegion
  //## is outside the BufferedRegion (true if at least one pixel is
  //## outside). This is used by the pipeline mechanism to determine
  //## whether a filter needs to re-execute in order to satisfy the
  //## current request.  If the current RequestedRegion is already
  //## inside the BufferedRegion from the previous execution (and the
  //## current filter is up to date), then a given filter does not need
  //## to re-execute
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion()=0;

  //##Documentation
  //## @brief Verify that the RequestedRegion is within the LargestPossibleRegion.
  //##
  //## If the RequestedRegion is not within the LargestPossibleRegion,
  //## then the filter cannot possibly satisfy the request. This method
  //## returns true if the request can be satisfied (even if it will be
  //## necessary to process the entire LargestPossibleRegion) and
  //## returns false otherwise.  This method is used by
  //## PropagateRequestedRegion().  PropagateRequestedRegion() throws a
  //## InvalidRequestedRegionError exception if the requested region is
  //## not within the LargestPossibleRegion.
  virtual bool VerifyRequestedRegion() = 0;

  //##Documentation
  //## @brief Copy information from the specified data set.
  //##
  //## This method is part of the pipeline execution model. By default, a
  //## BaseProcess will copy meta-data from the first input to all of its
  //## outputs. See ProcessObject::GenerateOutputInformation().  Each
  //## subclass of DataObject is responsible for being able to copy
  //## whatever meta-data it needs from another DataObject.
  //## The default implementation of this method copies the time sliced geometry
  //## and the property list of an object. If a subclass overrides this
  //## method, it should always call its superclass' version.
  void CopyInformation(const itk::DataObject* data);

  //##Documentation
  //## @brief Check whether the data has been initialized, i.e.,
  //## at least the Geometry and other header data has been set
  //##
  //## \warning Set to \a true by default for compatibility reasons.
  //## Set m_Initialized=false in constructors of sub-classes that
  //## support distinction between initialized and uninitialized state.
  virtual bool IsInitialized() const;

  //##Documentation
  //## @brief Calls ClearData() and InitializeEmpty();
  //## \warning Only use in subclasses that reimplemented these methods.
  //## Just calling Clear from BaseData will reset an object to a not initialized,
  //## invalid state.
  virtual void Clear();

  //##Documentation
  //## @brief Check whether object contains data (at
  //## a specified time), e.g., a set of points may be empty
  //##
  //## \warning Returns IsInitialized()==false by default for
  //## compatibility reasons. Override in sub-classes that
  //## support distinction between empty/non-empty state.
  virtual bool IsEmptyTimeStep(unsigned int t) const;

  //##Documentation
  //## @brief Check whether object contains data (at
  //## least at one point in time), e.g., a set of points
  //## may be empty
  //##
  //## \warning Returns IsInitialized()==false by default for
  //## compatibility reasons. Override in sub-classes that
  //## support distinction between empty/non-empty state.
  virtual bool IsEmpty() const;

  //##Documentation
  //## @brief Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.
  //##
  //## This method is implemented in the concrete subclasses of BaseData.
  virtual void SetRequestedRegion(const itk::DataObject *data)=0;

  //##Documentation
  //##@brief overwrite if the Data can be called by an Interactor (StateMachine).
  //##
  //## Empty by default. Overwrite and implement all the necessary operations here
  //## and get the necessary information from the parameter operation.
  void ExecuteOperation(Operation* operation);

  /**
  * \brief Set the BaseGeometry of the data, which will be referenced (not copied!).
  * Assumes the data object has only 1 time step ( is a 3D object ) and creates a
  * new TimeGeometry which saves the given BaseGeometry. If an TimeGeometry has already
  * been set for the object, it will be replaced after calling this function.
  *
  * @warning This method will normally be called internally by the sub-class of BaseData
  * during initialization.
  * \sa SetClonedGeometry
  */
  virtual void SetGeometry(BaseGeometry* aGeometry3D);

  /**
  * \brief Set the TimeGeometry of the data, which will be referenced (not copied!).
  *
  * @warning This method will normally be called internally by the sub-class of BaseData
  * during initialization.
  * \sa SetClonedTimeGeometry
  */
  virtual void SetTimeGeometry (TimeGeometry* geometry);

  /**
  * \brief Set a clone of the provided TimeGeometry as TimeGeometry of the data.
  * Assumes the data object has only 1 time step ( is a 3D object ) and
  * creates a new TimeGeometry. If an TimeGeometry has already
  * been set for the object, it will be replaced after calling this function.
  *
  * \sa SetGeometry
  */
  virtual void SetClonedGeometry(const BaseGeometry* aGeometry3D);

    /**
  * \brief Set a clone of the provided TimeGeometry as TimeGeometry of the data.
  *
  * \sa SetGeometry
  */
  virtual void SetClonedTimeGeometry (const TimeGeometry* geometry);

  //##Documentation
  //## @brief Set a clone of the provided geometry as BaseGeometry of a given time step.
  //##
  //## \sa SetGeometry
  virtual void SetClonedGeometry(const BaseGeometry* aGeometry3D, unsigned int time);

  //##Documentation
  //## @brief Get the data's property list
  //## @sa GetProperty
  //## @sa m_PropertyList
  mitk::PropertyList::Pointer GetPropertyList() const;

  //##Documentation
  //## @brief Set the data's property list
  //## @sa SetProperty
  //## @sa m_PropertyList
  void SetPropertyList(PropertyList* propertyList);

  //##Documentation
  //## @brief Get the property (instance of BaseProperty) with key @a propertyKey from the PropertyList,
  //## and set it to this, respectively;
  //## @sa GetPropertyList
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  mitk::BaseProperty::Pointer GetProperty(const char *propertyKey) const;

  void SetProperty(const char *propertyKey, BaseProperty* property);

  //##Documentation
  //## @brief Convenience method for setting the origin of
  //## the BaseGeometry instances of all time steps
  //##
  //## \warning Geometries contained in the BaseGeometry will
  //## \em not be changed, e.g. in case the BaseGeometry is a
  //## SlicedGeometry3D the origin will \em not be propagated
  //## to the contained slices. The sub-class SlicedData
  //## does this for the case that the SlicedGeometry3D is
  //## evenly spaced.
  virtual void SetOrigin(const Point3D& origin);

  /** \brief Get the process object that generated this data object.
   *
   * If there is no process object, then the data object has
   * been disconnected from the pipeline, or the data object
   * was created manually. (Note: we cannot use the GetObjectMacro()
   * defined in itkMacro because the mutual dependency of
   * DataObject and ProcessObject causes compile problems. Also,
   * a forward reference smart pointer is returned, not a smart pointer,
   * because of the circular dependency between the process and data object.)
   *
   * GetSource() returns a SmartPointer and not a WeakPointer
   * because it is assumed the code calling GetSource() wants to hold a
   * long term reference to the source. */
  itk::SmartPointer<mitk::BaseDataSource> GetSource() const;

  //##Documentation
  //## @brief Get the number of time steps from the TimeGeometry
  //## As the base data has not a data vector given by itself, the number
  //## of time steps is defined over the time sliced geometry. In sub classes,
  //## a better implementation could be over the length of the data vector.
  unsigned int GetTimeSteps() const
  {
    return m_TimeGeometry->CountTimeSteps();
  }


  //##Documentation
  //## @brief Get the modified time of the last change of the contents
  //## this data object or its geometry.
  virtual unsigned long GetMTime() const;

  /**
   * \sa itk::ProcessObject::Graft
   */
  virtual void Graft(const DataObject*);

protected:
  BaseData();
  BaseData(const BaseData &other);
  ~BaseData();

  //##Documentation
  //## \brief Initialize the TimeGeometry for a number of time steps.
  //## The TimeGeometry is initialized empty and evenly timed.
  //## In many cases it will be necessary to overwrite this in sub-classes.
  virtual void InitializeTimeGeometry( unsigned int timeSteps = 1 );

  /**
  * \brief Initialize the TimeGeometry for a number of time steps.
  * The TimeGeometry is initialized empty and evenly timed.
  * In many cases it will be necessary to overwrite this in sub-classes.
  * \deprecatedSince{2013_09} Please use GetUpdatedTimeGeometry instead: For additional information see http://www.mitk.org/Development/Refactoring%20of%20the%20Geometry%20Classes%20-%20Part%201
  */
  DEPRECATED(virtual void InitializeTimeSlicedGeometry( unsigned int timeSteps = 1 ))
  {
    InitializeTimeGeometry(timeSteps);
  }

  //##Documentation
  //## @brief reset to non-initialized state, release memory
  virtual void ClearData();

  //##Documentation
  //## @brief Pure virtual; Must be used in subclasses to get a data object to a
  //## valid state. Should at least create one empty object and call
  //## Superclass::InitializeTimeGeometry() to ensure an existing valid geometry
  virtual void InitializeEmpty(){}


  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  bool m_LastRequestedRegionWasOutsideOfTheBufferedRegion;

  mutable unsigned int m_SourceOutputIndexDuplicate;

  bool m_Initialized;

private:

  //##Documentation
  //## @brief PropertyList, f.e. to hold pic-tags, tracking-data,..
  //##
  PropertyList::Pointer m_PropertyList;

  TimeGeometry::Pointer m_TimeGeometry;

};

} // namespace mitk


#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
