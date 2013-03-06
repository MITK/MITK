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

#include "mitkContourModelToPointSetFilter.h"
#include <mitkInteractionConst.h>
#include <mitkPointOperation.h>


mitk::ContourModelToPointSetFilter::ContourModelToPointSetFilter()
{
  OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());
}
mitk::ContourModelToPointSetFilter::~ContourModelToPointSetFilter()
{

}

void mitk::ContourModelToPointSetFilter::SetInput ( const mitk::ContourModelToPointSetFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ContourModelToPointSetFilter::SetInput (DataObjectPointerArraySizeType idx, const mitk::ContourModelToPointSetFilter::InputType* input )
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

void mitk::ContourModelToPointSetFilter::GraftOutput(mitk::ContourModelToPointSetFilter::OutputType* graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::ContourModelToPointSetFilter::GraftNthOutput(unsigned int idx, mitk::ContourModelToPointSetFilter::OutputType *graft)
{
  if (idx < this->GetNumberOfOutputs())
  {
    mitk::ContourModelToPointSetFilter::OutputType* output = this->GetOutput(idx);

    if (output && graft)
    {
      //// grab a handle to the bulk data of the specified data object
      ////      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!

      //// copy the region ivars of the specified data object
      //output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
      ////      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
      ////      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!

      //// copy the meta-information
      //output->CopyInformation( graft );
    }
  }
}

void mitk::ContourModelToPointSetFilter::GenerateData()
{

  mitk::ContourModel::Pointer inputContour =  const_cast<mitk::ContourModel*>(this->GetInput(0));
  mitk::ContourModelToPointSetFilter::OutputType* outputPointSet = this->GetOutput(0);

  InputType::VertexIterator it = inputContour->IteratorBegin();
  InputType::VertexIterator end = inputContour->IteratorEnd();
  unsigned int pointId = 0;


  unsigned int timestep = inputContour->GetTimeSteps();

  for ( int i = 0; i < timestep; i++)
  {


    while ( it <= end )
    {
      mitk::Point3D p = (*it)->Coordinates;
      mitk::PointOperation popInsert( mitk::OpINSERT,
                                      inputContour->GetTimeSlicedGeometry()->TimeStepToMS(timestep),
                                      p,
                                      pointId++,
                                      false );

      outputPointSet->ExecuteOperation( &popInsert );

      it++;
    }
  }
}
