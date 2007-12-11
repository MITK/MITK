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


#ifndef MITKSEEDSIMAGELOOKUPTABLESOURCE_H_HEADER_INCLUDED
#define MITKSEEDSIMAGELOOKUPTABLESOURCE_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkLookupTableSource.h"


namespace mitk
{

class SeedsImageLookupTableSource : public LookupTableSource
{
public:
  typedef enum {Seeds, DefaultLUT, Force} LookupTableMode;

  mitkClassMacro( SeedsImageLookupTableSource, LookupTableSource );
  itkNewMacro( Self );
  
  /** @brief Some convenient typedefs. */
  typedef mitk::LookupTable OutputType;

  void SetUseSeedsLookupTable() {m_Mode = Seeds; this->Modified();};

  void SetUseForceLookupTable() {m_Mode = Force; this->Modified();};

protected:
  SeedsImageLookupTableSource();
  virtual ~SeedsImageLookupTableSource();

    /**
    * Generates a LookupTable depended on the given mode and stores
    * it in an output object
    */
  virtual void GenerateData();

  /**
    * Generates a lookup table, dependend on the given mode
    * @returns a vtkLookupTable
    */
  vtkLookupTable* BuildVtkLookupTable( );

  /**
    * Generates a Seeds lookup table
    * @returns a vtkLookupTable
    */
  vtkLookupTable* BuildSeedsLookupTable();

  /**
    * Generates a force lookup table
    * @returns a vtkLookupTable
    */
  vtkLookupTable* BuildForceLookupTable();

  /**
    * Generates a default lookup table
    * @returns a vtkLookupTable
    */
  vtkLookupTable* BuildDefaultLookupTable();
    
private:
  LookupTableMode m_Mode;
  OutputTypePointer m_LookupTable;
};

} // namespace mitk

#endif /* MITKSEEDSIMAGELOOKUPSOURCE_H_HEADER_INCLUDED */
