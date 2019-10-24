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

#include "mitkVectorImageVtkGlyphMapper3D.h"

#include <vtkMaskedGlyph3D.h>
#include <vtkActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkArrowSource.h>
#include <vtkLineSource.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>

#include <mitkImageVtkAccessor.h>


/*
* Constructor. Doesn't do anything...
*/
mitk::VectorImageVtkGlyphMapper3D::VectorImageVtkGlyphMapper3D()
{
  m_RandomMode = true;
  m_UseMaskPoints = true;
  m_MaximumNumberOfPoints = 5000;
  m_GlyphType = ArrowGlyph;
  m_Glyph3DGenerator = vtkMaskedGlyph3D::New();
  m_Glyph3DMapper = vtkOpenGLPolyDataMapper::New();
  m_Glyph3DActor = vtkActor::New();
}

vtkProp* mitk::VectorImageVtkGlyphMapper3D::GetVtkProp(mitk::BaseRenderer*  /*renderer*/)
{
  return m_Glyph3DActor;
}

/*
* Destructor
*/
mitk::VectorImageVtkGlyphMapper3D::~VectorImageVtkGlyphMapper3D()
{
  if ( m_Glyph3DMapper != NULL )
    m_Glyph3DMapper->Delete();
  if ( m_Glyph3DGenerator != NULL )
    m_Glyph3DGenerator->Delete();
}

/*
* Generate a vtkPolyData by creating vectors as glyphs
* This method is called, each time a specific renderer is updated.
*/
void mitk::VectorImageVtkGlyphMapper3D::GenerateDataForRenderer( mitk::BaseRenderer* renderer )
{

  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if ( !visible )
  {
    if ( m_Glyph3DActor != NULL )
      m_Glyph3DActor->VisibilityOff();
    return ;
  }
  else
  {
    if ( m_Glyph3DActor != NULL )
      m_Glyph3DActor->VisibilityOn();
  }

  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  bool needGenerateData = ls->IsGenerateDataRequired( renderer, this, GetDataNode() );

  if(!needGenerateData)
  {
    return;
  }

  ls->UpdateGenerateDataTime();

  //
  // get the input image...
  //
  mitk::Image::Pointer mitkImage = this->GetInput();
  if ( mitkImage.GetPointer() == NULL )
  {
    itkWarningMacro( << "VectorImage is null !" );
    return;
  }

  //
  // make sure, that the input image is an vector image
  //
  if ( mitkImage->GetPixelType().GetNumberOfComponents() <= 1 )
  {
    itkWarningMacro( << "VectorImage has only one scalar component!" );
    return ;
  }

  ImageVtkAccessor accessor(mitkImage);
  ImageAccessLock lock(&accessor);
  vtkImageData* vtkImage = accessor.getVtkImageData();

  //
  // make sure, that we have point data with more than 1 component (as vectors)
  //
  vtkPointData* pointData = vtkImage->GetPointData();
  if ( pointData == NULL )
  {
    itkWarningMacro( << "vtkImage->GetPointData() returns NULL!" );
    return ;
  }
  if ( pointData->GetNumberOfArrays() == 0 )
  {
    itkWarningMacro( << "vtkImage->GetPointData()->GetNumberOfArrays() is 0!" );
    return ;
  }
  else if ( pointData->GetArrayName( 0 ) == NULL )
  {
    vtkImage->GetPointData() ->GetArray( 0 ) ->SetName( "vector" );
  }

  if ( vtkImage->GetNumberOfPoints() != 0 )
  {

    //
    // create the glyph, which has to be shown at each point
    // of the masked image
    //
    vtkPolyData* glyph;
    if ( m_GlyphType == LineGlyph )
    {
      vtkLineSource * lineSource = vtkLineSource::New();
      lineSource->Update();
      glyph = lineSource->GetOutput();
    }
    else if ( m_GlyphType == ArrowGlyph )
    {
      vtkArrowSource * arrowSource = vtkArrowSource::New();
      arrowSource->Update();
      glyph = arrowSource->GetOutput();
    }
    else
    {
      // Use a vtkLineSource as default, if the GlyphType is
      // unknown
      itkWarningMacro( << "unknown glyph type!" );
      vtkLineSource * lineSource = vtkLineSource::New();
      lineSource->Update();
      glyph = lineSource->GetOutput();
    }
m_RandomMode = false;
m_UseMaskPoints = false;
m_MaximumNumberOfPoints = 80*80*80;
    //
    // set up the actual glyphing filter
    //
    m_Glyph3DGenerator->SetSourceData( glyph );
    m_Glyph3DGenerator->SetInput( vtkImage );
    //m_Glyph3DGenerator->SetInputConnection(m_Cutter->GetOutputPort());
    m_Glyph3DGenerator->SetInputArrayToProcess (1, 0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS , "vector");

    //m_Glyph3DGenerator->SelectInputVectors( vtkImage->GetPointData() ->GetArray( 0 ) ->GetName() );
    m_Glyph3DGenerator->OrientOn();
    m_Glyph3DGenerator->SetVectorModeToUseVector();
    m_Glyph3DGenerator->SetScaleFactor( 0.00392156862745 );
    m_Glyph3DGenerator->SetScaleModeToScaleByVector();
    m_Glyph3DGenerator->SetUseMaskPoints( m_UseMaskPoints );
    m_Glyph3DGenerator->SetRandomMode( m_RandomMode );
    m_Glyph3DGenerator->SetMaximumNumberOfPoints( m_MaximumNumberOfPoints );
    m_Glyph3DGenerator->Update();
    m_Glyph3DMapper->SetInputConnection( m_Glyph3DGenerator->GetOutputPort() );
    m_Glyph3DActor->SetMapper( m_Glyph3DMapper );

    if (GetDataNode()->GetProperty("LookupTable"))
    {
      mitk::LookupTable::Pointer mitkLookupTable = mitk::LookupTable::New();
      m_Glyph3DMapper->Update();
      mitkLookupTable->SetVtkLookupTable(dynamic_cast<vtkLookupTable*>(m_Glyph3DMapper->GetLookupTable()));
      mitk::LookupTableProperty::Pointer LookupTableProp = mitk::LookupTableProperty::New( mitkLookupTable );
      GetDataNode()->SetProperty( "LookupTable", LookupTableProp );
    }
    else
    {
      mitk::LookupTableProperty::Pointer mitkLutProp = dynamic_cast<mitk::LookupTableProperty*>(GetDataNode()->GetProperty("LookupTable"));
      if (mitkLutProp.IsNotNull())
        m_Glyph3DMapper->SetLookupTable( mitkLutProp->GetLookupTable()->GetVtkLookupTable() );

    }

    //vtkDataSetWriter* writer = vtkDataSetWriter::New();
    //writer->SetInput( vtkImage );
    //writer->SetFileName( "out.vtk" );
    //writer->Update();
  }

}


/*
* Returns the input data object of the given filter. In this
* case, a mitk::Image is returned.
*/
mitk::Image* mitk::VectorImageVtkGlyphMapper3D::GetInput()
{
  return const_cast<mitk::Image*>( dynamic_cast<mitk::Image*>( GetDataNode()->GetData() ) );
}

