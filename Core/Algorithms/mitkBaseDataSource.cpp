#include "mitkBaseDataSource.h"


mitk::BaseDataSource::BaseDataSource()
{
    OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    Superclass::SetNumberOfRequiredOutputs( 1 );
    Superclass::SetNthOutput( 0, output.GetPointer() );
}



mitk::BaseDataSource::~BaseDataSource()
{
}



void mitk::BaseDataSource::SetOutput( OutputType* output )
{
    this->SetNthOutput( 0, output );
}



void mitk::BaseDataSource::SetOutput( unsigned int idx, OutputType* output )
{
    this->SetNthOutput(idx, output);    
}



mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    return static_cast<OutputType*> ( Superclass::GetOutput( 0 ) );
}



mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( Superclass::GetOutput( idx ) );
}


