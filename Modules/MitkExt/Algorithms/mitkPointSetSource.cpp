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


#include "mitkPointSetSource.h"


mitk::PointSetSource::PointSetSource()
{
  // Create the output.
  OutputType::Pointer output = dynamic_cast<OutputType*>(this->MakeOutput(0).GetPointer()); 
  Superclass::SetNumberOfRequiredInputs(0);
  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, output.GetPointer());
}




mitk::PointSetSource::~PointSetSource()
{
}




itk::DataObject::Pointer mitk::PointSetSource::MakeOutput ( unsigned int /*idx */)
{
    return OutputType::New().GetPointer();
}




void mitk::PointSetSource::SetOutput( OutputType* output )
{
  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
    this->ProcessObject::SetNthOutput( 0, output );
}




void mitk::PointSetSource::GraftOutput(OutputType *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::PointSetSource::GraftNthOutput(unsigned int /*idx*/, OutputType* /*graft*/)
{
  itkWarningMacro(<< "GraftNthOutput(): This method is not yet implemented for mitk. Implement it before using!!" );
  assert(false);
}



mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    else
    {
      return dynamic_cast<OutputType*>
                         (this->BaseProcess::GetOutput(0));
    }
}




mitk::PointSetSource::OutputType* mitk::PointSetSource::GetOutput ( unsigned int idx )
{
    return dynamic_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}


