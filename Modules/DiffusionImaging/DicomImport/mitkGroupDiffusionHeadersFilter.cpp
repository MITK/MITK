/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-03-05 12:05:09 +0100 (Mo, 05 Mrz 2007) $
Version:   $Revision: 9583 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkGroupDiffusionHeadersFilter.h"


mitk::GroupDiffusionHeadersFilter::GroupDiffusionHeadersFilter()
{
  // Modify superclass default values, may be overridden by subclasses
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfRequiredOutputs( 4 );
}


mitk::GroupDiffusionHeadersFilter::~GroupDiffusionHeadersFilter()
{}

void mitk::GroupDiffusionHeadersFilter::GenerateOutputInformation()
{
}

void mitk::GroupDiffusionHeadersFilter::Update()
{
  InputType input =  static_cast<InputType>( this->GetInput( ) );
  this->SetNthOutput(0, input);

  InputType dwi;
  InputType zerodwi;
  InputType other;

  bool foundDWI = false;

  // check each series' first image
  unsigned int size = input.size();
  HeaderPointer header;
  HeaderPointer dwiHeader;
  for ( unsigned int i = 0 ; i < size ; ++i )
  {
    header = input[i];

    // list of files
    if( header->bValue > 0)
    {
      header->headerGroup = DHG_NonZeroDiffusionWeighted;
      if(!foundDWI)
        dwiHeader = header;
      foundDWI = true;
    }
    else
    {
      header->headerGroup = DHG_ZeroDiffusionWeighted;
    }
  }

  if(foundDWI)
  {
    for ( unsigned int i = 0 ; i < size ; ++i )
    {
      header = input[i];

      // list of files
      if( !header->isIdentical(dwiHeader))
      {
        header->headerGroup = DHG_Other;
      }
    }
  }
  else
  {
    for ( unsigned int i = 0 ; i < size ; ++i )
    {
      header = input[i];
      header->headerGroup = DHG_Other;
    }
  }

  for ( unsigned int i = 0 ; i < size ; ++i )
  {
    header = input[i];
    
    switch (header->headerGroup)
    {
    case DHG_Other:
      other.push_back(header);
      break;
    case DHG_ZeroDiffusionWeighted:
      zerodwi.push_back(header);
      break;
    case DHG_NonZeroDiffusionWeighted:
      dwi.push_back(header);
      break;
    case DHG_NotYetGrouped:
      break;
    }
  }

  this->SetNthOutput(1, dwi);
  this->SetNthOutput(2, zerodwi);
  this->SetNthOutput(3, other);

}

void mitk::GroupDiffusionHeadersFilter::SetInput( mitk::GroupDiffusionHeadersFilter::InputType input )
{
  // Process object is not const-correct so the const_cast is required here
  m_Input = input;
}


void mitk::GroupDiffusionHeadersFilter::SetInput( const unsigned int& /*idx*/, mitk::GroupDiffusionHeadersFilter::InputType input )
{
  this->SetInput( input );
}


mitk::GroupDiffusionHeadersFilter::InputType mitk::GroupDiffusionHeadersFilter::GetInput( void )
{
  return m_Input;
}


mitk::GroupDiffusionHeadersFilter::InputType mitk::GroupDiffusionHeadersFilter::GetInput( const unsigned int& /*idx*/ )
{
  return this->GetInput();
}

mitk::GroupDiffusionHeadersFilter::OutputType 
mitk::GroupDiffusionHeadersFilter::GetOutput()
{
  return m_Output;
}

void mitk::GroupDiffusionHeadersFilter::SetNthOutput( const unsigned int& idx, InputType output )
{
  if(idx >= m_Output.size())
    m_Output.resize(idx+1);
  m_Output[idx] = output;
}

void mitk::GroupDiffusionHeadersFilter::operator=( const mitk::GroupDiffusionHeadersFilter::Self& )
{}


