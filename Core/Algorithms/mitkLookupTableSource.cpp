#include "mitkLookupTableSource.h"

mitk::LookupTableSource::LookupTableSource()
{
    // Create the output.

    std::cout << "mitk::LookupTableSource::LookupTableSource()... " << std::endl;
    //    OutputType::Pointer output = static_cast<OutputType*> ( this->MakeOutput( 0 ).GetPointer() );
    //    Superclass::SetNumberOfRequiredOutputs( 1 );
    //    Superclass::SetNthOutput( 0, output.GetPointer() );


    // Create the output. We use static_cast<> here because we know the default
    // output must be of type TOutputImage
    OutputType::Pointer output = static_cast<OutputType*>( this->MakeOutput( 0 ).GetPointer() );

    if ( output.GetPointer() == NULL )
        std::cout << "something went wrong..." << std::endl;

    this->ProcessObject::SetNumberOfRequiredOutputs( 1 );
    this->ProcessObject::SetNthOutput( 0, output.GetPointer() );


    std::cout << "mitk::LookupTableSource::LookupTableSource() OK! " << std::endl;
}


mitk::LookupTableSource::~LookupTableSource()
{}




mitk::LookupTableSource::DataObjectPointer
mitk::LookupTableSource::MakeOutput ( unsigned int )
{
    std::cout << "mitk::LookupTableSource::MakeOutput()... " << std::endl;
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

    std::cout << "returning lookuptable output ... " << std::endl;

    if ( static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) ) == NULL )
        std::cout << "oooops..." << std::endl;
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( 0 ) );
}




mitk::LookupTableSource::OutputType*
mitk::LookupTableSource::GetOutput ( unsigned int idx )
{
    return static_cast<OutputType*> ( this->ProcessObject::GetOutput( idx ) );
}




void
mitk::LookupTableSource::GenerateInputRequestedRegion()
{
    this->ProcessObject::GenerateInputRequestedRegion();
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
