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

#include "mitkContourModelSubDivisionFilter.h"
#include <mitkInteractionConst.h>
#include <mitkPointOperation.h>


mitk::ContourModelSubDivisionFilter::ContourModelSubDivisionFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());
  this->m_InterpolationIterations = 4;
}
mitk::ContourModelSubDivisionFilter::~ContourModelSubDivisionFilter()
{

}

void mitk::ContourModelSubDivisionFilter::SetInput ( const mitk::ContourModelSubDivisionFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ContourModelSubDivisionFilter::SetInput ( unsigned int idx, const mitk::ContourModelSubDivisionFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}



const mitk::ContourModelSubDivisionFilter::InputType* mitk::ContourModelSubDivisionFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelSubDivisionFilter::InputType*>(this->ProcessObject::GetInput(0));
}


const mitk::ContourModelSubDivisionFilter::InputType* mitk::ContourModelSubDivisionFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelSubDivisionFilter::InputType*>(this->ProcessObject::GetInput(idx));
}


void mitk::ContourModelSubDivisionFilter::GenerateData()
{

  mitk::ContourModel::Pointer input =  const_cast<mitk::ContourModel*>(this->GetInput(0));

  mitk::ContourModelSubDivisionFilter::OutputType::Pointer outputContour = this->GetOutput();

  mitk::ContourModel::Pointer contour(input);


  unsigned int timestep = input->GetTimeSteps();

  for ( int currentTimestep = 0; currentTimestep < timestep; currentTimestep++)
  {
    if( input->GetNumberOfVertices(currentTimestep) >= 4)
    {
      for( int iterations = 0; iterations < this->m_InterpolationIterations; iterations++)
      {
        InputType::VertexIterator it = contour->IteratorBegin();
        InputType::VertexIterator end = contour->IteratorEnd();

        InputType::VertexIterator first = contour->IteratorBegin();
        InputType::VertexIterator last = contour->IteratorEnd()-1;

        //tempory contour to store result of a subdivision iteration
        mitk::ContourModel::Pointer tempContour = mitk::ContourModel::New();

        //insert subpoints
        while ( it != end )
        {
          //add the current point to the temp contour
          tempContour->AddVertex((*it)->Coordinates, (*it)->IsControlPoint, currentTimestep);

          //control points for interpolation
          InputType::VertexIterator Ci = it;
          InputType::VertexIterator CiPlus1;
          InputType::VertexIterator CiPlus2;
          InputType::VertexIterator CiMinus1;

          //consider all possible cases
          if( it == first)
          {
            if( input->IsClosed(currentTimestep) )
            {
              CiPlus1 = it + 1;
              CiPlus2 = it + 2;
              CiMinus1 = last;
            }
            else
            {
              CiPlus1 = it + 1;
              CiPlus2 = it + 2;
              CiMinus1 = it;
            }
          }
          else if( it == last )
          {
            if( input->IsClosed(currentTimestep) )
            {
              CiPlus1 = first;
              CiPlus2 = first + 1;
              CiMinus1 = it -1;
            }
            else
            {
              //don't add point after last
              break;
            }
          }
          else if( it == (last - 1) )
          {
            if( input->IsClosed(currentTimestep) )
            {
              CiPlus1 = it + 1;
              CiPlus2 = first;
              CiMinus1 = it -1;
            }
            else
            {
              CiPlus1 = it + 1;
              CiPlus2 = it + 1;
              CiMinus1 = it -1;
            }
          }
          else
          {
            CiPlus1 = it + 1;
            CiPlus2 = it + 2;
            CiMinus1 = it -1;
          }


          /* F2i = Ci
          * F2i+1 = -1/16Ci-1 + 9/16Ci + 9/16Ci+1 - 1/16Ci+2
          */
          mitk::Point3D subpoint;

          mitk::Point3D a;
          a[0]=(-1.0/16.0) * (*CiMinus1)->Coordinates[0];
          a[1]=(-1.0/16.0) * (*CiMinus1)->Coordinates[1];
          a[2]= (-1.0/16.0) * (*CiMinus1)->Coordinates[2];


          mitk::Point3D b;
          b[0]=(9.0/16.0) * (*Ci)->Coordinates[0];
          b[1]=(9.0/16.0) * (*Ci)->Coordinates[1];
          b[2]= (9.0/16.0) * (*Ci)->Coordinates[2];

          mitk::Point3D c;
          c[0]=(9.0/16.0) * (*CiPlus1)->Coordinates[0];
          c[1]=(9.0/16.0) * (*CiPlus1)->Coordinates[1];
          c[2]= (9.0/16.0) * (*CiPlus1)->Coordinates[2];

          mitk::Point3D d;
          d[0]=(-1.0/16.0) * (*CiPlus2)->Coordinates[0];
          d[1]=(-1.0/16.0) * (*CiPlus2)->Coordinates[1];
          d[2]= (-1.0/16.0) * (*CiPlus2)->Coordinates[2];


          subpoint[0] = a[0] + b[0] + c[0] + d[0];
          subpoint[1] = a[1] + b[1] + c[1] + d[1];
          subpoint[2] = a[2] + b[2] + c[2] + d[2];
          InputType::VertexType subdivisionPoint(subpoint,false);

          //add the new subdivision point to our tempContour
          tempContour->AddVertex(subdivisionPoint.Coordinates, currentTimestep);

          it++;
        }

        //set the interpolated contour as the contour for the next iteration
        contour = tempContour;
      }
    }
    else
    {
      //filter not executeable - set input to output
      contour = input;
    }
  }

  //somehow the isClosed property is not set via copy constructor
  contour->SetIsClosed(input->IsClosed());

  this->SetOutput(contour);

}
