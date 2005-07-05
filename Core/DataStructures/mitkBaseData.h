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


#ifndef BASEDATA_H_HEADER_INCLUDED_C1EBB6FA
#define BASEDATA_H_HEADER_INCLUDED_C1EBB6FA

#include <itkDataObject.h>
#include "mitkGeometry3D.h"
#include "mitkCommon.h"
#include "mitkOperationActor.h"
#include "mitkXMLIO.h"
#include "mitkPropertyList.h"
#include <map>

namespace mitk {

class BaseProcess;

//##ModelId=3D6A0D7D00EC
//##Documentation
//## @brief Base of all data objects
//## @ingroup Data
//## Base of all data objects, e.g., images, contours, surfaces etc. Inherits
//## from itk::DataObject and thus can be included in a pipeline.
//## Inherits also from OperationActor and can be used as a destination for Undo
class BaseData : public itk::DataObject, public OperationActor, public XMLIO
{
public:
  //##ModelId=3E10262200CE
  typedef Geometry3D::Pointer Geometry3DPointer;

  mitkClassMacro(BaseData,itk::DataObject)

  //##ModelId=3DCBE2BA0139
  //##Documentation
  //## @brief Return the Geometry3D of the data. 
  //## 
  //## The method does not simply return the value of the m_Geometry3D member. 
  //## Before doing this, it makes sure that the Geometry3D is up-to-date before 
  //## returning it (by setting the update extent appropriately and calling
  //## UpdateOutputInformation).
  //## 
  //## @warning GetGeometry not yet completely implemented. 
  //## @todo Appropriate setting of the update extent is missing.
  const mitk::Geometry3D* GetUpdatedGeometry();

  //##Documentation
  //## @brief Return the Geometry3D of the data as non-const pointer. 
  //## 
  //## @em No update will be called. Normally used in GenerateOutputInformation of 
  //## subclasses of BaseProcess.
  mitk::Geometry3D* GetGeometry() const;

  //##ModelId=3E8600DB0188
  //##Documentation
  //## @brief Helps to deal with the weak-pointer-problem.
  virtual void UnRegister() const;

  //##ModelId=3E8600DB02DC
  //##Documentation
  //## @brief for internal use only. Helps to deal with the
  //## weak-pointer-problem.
  virtual int GetExternalReferenceCount() const;

  //##ModelId=3EDD06DC017A
  //##Documentation 
  //## @brief Update the information for this BaseData so that it can be used
  //## as an output of a BaseProcess. 
  //## 
  //## This method is used in the pipeline mechanism to propagate information and 
  //## initialize the meta data associated with a BaseData. Any implementation 
  //## of this method in a derived class is assumed to call its source's
  //## BaseProcess::UpdateOutputInformation() which determines modified
  //## times, LargestPossibleRegions, and any extra meta data like spacing,
  //## origin, etc. 
  void UpdateOutputInformation()=0;

  //##ModelId=3EDD06DC035E
  //##Documentation 
  //## @brief Set the RequestedRegion to the LargestPossibleRegion.
  //## 
  //## This forces a filter to produce all of the output in one execution
  //## (i.e. not streaming) on the next call to Update(). 
  void SetRequestedRegionToLargestPossibleRegion()=0;

  //##ModelId=3EDD06DD017A
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
  bool RequestedRegionIsOutsideOfTheBufferedRegion()=0;

  //##ModelId=3EDD09370191
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

  //##ModelId=3EDD06DE0274
  //##Documentation 
  //## @brief Copy information from the specified data set.
  //## 
  //## This method is part of the pipeline execution model. By default, a 
  //## BaseProcess will copy meta-data from the first input to all of its
  //## outputs. See ProcessObject::GenerateOutputInformation().  Each
  //## subclass of DataObject is responsible for being able to copy
  //## whatever meta-data it needs from from another DataObject.
  //## The default implementation of this method is empty. If a subclass
  //## overrides this method, it should always call its superclass'
  //## version. 
  void CopyInformation(const itk::DataObject*) {};

