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
  vtkLookupTable->SetTableRange(0,255);

  #if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double rgba[ 4 ];
  #else
    float rgba[ 4 ];
  #endif

  int index;

  // red
  index = 254;
  rgba[ 0 ] = 1.0;    // red
  rgba[ 1 ] = 0.0;    // green
  rgba[ 2 ] = 0.0;    // blue
  rgba[ 3 ] = 0.7;    // alpha
  vtkLookupTable->SetTableValue ( index, rgba );

  // blue
  index = 253;
  rgba[ 0 ] = 0.0;    // red
  rgba[ 1 ] = 0.0;    // green
  rgba[ 2 ] = 1.0;    // blue
  rgba[ 3 ] = 0.7;    // alpha
  vtkLookupTable->SetTableValue ( index, rgba );

  // black
  for (int i = 0; i<253 ; i++ )
  {
    rgba[ 0 ] = 0.0;    // red
    rgba[ 1 ] = 0.0;    // green
    rgba[ 2 ] = 0.0;    // blue
    rgba[ 3 ] = 0.0;    // alpha
    vtkLookupTable->SetTableValue ( i, rgba );
  }
  return( vtkLookupTable );
}

vtkLookupTable* mitk::SeedsImageLookupTableSource::BuildForceLookupTable()
{
  vtkLookupTable *vtkLookupTable = vtkLookupTable::New();
  vtkLookupTable->SetTableRange(0,1);
  vtkLookupTable->SetValueRange(0,1);
  vtkLookupTable->SetNumberOfColors( 256 );
  vtkLookupTable->Build();

  #if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double rgba[ 4 ];
  #else
    float rgba[ 4 ];
  #endif

  for (int i = 0; i<128; i++ )
  {
    rgba[ 0 ] = 0.0;    // red
    rgba[ 1 ] = 0.0;    // green
    rgba[ 2 ] = 1.0;    // blue
    rgba[ 3 ] = 0.2 + fabs((float)128-i)/256.0;    // alpha
    vtkLookupTable->SetTableValue ( i, rgba );
  }
  rgba[ 0 ] = 0.0;    // red
  rgba[ 1 ] = 0.0;    // green
  rgba[ 2 ] = 1.0;    // blue
  rgba[ 3 ] = 0.0;    // alpha
  vtkLookupTable->SetTableValue ( 127, rgba );


  for (int i = 128; i<256; i++ )
  {
    rgba[ 0 ] = 1.0;    // red
    rgba[ 1 ] = 0.0;    // green
    rgba[ 2 ] = 0.0;    // blue
    rgba[ 3 ] = 0.2 + fabs((float)i-128)/256.0;    // alpha
    vtkLookupTable->SetTableValue ( i, rgba );
  }

  return( vtkLookupTable );
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
