#include "mitkPointSetReader.h"
#include "vtkPointSetXMLParser.h"
#include <iostream>
#include <fstream>

mitk::PointSetReader::PointSetReader()
{
    this->SetNumberOfOutputs( 0 );
}


mitk::PointSetReader::~PointSetReader()
{}


void mitk::PointSetReader::GenerateData()
{
    std::cout << "Generate data called!" << std::endl;
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }
    if ( ! this->CanReadFile( m_FileName.c_str() ) )
    {
        itkWarningMacro( << "Sorry, can't read file " << m_FileName << "!" );
        return ;
    }
    std::ifstream in( m_FileName.c_str() );
    mitk::vtkPointSetXMLParser* parser = new mitk::vtkPointSetXMLParser();
    parser->SetStream( &in );
    parser->Parse();
    mitk::vtkPointSetXMLParser::PointSetList pointSetList = parser->GetParsedPointSets();
    this->ResizeOutputs(pointSetList.size());
    
    unsigned int i = 0;
    for (mitk::vtkPointSetXMLParser::PointSetList::iterator it = pointSetList.begin(); it != pointSetList.end(); ++it, ++i)
    {
        this->SetOutput(i, *it);    
    }
    in.close();
}


void mitk::PointSetReader::GenerateOutputInformation()
{}

int mitk::PointSetReader::CanReadFile ( const char *name )
{
    std::ifstream in( name );
    if ( !in.good() )
    {
        in.close();
        return false;
    }
    else
    {
        in.close();
        return true;
    }
}

void mitk::PointSetReader::ResizeOutputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfOutputs();
    this->SetNumberOfOutputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
    }
}

void mitk::PointSetReader::Read()
{
    this->GenerateData();    
}

