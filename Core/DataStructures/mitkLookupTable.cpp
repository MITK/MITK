#include "mitkLookupTable.h"
#include "itkProcessObject.h"

mitk::LookupTable::LookupTable()
{
    m_LookupTable = vtkLookupTable::New();
    this->SetRequestedRegionToLargestPossibleRegion();
}


mitk::LookupTable::~LookupTable()
{}

void mitk::LookupTable::SetVtkLookupTable( vtkLookupTable* lut )
{
    if (m_LookupTable != lut)
    {
        m_LookupTable = lut;
        this->Modified();
    }
}


void mitk::LookupTable::ChangeOpacityForAll( float opacity )
{

    int noValues = m_LookupTable->GetNumberOfTableValues ();

    float rgba[ 4 ];
    for ( int i = 0;i < noValues;i++ )
    {
        m_LookupTable->GetTableValue ( i, rgba );
        rgba[ 3 ] = opacity;
        m_LookupTable->SetTableValue ( i, rgba );
    }
    this->Modified();  // need to call modiefied, since LookupTableProperty seems to be unchanged so no widget-updat is executed
}



vtkLookupTable* mitk::LookupTable::GetVtkLookupTable() const
{
    return m_LookupTable;
};

mitk::LookupTable::RawLookupTableType * mitk::LookupTable::GetRawLookupTable() const
{
    return m_LookupTable->GetPointer( 0 );
};


/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator==( const mitk::LookupTable& LookupTable ) const
{

    if ( m_LookupTable == ( LookupTable.GetVtkLookupTable() ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief equality operator inplementation
 */
bool mitk::LookupTable::operator!=( const mitk::LookupTable& LookupTable ) const
{

    if ( m_LookupTable == ( LookupTable.GetVtkLookupTable() ) )
    {
        return false;
    }
    else
    {
        return true;
    }

}

/*!
 * \brief non equality operator inplementation
 */
mitk::LookupTable& mitk::LookupTable::operator=( const mitk::LookupTable& LookupTable )
{
    if ( this == &LookupTable )
    {
        return * this;
    }
    else
    {
        m_LookupTable = LookupTable.GetVtkLookupTable();
        return *this;
    }
}

void mitk::LookupTable::UpdateOutputInformation( )
{
    if ( this->GetSource( ) )
    {
        this->GetSource( ) ->UpdateOutputInformation( );
    }
}


void mitk::LookupTable::SetRequestedRegionToLargestPossibleRegion( )
{}


bool mitk::LookupTable::RequestedRegionIsOutsideOfTheBufferedRegion( )
{
    if ( VerifyRequestedRegion( ) == false )
        return true;
    else
        return false;
}


bool mitk::LookupTable::VerifyRequestedRegion( )
{
    //normally we should check if the requested region lies within the
    //largest possible region. Since for lookup-tables we assume, that the
    //requested region is always the largest possible region, we can always
    //return true!
    return true;
}


void mitk::LookupTable::SetRequestedRegion( itk::DataObject *data )
{
    //not implemented, since we always want to have the RequestedRegion
    //to be set to LargestPossibleRegion
}


