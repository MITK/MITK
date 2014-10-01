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

#include "mitkContourModelSetWriter.h"
#include "mitkContourModelWriter.h"



mitk::ContourModelSetWriter::ContourModelSetWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfIndexedOutputs( 1 );
  this->SetNthOutput( 0, mitk::ContourModel::New().GetPointer() );
  m_Success = false;
}




mitk::ContourModelSetWriter::~ContourModelSetWriter()
{}




void mitk::ContourModelSetWriter::GenerateData()
{
  //Use regular ContourModel writer to write each contour of the set to a single file.
  //Just use a different file extension .cnt_set

  mitk::ContourModelWriter::Pointer writer = mitk::ContourModelWriter::New();
  writer->SetFileName(this->GetFileName());

  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set! Setting filename to default." );
    m_FileName = GetDefaultFilename();
  }

  InputType::Pointer contourModelSet = this->GetInput();

  //
  // for each contour object set input of writer
  //
  for ( int i = 0 ; i < contourModelSet->GetSize(); ++i )
  {
    mitk::ContourModel* contour = contourModelSet->GetContourModelAt(i);
    writer->SetInput( i, contour );
  }

  writer->Update();

  m_Success = true;
  m_MimeType = "application/MITK.ContourModel";
}


void mitk::ContourModelSetWriter::ResizeInputs( const unsigned int& num )
{
  unsigned int prevNum = this->GetNumberOfInputs();
  this->SetNumberOfIndexedInputs( num );
  for ( unsigned int i = prevNum; i < num; ++i )
  {
    this->SetNthInput( i, mitk::ContourModel::New().GetPointer() );
  }
}




void mitk::ContourModelSetWriter::SetInput( InputType* contourModel )
{
  this->ProcessObject::SetNthInput( 0, contourModel );
}




void mitk::ContourModelSetWriter::SetInput( const unsigned int& id, InputType* contourModel )
{
  if ( id >= this->GetNumberOfInputs() )
    this->ResizeInputs( id + 1 );
  this->ProcessObject::SetNthInput( id, contourModel );
}



mitk::ContourModelSet* mitk::ContourModelSetWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return 0;
  }
  else
  {
    return dynamic_cast<InputType*> ( this->GetInput( 0 ) );
  }
}




mitk::ContourModelSet* mitk::ContourModelSetWriter::GetInput( const unsigned int& num )
{
  return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( num ) );
}




bool mitk::ContourModelSetWriter::GetSuccess() const
{
  return m_Success;
}

std::string mitk::ContourModelSetWriter::GetSupportedBaseData() const
{
  return ContourModelSet::GetStaticNameOfClass();
}

bool mitk::ContourModelSetWriter::CanWriteDataType( DataNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
      mitk::ContourModel::Pointer contourModel = dynamic_cast<mitk::ContourModel*>( data );
      if( contourModel.IsNotNull() )
      {
        //this writer has no "SetDefaultExtension()" - function
        m_Extension = ".cnt_set";
        return true;
      }
    }
  }
  return false;
}

void mitk::ContourModelSetWriter::SetInput( DataNode* input )
{
  if( input && CanWriteDataType( input ) )
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::ContourModel*>( input->GetData() ) );
}

std::vector<std::string> mitk::ContourModelSetWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".cnt_set");
  return possibleFileExtensions;
}

std::string mitk::ContourModelSetWriter::GetFileExtension()
{
  return m_Extension;
}

std::string mitk::ContourModelSetWriter::GetWritenMIMEType()
{
  return "application/MITK.ContourModelSet";
}
