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


#include "mitkLookupTable.h"
#include "itkProcessObject.h"

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>


mitk::LookupTable::LookupTable()
{
    m_LookupTable = vtkLookupTable::New();
    this->SetRequestedRegionToLargestPossibleRegion();
}


mitk::LookupTable::~LookupTable()
{}

void mitk::LookupTable::SetVtkLookupTable( vtkLookupTable* lut )
{
    if (m_LookupTable != lut)
    {
        m_LookupTable = lut;
        this->Modified();
    }

}


void mitk::LookupTable::ChangeOpacityForAll( float opacity )
{

    int noValues = m_LookupTable->GetNumberOfTableValues ();


#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double rgba[ 4 ];
#else
    float rgba[ 4 ];
#endif

    for ( int i = 0;i < noValues;i++ )
    {
        m_LookupTable->GetTableValue ( i, rgba );
        rgba[ 3 ] = opacity;
        m_LookupTable->SetTableValue ( i, rgba );
    }
    this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-updat is executed
}

void mitk::LookupTable::ChangeOpacity(int index, float opacity )
{

    int noValues = m_LookupTable->GetNumberOfTableValues ();
		if (index>noValues)
		{
			std::cout << "could not change opacity. index exceed size of lut ... " << std::endl;
			return;
		}
		
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double rgba[ 4 ];
#else
    float rgba[ 4 ];
#endif
    m_LookupTable->GetTableValue ( index, rgba );
    rgba[ 3 ] = opacity;
    m_LookupTable->SetTableValue ( index, rgba );

    this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-updat is executed
}


vtkLookupTable* mitk::LookupTable::GetVtkLookupTable() const
{
    return m_LookupTable;
};

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{

		if (m_LookupTable==NULL) std::cout << "uuups..." << std::endl;
    return m_LookupTable->GetPointer( 0 );
};

/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator==( const mitk::LookupTable& LookupTable ) const
{
    if ( m_LookupTable == LookupTable.GetVtkLookupTable() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator!=( const mitk::LookupTable& LookupTable ) const
{

    if ( m_LookupTable == ( LookupTable.GetVtkLookupTable() ) )
    {
        return false;
    }
    else
    {
        return true;
    }

}

/*!
 * \brief non equality operator inplementation
 */
mitk::LookupTable& mitk::LookupTable::operator=( const mitk::LookupTable& LookupTable )
{
    if ( this == &LookupTable )
    {
        return * this;
    }
    else
    {
        m_LookupTable = LookupTable.GetVtkLookupTable();
        return *this;
    }
}

void mitk::LookupTable::UpdateOutputInformation( )
{
    if ( this->GetSource( ) )
    {
        this->GetSource( ) ->UpdateOutputInformation( );
    }
}


void mitk::LookupTable::SetRequestedRegionToLargestPossibleRegion( )
{}


bool mitk::LookupTable::RequestedRegionIsOutsideOfTheBufferedRegion( )
{
    return false;
}


bool mitk::LookupTable::VerifyRequestedRegion( )
{
    //normally we should check if the requested region lies within the
    //largest possible region. Since for lookup-tables we assume, that the
    //requested region is always the largest possible region, we can always
    //return true!
    return true;
}


void mitk::LookupTable::SetRequestedRegion( itk::DataObject *data )
{
    //not implemented, since we always want to have the RequestedRegion
    //to be set to LargestPossibleRegion
}

void mitk::LookupTable::CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction)
{
  if(colorFunction==NULL)
    colorFunction = vtkColorTransferFunction::New();
  
  mitk::LookupTable::RawLookupTableType *rgba = GetRawLookupTable();
  int i, num_of_values=m_LookupTable->GetNumberOfTableValues();


#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double *cols;
  double *colsHead;
  colsHead=cols=(double *)malloc(sizeof(double*)*num_of_values*3);
#else
  float *cols;
  float *colsHead;
  colsHead=cols=(float *)malloc(sizeof(float *)*num_of_values*3);
#endif



  for(i=0;i<num_of_values;++i)
  {
    *cols=*rgba/255.0; ++cols; ++rgba;
    *cols=*rgba/255.0; ++cols; ++rgba;
    *cols=*rgba/255.0; ++cols; ++rgba;
    ++rgba;
  }
  colorFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values-1, colsHead);

  free(colsHead);
}

void mitk::LookupTable::CreateOpacityTransferFunction(vtkPiecewiseFunction*& opacityFunction)
{
  if(opacityFunction==NULL)
    opacityFunction = vtkPiecewiseFunction::New();

  mitk::LookupTable::RawLookupTableType *rgba = GetRawLookupTable();
  int i, num_of_values=m_LookupTable->GetNumberOfTableValues();


#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double *alphas;
  double *alphasHead;
  alphasHead=alphas=(double*)malloc(sizeof(double*)*num_of_values);
#else
  float *alphas;
  float *alphasHead;
  alphasHead=alphas=(float *)malloc(sizeof(float *)*num_of_values);
#endif


  rgba+=3;
  for(i=0;i<num_of_values;++i)
  {
    *alphas=*rgba/255.0; ++alphas; rgba+=4;
  }

  opacityFunction->BuildFunctionFromTable(m_LookupTable->GetTableRange()[0], m_LookupTable->GetTableRange()[1], num_of_values-1, alphasHead);

  free(alphasHead);
}
