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

#ifndef _mitk_Point_Set_To_Geometry_Data_Filter__h_
#define _mitk_Point_Set_To_Geometry_Data_Filter__h_

#include <mitkPointSet.h>
#include <mitkGeometryDataSource.h>

namespace mitk
{

/**
 * Common superclass for all algorithms having one or more point
 * sets as input and producing one or more geometry datas as output
 */
class MITK_CORE_EXPORT PointSetToGeometryDataFilter : public GeometryDataSource
{
public:
  /**
   * Standard mitk class macro
   */
  mitkClassMacro( PointSetToGeometryDataFilter, GeometryDataSource );
  
  itkNewMacro( Self );
  
  /**
   * Common typdedefs for the input type and corresponding pointers
   */
  typedef mitk::PointSet InputType;
  typedef InputType::Pointer InputTypePointer;
  typedef InputType::ConstPointer InputTypeConstPointer;
  
  /**
   * Set and Get input methods.
   */
  virtual void SetInput( const InputType* input );
  virtual void SetInput( const unsigned int& idx, const InputType* input );
  virtual InputType* GetInput();
  virtual InputType* GetInput(const unsigned int& idx);
  
protected:
  
  /**
   * Protected constructor. Use ::New instead()
   */
  PointSetToGeometryDataFilter();
  
  /**
   * Protected destructor. Instances are destroyed when reference count is zero
   */
  virtual ~PointSetToGeometryDataFilter();
  
  
};  
    
} //end of namespace mitk


#endif


