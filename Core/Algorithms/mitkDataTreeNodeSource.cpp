#include "mitkDataTreeNodeSource.h"


mitk::DataTreeNodeSource::DataTreeNodeSource()
{
    // Create the output.
    OutputType::Pointer output = dynamic_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    assert (output.IsNotNull());
    this->SetNumberOfOutputs( 1 );
    this->SetOutput(0, output.GetPointer());
}




mitk::DataTreeNodeSource::~DataTreeNodeSource()
{
}




itk::DataObject::Pointer mitk::DataTreeNodeSource::MakeOutput ( unsigned int idx )
{
    return OutputType::New().GetPointer();
}




void mitk::DataTreeNodeSource::SetOutput( OutputType* output )
{
    this->ProcessObject::SetNthOutput( 0, output );
}




void mitk::DataTreeNodeSource::SetOutput( unsigned int idx, OutputType* output )
{
    this->ProcessObject::SetNthOutput(idx, output);    
}




mitk::DataTreeNodeSource::OutputType* mitk::DataTreeNodeSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    else
    {
        return dynamic_cast<OutputType*> ( this->GetOutput( 0 ) );
    }
}




mitk::DataTreeNodeSource::OutputType* mitk::DataTreeNodeSource::GetOutput ( unsigned int idx )
{
    return dynamic_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}


