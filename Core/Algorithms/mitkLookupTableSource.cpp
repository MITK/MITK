#include "mitkLookupTableSource.h"

mitk::LookupTableSource::LookupTableSource()
{
    // Create the output.
    OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    Superclass::SetNumberOfRequiredOutputs( 1 );
    Superclass::SetNthOutput( 0, output.GetPointer() );
}


mitk::LookupTableSource::~LookupTableSource()
{}




mitk::LookupTableSource::DataObjectPointer
mitk::LookupTableSource::MakeOutput ( unsigned int )
{
    return OutputType::New().GetPointer();
}




void
mitk::LookupTableSource::SetOutput( OutputType* output )
{
    itkWarningMacro( << "SetOutput(): This method is slated to be removed from ITK.  Please use GraftOutput() in possible combination with DisconnectPipeline() instead." );
    this->SetNthOutput( 0, output );
}




mitk::LookupTableSource::OutputType*
mitk::LookupTableSource::GetOutput()
{
    if ( this->GetNumberOfOutputs() < 1 )
    {
        return 0;
    }
    return static_cast<OutputType*> ( Superclass::GetOutput( 0 ) );
}




mitk::LookupTableSource::OutputType*
mitk::LookupTableSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( Superclass::GetOutput( idx ) );
}




void
mitk::LookupTableSource::GenerateInputRequestedRegion()
{
    Superclass::GenerateInputRequestedRegion();
}




void
mitk::LookupTableSource::GraftOutput( OutputType* graft )
{
    OutputType * output = this->GetOutput();

    if ( output && graft )
    {
        // grab a handle to the bulk data of the specified data object
        // output->SetPixelContainer( graft->GetPixelContainer() );

        // copy the region ivars of the specified data object
        // output->SetRequestedRegion( graft->GetRequestedRegion() );
        // output->SetLargestPossibleRegion( graft->GetLargestPossibleRegion() );
        // output->SetBufferedRegion( graft->GetBufferedRegion() );

        // copy the meta-information
        output->CopyInformation( graft );
    }
}
