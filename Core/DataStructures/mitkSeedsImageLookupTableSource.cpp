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


#include "mitkSeedsImageLookupTableSource.h"
#include <vtkLookupTable.h>


mitk::SeedsImageLookupTableSource::~SeedsImageLookupTableSource()
{}


mitk::SeedsImageLookupTableSource::SeedsImageLookupTableSource() : mitk::LookupTableSource()
{
  m_Mode = DefaultLUT;
  m_LookupTable = NULL;

  this->Modified();    
}


void mitk::SeedsImageLookupTableSource::GenerateData()
{
  OutputType::Pointer output = this->GetOutput();
  output->SetVtkLookupTable( this->BuildVtkLookupTable( ) );
}

vtkLookupTable* mitk::SeedsImageLookupTableSource::BuildSeedsLookupTable()
{
  vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
  vtkLookupTable->SetNumberOfTableValues( 256 );
  vtkLookupTable->SetTableRange( 0.0, 255.0 );

  // black
  int i;
  for ( i = 0; i < 253; i++ )
  {
    vtkLookupTable->SetTableValue ( i, 0.0, 0.0, 0.0, 0.0 );
  }

  // blue
  vtkLookupTable->SetTableValue( 254, 0.0, 0.0, 1.0, 0.7 );

  // red
  vtkLookupTable->SetTableValue( 255, 1.0, 0.0, 0.0, 0.7 );

  return vtkLookupTable;
}

vtkLookupTable* mitk::SeedsImageLookupTableSource::BuildForceLookupTable()
{
  vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
  vtkLookupTable->SetTableRange( 0.0, 1.0 );
  vtkLookupTable->SetValueRange( 0.0, 1.0 );
  vtkLookupTable->SetNumberOfColors( 256 );
  vtkLookupTable->Build();

  int i;
  for ( i = 0; i < 128; i++ )
  {
    vtkLookupTable->SetTableValue ( i, 
      0.0, 0.0, 1.0, 0.2 + fabs(128.0 - i) / 256.0 );
  }

  vtkLookupTable->SetTableValue ( 127, 0.0, 0.0, 1.0, 0.0 );

  for ( i = 128; i < 256; i++ )
  {
    vtkLookupTable->SetTableValue ( i,
      1.0, 1.0, 0.0, 0.2 + fabs(i - 128.0) / 256.0 );
  }

  return vtkLookupTable;
}

vtkLookupTable* mitk::SeedsImageLookupTableSource::BuildDefaultLookupTable()
{
  vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
  int size = 256;
  vtkLookupTable->SetTableRange(0,255);
  vtkLookupTable->SetNumberOfColors( size );
  vtkLookupTable->Build();
  return( vtkLookupTable );
}


vtkLookupTable* mitk::SeedsImageLookupTableSource::BuildVtkLookupTable()
{
  if ( m_Mode == Seeds )
  {
    return BuildSeedsLookupTable();
  }
  else if ( m_Mode == Force )
  {
    return BuildForceLookupTable();
  }

  else
  {
    return BuildDefaultLookupTable();
  }
}
