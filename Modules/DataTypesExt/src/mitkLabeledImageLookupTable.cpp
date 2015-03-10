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
  double r, g, b;

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
  m_LevelWindow.SetRangeMinMax(0,255);
  m_LevelWindow.SetWindowBounds(0,255);
  m_LevelWindow.SetFixed(true);

}

mitk::LabeledImageLookupTable::LabeledImageLookupTable(const mitk::LabeledImageLookupTable &other)
  : LookupTable(other)
  , m_LevelWindow(other.m_LevelWindow)
{
}

/**
 * Virtual destructor
 */
mitk::LabeledImageLookupTable::~LabeledImageLookupTable()
{

}

mitk::LabeledImageLookupTable &mitk::LabeledImageLookupTable::operator=(const mitk::LookupTable& other)
{
  LookupTable::operator =(other);
  if (const LabeledImageLookupTable* lut = dynamic_cast<const LabeledImageLookupTable*>(&other))
  {
    this->m_LevelWindow = lut->m_LevelWindow;
  }
  return *this;
}


/**
 * Sets the color for a given label
 * @param label The pixel value used as a label in the image
 * @param r The red component of the rgba color value. Values sould be given in the range [0,1]
 * @param g The green component of the rgba color value. Values sould be given in the range [0,1]
 * @param b The blue component of the rgba color value. Values sould be given in the range [0,1]
 * @param a The alpha component of the rgba color value. Values sould be given in the range [0,1]. Default is 1.
 */
void mitk::LabeledImageLookupTable::SetColorForLabel ( const mitk::LabeledImageLookupTable::LabelType& label, const double& r, const double& g, const double& b, const double a )
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
double* mitk::LabeledImageLookupTable::GetColorForLabel ( const mitk::LabeledImageLookupTable::LabelType& label )
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
void mitk::LabeledImageLookupTable::GenerateRandomColor ( double& r, double& g, double& b )
{
  r = GenerateRandomNumber();
  g = GenerateRandomNumber();
  b = GenerateRandomNumber();
}


/**
 * Generates a radnom number drawn from a uniform
 * distribution in the range [0,1].
 */
double mitk::LabeledImageLookupTable::GenerateRandomNumber()
{
  return ( ( ( double ) ( std::rand( ) ) ) / ( ( double ) ( RAND_MAX ) ) );
}

itk::LightObject::Pointer mitk::LabeledImageLookupTable::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
