/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:31:16 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITK_OdfScaleByProperty__H_
#define _MITK_OdfScaleByProperty__H_

#include "MitkDiffusionImagingExports.h"
#include "mitkEnumerationProperty.h"

namespace mitk
{

  enum OdfScaleBy
  {
    ODFSB_NONE, 
    ODFSB_GFA, 
    ODFSB_PC
  };

/**
 * Encapsulates the enumeration for ODF normalization. Valid values are 
 *     ODFSB_NONE, ODFSB_GFA, ODFSB_PC
 * Default is ODFSB_NONE
 */
class MitkDiffusionImaging_EXPORT OdfScaleByProperty : public EnumerationProperty
{
public:

  mitkClassMacro( OdfScaleByProperty, EnumerationProperty );
  itkNewMacro(OdfScaleByProperty);
  
  mitkNewMacro1Param(OdfScaleByProperty, const IdType&);
  
  mitkNewMacro1Param(OdfScaleByProperty, const std::string&);
  
  /**
   * Returns the current interpolation value as defined by VTK constants.
   */
  virtual int GetScaleBy();
  
protected:
  
  /** Sets reslice interpolation mode to default (ODFSB_NONE).
   */
  OdfScaleByProperty( );
  
  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfScaleByProperty( const IdType& value );
  
  /**
   * Constructor. Sets reslice interpolation to the given value.
   */
  OdfScaleByProperty( const std::string& value );

  /**
   * this function is overridden as protected, so that the user may not add
   * additional invalid interpolation types.
   */
  virtual bool AddEnum( const std::string& name, const IdType& id );

  /**
   * Adds the enumeration types as defined by vtk to the list of known 
   * enumeration values.
   */
  virtual void AddInterpolationTypes();
};

} // end of namespace mitk

#endif


