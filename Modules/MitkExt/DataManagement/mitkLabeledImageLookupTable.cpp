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


#include "mitkLabeledImageLookupTable.h"
#include <vtkLookupTable.h>
#include <cstdlib>


/**
 * Default constructor. Protected to prevent "normal" creation
 */
mitk::LabeledImageLookupTable::LabeledImageLookupTable()
  : m_LevelWindow(128,256)
{
  if ( m_LookupTable == NULL )
  {
    itkWarningMacro("LookupTable is NULL, it should have been initialized by the default constructor of mitk::LookupTable");
    m_LookupTable = vtkLookupTable::New();
  }
  m_LookupTable->SetNumberOfTableValues(256);
  
  // set the background to black and fully transparent
  m_LookupTable->SetTableValue( 0, 0.0, 0.0, 0.0, 0.0 ); 
  
  // initialize the remaining 255 colors with random values and 
  // an alpha value of 1.0
  vtkFloatingPointType r, g, b;
  
  //
  // Initialize the random number generator with an arbitrary seed.
  // This way, the generated colors are random, but always the same...
  std::srand( 2 );
  for ( vtkIdType index = 1 ; index < 256 ; ++index )
  {
    GenerateRandomColor(r, g, b);
    m_LookupTable->SetTableValue(index, r, g, b);
  }
  
  // initialize the default level/window settings,
  // which can be accessed via GetLevelWindow();
  m_LevelWindow.SetDefaultRangeMinMax(0,255);
  m_LevelWindow.SetRangeMinMax(0,255);
  m_LevelWindow.SetWindowBounds(0,255);
  m_LevelWindow.SetFixed(true);
  
}

/**
 * Virtual destructor
 */
mitk::LabeledImageLookupTable::~LabeledImageLookupTable()
{
  
}


/**
 * Sets the color for a given label
 * @param label The pixel value used as a label in the image
 * @param r The red component of the rgba color value. Values sould be given in the range [0,1]
 * @param g The green component of the rgba color value. Values sould be given in the range [0,1] 
 * @param b The blue component of the rgba color value. Values sould be given in the range [0,1]
 * @param a The alpha component of the rgba color value. Values sould be given in the range [0,1]. Default is 1.
 */
void mitk::LabeledImageLookupTable::SetColorForLabel ( const mitk::LabeledImageLookupTable::LabelType& label, const vtkFloatingPointType& r, const vtkFloatingPointType& g, const vtkFloatingPointType& b, const vtkFloatingPointType a )
{
  if ( m_LookupTable == NULL )
  {
    itkWarningMacro("LookupTable is NULL, but it should have been initialized by the constructor");
    return;
  }  
  m_LookupTable->SetTableValue(label, r, g, b, a);
}



/**
 * Determines the color which will be used for coloring a given label.
 * @param label the label for which the color should be returned
 * @returns an rgba array containing the color information for the given label
 *          Color components are expressed as [0,1] double values.
 */
vtkFloatingPointType* mitk::LabeledImageLookupTable::GetColorForLabel ( const mitk::LabeledImageLookupTable::LabelType& label )
{
  if ( m_LookupTable == NULL )
  {
    itkWarningMacro("LookupTable is NULL, but it should have been initialized by the constructor");
    return NULL;
  }
  return m_LookupTable->GetTableValue( label );
}


/**
 * Generates a random rgb color value. Values for rgb are in the range
 * [0,1]
 */
void mitk::LabeledImageLookupTable::GenerateRandomColor ( vtkFloatingPointType& r, vtkFloatingPointType& g, vtkFloatingPointType& b )
{
  r = GenerateRandomNumber();
  g = GenerateRandomNumber();
  b = GenerateRandomNumber();
}


/**
 * Generates a radnom number drawn from a uniform
 * distribution in the range [0,1].
 */
vtkFloatingPointType mitk::LabeledImageLookupTable::GenerateRandomNumber()
{
  return ( ( ( vtkFloatingPointType ) ( std::rand( ) ) ) / ( ( vtkFloatingPointType ) ( RAND_MAX ) ) );
}