  //##ModelId=3EDD06DF017A
  //##Documentation
  //## @brief Set the requested region from this data object to match the requested
  //## region of the data object passed in as a parameter.
  //## 
  //## This method is implemented in the concrete subclasses of BaseData. 
  void SetRequestedRegion(itk::DataObject *data)=0;

  //##Documentation
  //##@brief overwrite if the Data can be called by an Interactor (StateMachine).
  //## 
  //## Empty by default. Overwrite and implement all the necessary operations here 
  //## and get the necessary information from the parameter operation.
  void ExecuteOperation(Operation* operation);

  //##Documentation
  //## @brief Set the Geometry3D of the data, which will be referenced (not copied!). 
  //## 
  //## @warning This method will normally be called internally by the sub-class of BaseData 
  //## during initialization.
  virtual void SetGeometry(Geometry3D* aGeometry3D);


  //##Documentation
  //## @brief Get the PropertyList 
  //## @sa GetProperty
  //## @sa m_PropertyList
  mitk::PropertyList::Pointer GetPropertyList() const;

  //##Documentation
  //## @brief Set the PropertyList
  //## @sa SetProperty
  //## @sa m_PropertyList
  void SetPropertyList(PropertyList* propertyList);
  
  //##ModelId=3EF189DB0111
  //##Documentation
  //## @brief Get the property (instance of BaseProperty) with key @a propertyKey from the PropertyList
  //## @sa GetPropertyList
  //## @sa m_PropertyList
  //## @sa m_MapOfPropertyLists
  mitk::BaseProperty::Pointer GetProperty(const char *propertyKey) const;

  void SetProperty(const char *propertyKey, BaseProperty* property);

  itk::SmartPointerForwardReference<mitk::BaseProcess> GetSource() const;

  //## 
  virtual bool WriteXML( XMLWriter& xmlWriter );

  //##
  virtual bool ReadXML( XMLReader& xmlReader );

  //##
  static BaseData* CreateObject( const char* className );

protected:
  //##ModelId=3E3FE04202B9
  BaseData();
  //##ModelId=3E3FE042031D
  ~BaseData();
  //##ModelId=3E15551A03CE
  Geometry3DPointer m_Geometry3D;

#if ITK_VERSION_MAJOR == 2 || ( ITK_VERSION_MAJOR == 1 && ITK_VERSION_MINOR > 6 )
  bool m_RequestedRegionInitialized;
  bool m_LastRequestedRegionWasOutsideOfTheBufferedRegion;
#endif

  //##ModelId=3E8600D9006D
  mutable itk::SmartPointer<mitk::BaseProcess> m_SmartSourcePointer;
  //##ModelId=3E8600D9021B
  mutable unsigned int m_SourceOutputIndexDuplicate;
  //##ModelId=3E8600DC0053
  //##Documentation
  //## @brief for internal use only. Helps to deal with the
  //## weak-pointer-problem.
  virtual void ConnectSource(itk::ProcessObject *arg, unsigned int idx) const;
private:
  //##ModelId=3E8600D90384
  //##Documentation
  //## @brief Helps to deal with the weak-pointer-problem.
  mutable bool m_Unregistering;
  //##ModelId=3E8600DA0118
  //##Documentation
  //## @brief Helps to deal with the weak-pointer-problem.
  mutable bool m_CalculatingExternalReferenceCount;
  //##ModelId=3E8600DA02B3
  //##Documentation
  //## @brief Helps to deal with the weak-pointer-problem.
  mutable int m_ExternalReferenceCount;

  //##Documentation
  //## @brief PropertyList, f.e. to hold pic-tags, tracking-data,..
  //##
  PropertyList::Pointer m_PropertyList;  

  //##Documentation
  //## @brief Helps to deal with the weak-pointer-problem.
  friend class mitk::BaseProcess;
};

} // namespace mitk


#endif /* BASEDATA_H_HEADER_INCLUDED_C1EBB6FA */
