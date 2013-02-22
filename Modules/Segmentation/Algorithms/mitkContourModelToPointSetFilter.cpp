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

void mitk::ContourModelToPointSetFilter::SetInput ( unsigned int idx, const mitk::ContourModelToPointSetFilter::InputType* input )
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



mitk::ContourModelToPointSetFilter::OutputType* mitk::ContourModelToPointSetFilter::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
  {
    return 0;
  }

  return static_cast<mitk::ContourModelToPointSetFilter::OutputType*>
    (this->BaseProcess::GetOutput(0));
}

mitk::ContourModelToPointSetFilter::OutputType* mitk::ContourModelToPointSetFilter::GetOutput(unsigned int idx)
{
  return static_cast<mitk::ContourModelToPointSetFilter::OutputType*>
    (this->itk::ProcessObject::GetOutput(idx));
}

void mitk::ContourModelToPointSetFilter::SetOutput(mitk::ContourModelToPointSetFilter::OutputType* output)
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
  BaseProcess::SetNthOutput(0, output);
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

const mitk::ContourModelToPointSetFilter::InputType* mitk::ContourModelToPointSetFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelToPointSetFilter::InputType*>(this->ProcessObject::GetInput(0));
}


const mitk::ContourModelToPointSetFilter::InputType* mitk::ContourModelToPointSetFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ContourModelToPointSetFilter::InputType*>(this->ProcessObject::GetInput(idx));
}


void mitk::ContourModelToPointSetFilter::GenerateData()
{

  mitk::ContourModel::Pointer inputContour =  const_cast<mitk::ContourModel*>(this->GetInput(0));
  mitk::ContourModelToPointSetFilter::OutputType* outputPointSet = this->GetOutput();

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
