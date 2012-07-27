///*===================================================================
//
//The Medical Imaging Interaction Toolkit (MITK)
//
//Copyright (c) German Cancer Research Center,
//Division of Medical and Biological Informatics.
//All rights reserved.
//
//This software is distributed WITHOUT ANY WARRANTY; without
//even the implied warranty of MERCHANTABILITY or FITNESS FOR
//A PARTICULAR PURPOSE.
//
//See LICENSE.txt or http://www.mitk.org for details.
//
//===================================================================*/
//#include <mitkContourModelToVtkPolyDataFilter.h>
//
//mitk::ContourModelToVtkPolyDataFilter::ContourModelToVtkPolyDataFilter()
//{
//
//}
//mitk::ContourModelToVtkPolyDataFilter::~ContourModelToVtkPolyDataFilter()
//{
//
//}
//
//void mitk::ContourModelToVtkPolyDataFilter::SetInput ( const mitk::ContourModelToVtkPolyDataFilter::InputType* input )
//{
//  this->SetInput( 0, input );
//}
//
//void mitk::ContourModelToVtkPolyDataFilter::SetInput ( unsigned int idx, const mitk::ContourModelToVtkPolyDataFilter::InputType* input )
//{
//  if ( idx + 1 > this->GetNumberOfInputs() )
//  {
//    this->SetNumberOfRequiredInputs(idx + 1);
//  }
//  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
//  {
//    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
//    this->Modified();
//  }
//}
//
//itk::DataObject::Pointer mitk::ContourModelToVtkPolyDataFilter::MakeOutput ( unsigned int idx )
//{
//  return NULL;
//}
//
//mitk::ContourModelToVtkPolyDataFilter::OutputType* mitk::ContourModelToVtkPolyDataFilter::GetOutput()
//{
//  if (this->GetNumberOfOutputs() < 1)
//  {
//    return 0;
//  }
//
//  return static_cast<mitk::ContourModelToVtkPolyDataFilter::OutputType*>
//    (this->BaseProcess::GetOutput(0));
//}
//
//mitk::ContourModelToVtkPolyDataFilter::OutputType* mitk::ContourModelToVtkPolyDataFilter::GetOutput(unsigned int idx)
//{
//  return static_cast<mitk::ContourModelToVtkPolyDataFilter::OutputType*>
//    (this->itk::ProcessObject::GetOutput(idx));
//}
//
//void mitk::ContourModelToVtkPolyDataFilter::SetOutput(mitk::ContourModelToVtkPolyDataFilter::OutputType* output)
//{
//  itkWarningMacro(<< "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
//  BaseProcess::SetNthOutput(0, output);
//}
//
//void mitk::ContourModelToVtkPolyDataFilter::GraftOutput(mitk::ContourModelToVtkPolyDataFilter::OutputType* graft)
//{
//  this->GraftNthOutput(0, graft);
//}
//
//void mitk::ContourModelToVtkPolyDataFilter::GraftNthOutput(unsigned int idx, mitk::ContourModelToVtkPolyDataFilter::OutputType *graft)
//{
//  if (idx < this->GetNumberOfOutputs())
//  {
//    mitk::ContourModelToVtkPolyDataFilter::OutputType* output = this->GetOutput(idx);
//
//    if (output && graft)
//    {
//      //// grab a handle to the bulk data of the specified data object
//      ////      output->SetPixelContainer( graft->GetPixelContainer() ); @FIXME!!!!
//
//      //// copy the region ivars of the specified data object
//      //output->SetRequestedRegion( graft );//graft->GetRequestedRegion() );
//      ////      output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() ); @FIXME!!!!
//      ////      output->SetBufferedRegion( graft->GetBufferedRegion() ); @FIXME!!!!
//
//      //// copy the meta-information
//      //output->CopyInformation( graft );
//    }
//  }
//}
//
//
//void mitk::ContourModelToVtkPolyDataFilter::GenerateData()
//{
//
//}
