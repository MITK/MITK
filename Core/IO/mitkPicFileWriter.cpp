/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPicFileWriter.h"
#include "mitkPicFileReader.h"

#include "mitkImage.h"

#include <itksys/SystemTools.hxx>

#include <sstream>

mitk::PicFileWriter::PicFileWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::PicFileWriter::~PicFileWriter()
{
}

void mitk::PicFileWriter::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::Image::Pointer input = const_cast<mitk::Image*>(this->GetInput());

  ipPicDescriptor * picImage = input->GetPic();
  mitk::SlicedGeometry3D* slicedGeometry = input->GetSlicedGeometry();
  if (slicedGeometry != NULL)
  {
    //set tag "REAL PIXEL SIZE"
    const mitk::Vector3D & spacing = slicedGeometry->GetSpacing();
    ipPicTSV_t *pixelSizeTag;
    pixelSizeTag = ipPicQueryTag( picImage, "REAL PIXEL SIZE" );
    if (!pixelSizeTag)
    {
      pixelSizeTag = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
      pixelSizeTag->type = ipPicFloat;
      pixelSizeTag->bpe = 32;
      strcpy(pixelSizeTag->tag, "REAL PIXEL SIZE");
      pixelSizeTag->dim = 1;
      pixelSizeTag->n[0] = 3;
      pixelSizeTag->value = malloc( sizeof(float) * 3 );
      ipPicAddTag (picImage, pixelSizeTag);
    }
    ((float*)pixelSizeTag->value)[0] = spacing[0];
    ((float*)pixelSizeTag->value)[1] = spacing[1];
    ((float*)pixelSizeTag->value)[2] = spacing[2];
    //set tag "ISG"
    ipPicTSV_t *geometryTag;
    geometryTag = ipPicQueryTag( picImage, "ISG" );
    if (!geometryTag)
    {
      geometryTag = (ipPicTSV_t *) malloc( sizeof(ipPicTSV_t) );
      geometryTag->type = ipPicFloat;
      geometryTag->bpe = 32;
      strcpy(geometryTag->tag, "ISG");
      geometryTag->dim = 2;
      geometryTag->n[0] = 3;
      geometryTag->n[1] = 4;
      geometryTag->value = malloc( sizeof(float) * 3 * 4 );
      ipPicAddTag (picImage, geometryTag);
    }
    const mitk::AffineTransform3D::OffsetType& offset = slicedGeometry->GetIndexToWorldTransform()->GetOffset();
    ((float*)geometryTag->value)[0] = offset[0];
    ((float*)geometryTag->value)[1] = offset[1];
    ((float*)geometryTag->value)[2] = offset[2];

    const mitk::AffineTransform3D::MatrixType& matrix = slicedGeometry->GetIndexToWorldTransform()->GetMatrix();
    const mitk::AffineTransform3D::MatrixType::ValueType* row0 = matrix[0];
    const mitk::AffineTransform3D::MatrixType::ValueType* row1 = matrix[1];
    const mitk::AffineTransform3D::MatrixType::ValueType* row2 = matrix[2];

    mitk::Vector3D v;

    mitk::FillVector3D(v, row0[0], row1[0], row2[0]);
    v.Normalize();
    ((float*)geometryTag->value)[3] = v[0];
    ((float*)geometryTag->value)[4] = v[1];
    ((float*)geometryTag->value)[5] = v[2];

    mitk::FillVector3D(v, row0[1], row1[1], row2[1]);
    v.Normalize();
    ((float*)geometryTag->value)[6] = v[0];
    ((float*)geometryTag->value)[7] = v[1];
    ((float*)geometryTag->value)[8] = v[2];

    ((float*)geometryTag->value)[9] = spacing[0];
    ((float*)geometryTag->value)[10] = spacing[1];
    ((float*)geometryTag->value)[11] = spacing[2];
  }
  mitk::PicFileReader::ConvertHandedness(picImage);
  ipPicPut((char*)(m_FileName.c_str()), picImage);
  mitk::PicFileReader::ConvertHandedness(picImage);
}

void mitk::PicFileWriter::SetInput( mitk::Image* image )
{
  this->ProcessObject::SetNthInput( 0, image );
}

const mitk::Image* mitk::PicFileWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return static_cast< const mitk::Image * >( this->ProcessObject::GetInput( 0 ) );
  }
}
