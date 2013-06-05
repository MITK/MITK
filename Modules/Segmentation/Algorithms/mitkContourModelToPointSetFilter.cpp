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
  itk::DataObject::Pointer output = this->MakeOutput(0);
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs( 1 );
  this->SetNthOutput(0, output.GetPointer());
}
mitk::ContourModelToPointSetFilter::~ContourModelToPointSetFilter()
{

}

void mitk::ContourModelToPointSetFilter::GenerateData()
{

  mitk::ContourModel::Pointer inputContour =  static_cast<mitk::ContourModel*>(this->GetInput(0));
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
