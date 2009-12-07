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


#ifndef MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_
#define MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_

#include <itkProcessObject.h>
#include "mitkNavigationData.h"
#include "mitkPropertyList.h"

namespace mitk {

  /**Documentation
  * \brief Navigation Data source
  *
  * Base class for all navigation filters that produce NavigationData objects as output.
  * This class defines the output-interface for NavigationDataFilters. 
  * \warning: if Update() is called on any output object, all NavigationData filters will
  * generate new output data for all outputs, not just the one on which Update() was called.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataSource : public itk::ProcessObject
  {
  public:
    mitkClassMacro(NavigationDataSource, itk::ProcessObject);

    /**
    *\brief return the output (output with id 0) of the filter
    */
    NavigationData* GetOutput(void);
    
    /** 
    *\brief return the output with id idx of the filter
    */
    NavigationData* GetOutput(unsigned int idx);

    /** 
    *\brief return the output with name navDataName of the filter
    */
    NavigationData* GetOutput(std::string navDataName);

    /** 
    *\brief return the index of the output with name navDataName, -1 if no output with that name was found
    *
    * \warning if a subclass has outputs that have different data type than mitk::NavigationData, they have to overwrite this method
    */
    DataObjectPointerArraySizeType GetOutputIndex(std::string navDataName);

    /** 
    *\brief Graft the specified DataObject onto this ProcessObject's output.
    * 
    * See itk::ImageSource::GraftNthOutput for details
    */
    virtual void GraftNthOutput(unsigned int idx, itk::DataObject *graft);

    /** 
    * \brief Graft the specified DataObject onto this ProcessObject's output.
    * 
    * See itk::ImageSource::Graft Output for details
    */
    virtual void GraftOutput(itk::DataObject *graft);

    /**
    * \brief Make a DataObject of the correct type to used as the specified output.
    * 
    * This method is automatically called when DataObject::DisconnectPipeline() 
    * is called.  DataObject::DisconnectPipeline, disconnects a data object
    * from being an output of its current source.  When the data object
    * is disconnected, the ProcessObject needs to construct a replacement
    * output data object so that the ProcessObject is in a valid state.
    * Subclasses of NavigationDataSource that have outputs of different
    * data types must overwrite this method so that proper output objects
    * are created.
    */
    virtual DataObjectPointer MakeOutput(unsigned int idx);

    /** 
    * \brief Set all filter parameters as the PropertyList p
    *
    * This method allows to set all parameters of a filter with one
    * method call. For the names of the parameters, take a look at 
    * the GetParameters method of the filter
    * This method has to be overwritten by each MITK-IGT filter.
    */
    virtual void SetParameters(const mitk::PropertyList*){};

    /** 
    * \brief Get all filter parameters as a PropertyList
    *
    * This method allows to get all parameters of a filter with one
    * method call. The returned PropertyList must be assigned to a 
    * SmartPointer immediately, or else it will get destroyed.
    * Every filter must overwrite this method to create a filter-specific 
    * PropertyList. Note that property names must be unique over all 
    * MITK-IGT filters. Therefore each filter should use its name as a prefix
    * for each property name.
    * Secondly, each filter should list the property names and data types 
    * in the method documentation.
    */
    virtual mitk::PropertyList::ConstPointer GetParameters() const;

  protected:
    NavigationDataSource();
    virtual ~NavigationDataSource();
  };
} // namespace mitk
#endif /* MITKNAVIGATIONDATASOURCE_H_HEADER_INCLUDED_ */
