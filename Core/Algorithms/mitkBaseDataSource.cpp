#include "mitkBaseDataSource.h"


mitk::BaseDataSource::BaseDataSource()
{
    // Create the output.
    //OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    //Superclass::SetNumberOfRequiredOutputs( 1 );
    //Superclass::SetNthOutput( 0, output.GetPointer() );
}

mitk::BaseDataSource::~BaseDataSource()
{
}


itk::DataObject::Pointer mitk::BaseDataSource::MakeOutput ( unsigned int idx )
{
    //this doesn't work, since we can not instantiate a base data
    //return OutputType::New().getPointer();
}




/**
 * Allows to set the output of the base data source. According to the itk
 * this method is outdated and should not be used. Instead GraftOutput(...)
 * should be used.
 * @param output the intended output of the vessel graph source
 */
void mitk::BaseDataSource::SetOutput( OutputType* output )
{
    this->SetNthOutput( 0, output );
}




/**
 * Returns the output with index 0 of the base data source
 * @returns the output
 */
mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    return static_cast<OutputType*> ( Superclass::GetOutput( 0 ) );
}


/**
 * Returns the n'th output of the base data source
 * @param idx the index of the wanted output
 * @returns the output with index idx.
 */
mitk::BaseDataSource::OutputType* mitk::BaseDataSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( Superclass::GetOutput( idx ) );
}


