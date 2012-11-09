

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

#include <mitkConnectomicsHistogramBase.h>
#include <mitkConnectomicsConstantsManager.h>

#include <boost/numeric/conversion/converter.hpp>

mitk::ConnectomicsHistogramBase::ConnectomicsHistogramBase()
: m_Valid( false )
, m_BaselineValue( 0 )
, m_TopValue( 1 )
, m_StartValue( 0 )
, m_Subject( "" )
{
}

mitk::ConnectomicsHistogramBase::~ConnectomicsHistogramBase()
{
}


double mitk::ConnectomicsHistogramBase::GetYMin() const
{
  return m_BaselineValue;
}

double mitk::ConnectomicsHistogramBase::GetYMax() const
{
  return m_TopValue;
}

double mitk::ConnectomicsHistogramBase::GetMin() const
{
  return this->GetXMin();
}

double mitk::ConnectomicsHistogramBase::GetMax() const
{
  return this->GetXMax();
}

double mitk::ConnectomicsHistogramBase::GetXMin() const
{
  return m_StartValue;
}

double mitk::ConnectomicsHistogramBase::GetXMax() const
{
  return ( m_StartValue + this->GetRange() );
}

int mitk::ConnectomicsHistogramBase::GetRange() const
{
  return m_HistogramVector.size();
}

bool mitk::ConnectomicsHistogramBase::IsValid() const
{
  return m_Valid;
}

void mitk::ConnectomicsHistogramBase::PrintToConsole() const
{
  MITK_INFO << "Histogram - Maximum " << this->GetYMax() << " Minimum " << this->GetYMin() << " Range " << this->GetRange();

  for( int index( 0 ); index < m_HistogramVector.size(); index++ )
  {
    MITK_INFO << " Bin: " << index << " Value: " << m_HistogramVector[ index ];
  }
}

std::string mitk::ConnectomicsHistogramBase::GetSubject() const
{
  return m_Subject;
}

void mitk::ConnectomicsHistogramBase::SetSubject( std::string subject )
{
  m_Subject = subject;
}

void mitk::ConnectomicsHistogramBase::ComputeFromBaseData( BaseData* source )
{
  m_Valid = false;
  m_HistogramVector.clear();

  //check if input is valid
  if (source==NULL)
  { // empty base data
    return;
  }

  mitk::ConnectomicsNetwork* networkSource = dynamic_cast<mitk::ConnectomicsNetwork*>(source);

  if (networkSource==NULL)
  { // base data but no network
    return;
  }

  ComputeFromConnectomicsNetwork( networkSource );
}

float mitk::ConnectomicsHistogramBase::GetRelativeBin( double start, double end ) const
{
  // use the boost double to int converter
  // it defaults to trunc
  typedef boost::numeric::converter<int,double> Double2Int ;

  float result( 0.0 );

  if( !m_Valid )
  {
    MITK_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_TRIED_TO_ACCESS_INVALID_HISTOGRAM;
    return result;
  }

  if( ( start < 0.0 ) ||
      ( end   < 0.0 )   )
  {
    MITK_ERROR << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_PASSED_NEGATIVE_INDEX_TO_HISTOGRAM;
    return result;
  }

  // calculate result
  if( std::abs( end - start ) <= 1.0 )
  { // if the bin size is one or less, we can do not need to interpolate

    int index( 0 );
    try
    {
      // show the value for n between n - .5 and n + .5
        double temp = ( start + end ) / 2.0;
        index = Double2Int::convert( temp ); // By default throws positive_overflow()
    }
    catch ( boost::numeric::positive_overflow const& )
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_OUTSIDE_INTEGER_RANGE;
    }

    if( index < m_HistogramVector.size() )
    {
      result = m_HistogramVector[ index ];
    }
    else
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_BEYOND_SCOPE <<
        index << " on vector sized: " <<  m_HistogramVector.size();
    }
  }
  else
  { // if the bin size is more than one we need to interpolate

    int indexStart( 0 ), indexEnd( 0 );

    try
    {
      indexStart = Double2Int::convert( start );
      indexEnd   = Double2Int::convert( end   );
    }
    catch ( boost::numeric::positive_overflow const& )
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_OUTSIDE_INTEGER_RANGE;
    }

    if( ( indexStart < m_HistogramVector.size() ) &&
        ( indexEnd   < m_HistogramVector.size() )    )
    {
      // add up weighted values and divide by range

      // add partial start and end bin

      double startPercentage = 1.0 - start + indexStart;
      double endPercentage   = end - indexEnd;

      result += startPercentage * m_HistogramVector[ indexStart ];
      result += endPercentage   * m_HistogramVector[ indexEnd   ];

      // add whole inbetween bins
      for( int tempIndex = indexStart + 1; tempIndex < indexEnd; tempIndex++ )
      {
        result += m_HistogramVector[ tempIndex ];
      }
    }
    else
    {
      MBI_WARN << mitk::ConnectomicsConstantsManager::CONNECTOMICS_ERROR_BEYOND_SCOPE << indexStart << " to " <<
        indexEnd << " on vector sized: " <<  m_HistogramVector.size();
    }
  }

  // normalizeresult by dividing through maximum degree
  result = result / GetYMax();
  return result;
}

void mitk::ConnectomicsHistogramBase::UpdateYMax()
{
  for ( int index( 0 ); index < m_HistogramVector.size(); index++ )
  {
    if( m_HistogramVector[ index ] > m_TopValue )
    {
      m_TopValue = m_HistogramVector[ index ];
    }
  }
}

std::vector< double > mitk::ConnectomicsHistogramBase::GetHistogramVector()
{
  return m_HistogramVector;
}
