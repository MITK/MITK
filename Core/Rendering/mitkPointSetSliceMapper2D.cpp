/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 11618 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkGL.h>
#include "mitkPointSetSliceMapper2D.h"

#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkUnstructuredGrid.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkColorProperty.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkProperties.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkBaseVtkMapper3D.h"

#include "vtkPointSetSlicer.h"

#include <vtkPointSet.h>
#include <vtkPointSetSource.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkLinearTransform.h>
#include <vtkActor.h>
#include <vtkVolume.h>
#include <vtkAssembly.h>
#include <vtkVolumeProperty.h>
#include <vtkAbstractMapper3D.h>
#include <vtkMapper.h>
#include <vtkAbstractVolumeMapper.h>
#include <vtkScalarsToColors.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>

#include <itkProcessObject.h>

void mitk::PointSetSliceMapper2D::GenerateData()
{
  mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
  if ( node.IsNull() )
    return;


  if (!node->GetProperty(m_ScalarMode, "scalar mode"))
  {
    m_ScalarMode = mitk::VtkScalarModeProperty::New(0);
  }

  if (!node->GetProperty(m_ScalarVisibility, "scalar visibility"))
  {
    m_ScalarVisibility = mitk::BoolProperty::New(true);
  }
  
  if (!node->GetProperty(m_Outline, "outline polygons"))
  {
    m_Outline = mitk::BoolProperty::New(false);
  }
  
  if (!node->GetProperty(m_Color, "color"))
  {
    m_Color = mitk::ColorProperty::New(1.0f, 1.0f, 1.0f);
  }
  
  if (!node->GetProperty(m_LineWidth, "line width"))
  {
    m_LineWidth = mitk::IntProperty::New(1);
  }

}

void mitk::PointSetSliceMapper2D::GenerateData( mitk::BaseRenderer* renderer )
{
  mitk::BaseData::Pointer input = const_cast<mitk::BaseData*>( this->GetData() );
  assert( input );

  input->Update();
  
  if (m_VtkPointSet) m_VtkPointSet->UnRegister(0);
  m_VtkPointSet = this->GetVtkPointSet(renderer);
  assert(m_VtkPointSet);
  m_VtkPointSet->Register(0);
    
  if (m_ScalarVisibility->GetValue())
  {
    mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
    mitk::TransferFunctionProperty::Pointer transferFuncProp;
    node->GetProperty(transferFuncProp, "TransferFunction", renderer);
    if (transferFuncProp.IsNotNull())
    {
      mitk::TransferFunction::Pointer tf = transferFuncProp->GetValue();
      if (m_ScalarsToColors) m_ScalarsToColors->UnRegister(0);
      m_ScalarsToColors = static_cast<vtkScalarsToColors*>(tf->GetColorTransferFunction());
      m_ScalarsToColors->Register(0);
      
      if (m_ScalarsToOpacity) m_ScalarsToOpacity->UnRegister(0);
      m_ScalarsToOpacity = tf->GetScalarOpacityFunction();
      m_ScalarsToOpacity->Register(0);
    }
    else
    {
      if (m_ScalarsToColors) m_ScalarsToColors->UnRegister(0);
      m_ScalarsToColors = this->GetVtkLUT(renderer);
      assert(m_ScalarsToColors);
      m_ScalarsToColors->Register(0);
      
      float opacity;
      node->GetOpacity(opacity, renderer);
      if (m_ScalarsToOpacity) m_ScalarsToOpacity->UnRegister(0);
      m_ScalarsToOpacity = vtkPiecewiseFunction::New();
      double range[2];
      m_VtkPointSet->GetScalarRange(range);
      m_ScalarsToOpacity->AddSegment(range[0], opacity, range[1], opacity);
    }
  }
}

void mitk::PointSetSliceMapper2D::Paint( mitk::BaseRenderer* renderer )
{
  if ( IsVisible( renderer ) == false )
    return ;

  vtkLinearTransform * vtktransform = GetDataTreeNode()->GetVtkTransform();
  vtkLinearTransform * inversetransform = vtktransform->GetLinearInverse();

  Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
  PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>( worldGeometry.GetPointer() );

  Point3D point;
  Vector3D normal;

  if(worldPlaneGeometry.IsNotNull())
  {
    // set up vtkPlane according to worldGeometry
    point=worldPlaneGeometry->GetOrigin();
    normal=worldPlaneGeometry->GetNormal(); normal.Normalize();
    m_Plane->SetTransform((vtkAbstractTransform*)NULL);
  }
  else
  {
    //@FIXME: does not work correctly. Does m_Plane->SetTransform really transforms a "plane plane" into a "curved plane"?
    return;
    AbstractTransformGeometry::ConstPointer worldAbstractGeometry = dynamic_cast<const AbstractTransformGeometry*>(renderer->GetCurrentWorldGeometry2D());
    if(worldAbstractGeometry.IsNotNull())
    {
      // set up vtkPlane according to worldGeometry
      point=const_cast<mitk::BoundingBox*>(worldAbstractGeometry->GetParametricBoundingBox())->GetMinimum();
      FillVector3D(normal, 0, 0, 1);
      m_Plane->SetTransform(worldAbstractGeometry->GetVtkAbstractTransform()->GetInverse());
    }
    else
      return;
  }

  vtkFloatingPointType vp[ 3 ], vnormal[ 3 ];

  vnl2vtk(point.Get_vnl_vector(), vp);
  vnl2vtk(normal.Get_vnl_vector(), vnormal);

  //normally, we would need to transform the surface and cut the transformed surface with the cutter.
  //This might be quite slow. Thus, the idea is, to perform an inverse transform of the plane instead.
  //@todo It probably does not work for scaling operations yet:scaling operations have to be
  //dealed with after the cut is performed by scaling the contour.
  inversetransform->TransformPoint( vp, vp );
  inversetransform->TransformNormalAtPoint( vp, vnormal, vnormal );

  m_Plane->SetOrigin( vp );
  m_Plane->SetNormal( vnormal );

  // set data into cutter
  m_Slicer->SetInput( m_VtkPointSet );
  //    m_Cutter->GenerateCutScalarsOff();
  //    m_Cutter->SetSortByToSortByCell();

  // calculate the cut
  m_Slicer->Update();

  // fetch geometry
  mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
  assert( displayGeometry );
  //  float toGL=displayGeometry->GetSizeInDisplayUnits()[1];

  //apply color and opacity read from the PropertyList
  ApplyProperties( renderer );

  // traverse the cut contour
  vtkPolyData * contour = m_Slicer->GetOutput();

  vtkPoints *vpoints = contour->GetPoints();
  vtkCellArray *vlines = contour->GetLines();
  vtkCellArray *vpolys = contour->GetPolys();
  vtkPointData *vpointdata = contour->GetPointData();
  vtkDataArray* vscalars = vpointdata->GetScalars();

  vtkCellData *vcelldata = contour->GetCellData();
  vtkDataArray* vcellscalars = vcelldata->GetScalars();

  int numberOfLines = contour->GetNumberOfLines();
  int numberOfPolys = contour->GetNumberOfPolys();

  Point3D p;
  Point2D p2d, last, first;

  vlines->InitTraversal();
  vpolys->InitTraversal();
  
  for (int i = 0;i < numberOfLines;++i )
  {
    vtkIdType *cell(0);
    vtkIdType cellSize(0);

    vlines->GetNextCell( cellSize, cell );

    float rgba[4] = {1.0f, 1.0f, 1.0f, 0};
    if (m_ScalarVisibility->GetValue())
    {
      if ( m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_DEFAULT ||
           m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_CELL_DATA )
      {  // color each cell according to cell data
        double scalar = vcellscalars->GetComponent( i, 0 );
        double rgb[3] = { 1.0f, 1.0f, 1.0f };
        m_ScalarsToColors->GetColor(scalar, rgb);
        rgba[0] = (float)rgb[0];
        rgba[1] = (float)rgb[1];
        rgba[2] = (float)rgb[2];
        rgba[3] = (float)m_ScalarsToOpacity->GetValue(scalar);
      }
      else if ( m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_POINT_DATA )
      {
        double scalar = vscalars->GetComponent( i, 0 );
        double rgb[3] = { 1.0f, 1.0f, 1.0f };
        m_ScalarsToColors->GetColor(scalar, rgb);
        rgba[0] = (float)rgb[0];
        rgba[1] = (float)rgb[1];
        rgba[2] = (float)rgb[2];
        rgba[3] = (float)m_ScalarsToOpacity->GetValue(scalar);
      }
    }
    
    glColor4fv( rgba );
    

    glBegin ( GL_LINE_LOOP );
    for ( int j = 0;j < cellSize;++j )
    {
      vpoints->GetPoint( cell[ j ], vp );
      //take transformation via vtktransform into account
      vtktransform->TransformPoint( vp, vp );

      vtk2itk( vp, p );

      //convert 3D point (in mm) to 2D point on slice (also in mm)
      worldGeometry->Map( p, p2d );

      //convert point (until now mm and in worldcoordinates) to display coordinates (units )
      displayGeometry->WorldToDisplay( p2d, p2d );

      //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
      //p2d[1]=toGL-p2d[1];

      //add the current vertex to the line
      glVertex2f( p2d[0], p2d[1] );
    }
    glEnd ();

  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  for (int i = 0;i < numberOfPolys;++i )
  {
    vtkIdType *cell(0);
    vtkIdType cellSize(0);

    vpolys->GetNextCell( cellSize, cell );

    float rgba[4] = {1.0f, 1.0f, 1.0f, 0};
    if (m_ScalarVisibility->GetValue())
    {
      if ( m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_DEFAULT ||
            m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_CELL_DATA )
      {  // color each cell according to cell data
        double scalar = vcellscalars->GetComponent( i, 0 );
        double rgb[3] = { 1.0f, 1.0f, 1.0f };
        m_ScalarsToColors->GetColor(scalar, rgb);
        rgba[0] = (float)rgb[0];
        rgba[1] = (float)rgb[1];
        rgba[2] = (float)rgb[2];
        rgba[3] = (float)m_ScalarsToOpacity->GetValue(scalar);
      }
      else if (m_ScalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_POINT_DATA )
      {
        double scalar = vscalars->GetComponent( i, 0 );
        double rgb[3] = { 1.0f, 1.0f, 1.0f };
        m_ScalarsToColors->GetColor(scalar, rgb);
        rgba[0] = (float)rgb[0];
        rgba[1] = (float)rgb[1];
        rgba[2] = (float)rgb[2];
        rgba[3] = (float)m_ScalarsToOpacity->GetValue(scalar);
        glColor4fv( rgba );
      }
    }
    glColor4fv( rgba );

    //Point2D points[cellSize];
    glBegin( GL_POLYGON );
    for (int j = 0; j < cellSize; ++j)
    {
      vpoints->GetPoint( cell[ j ], vp );
      //take transformation via vtktransform into account
      vtktransform->TransformPoint( vp, vp );

      vtk2itk( vp, p );

      //convert 3D point (in mm) to 2D point on slice (also in mm)
      worldGeometry->Map( p, p2d );

      //convert point (until now mm and in worldcoordinates) to display coordinates (units )
      displayGeometry->WorldToDisplay( p2d, p2d );

      //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
      //p2d[1]=toGL-p2d[1];

      //add the current vertex to the line
      glVertex2f( p2d[0], p2d[1] );
    }
    glEnd();
  }
  glDisable(GL_BLEND);
  
  if (m_Outline->GetValue())
  {
    vpolys->InitTraversal();
    mitk::Color  color = m_Color->GetColor();
    glColor4f(color[0], color[1], color[2], 1.0f);
    glLineWidth((float)m_LineWidth->GetValue());
    for (int i = 0;i < numberOfPolys;++i )
    {
      vtkIdType *cell(0);
      vtkIdType cellSize(0);

      vpolys->GetNextCell( cellSize, cell );

      //Point2D points[cellSize];
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glBegin( GL_POLYGON );
      //glPolygonOffset(1.0, 1.0);
      for (int j = 0; j < cellSize; ++j)
      {
        vpoints->GetPoint( cell[ j ], vp );
        //take transformation via vtktransform into account
        vtktransform->TransformPoint( vp, vp );

        vtk2itk( vp, p );

        //convert 3D point (in mm) to 2D point on slice (also in mm)
        worldGeometry->Map( p, p2d );

        //convert point (until now mm and in worldcoordinates) to display coordinates (units )
        displayGeometry->WorldToDisplay( p2d, p2d );

        //convert display coordinates ( (0,0) is top-left ) in GL coordinates ( (0,0) is bottom-left )
        //p2d[1]=toGL-p2d[1];

        //add the current vertex to the line
        glVertex2f( p2d[0], p2d[1] );
      }
      glEnd();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }
}


vtkAbstractMapper3D* 
mitk::PointSetSliceMapper2D
::GetVtkAbstractMapper3D(mitk::BaseRenderer * renderer)
{
  //std::cout << "GETVTKABSTRACTMAPPER3D\n";
  mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
  if ( node.IsNull() )
    return 0;

  mitk::BaseVtkMapper3D::Pointer mitkMapper = dynamic_cast< mitk::BaseVtkMapper3D* > ( node->GetMapper( 2 ) );
  if ( mitkMapper.IsNull() )
  {
    return 0;
  }

  mitkMapper->Update(renderer);

  vtkActor* actor = dynamic_cast<vtkActor*>( mitkMapper->GetProp() );
  if (actor)
  {
    return dynamic_cast<vtkAbstractMapper3D*>( actor->GetMapper() );
  }
  vtkVolume* volume = dynamic_cast<vtkVolume*>( mitkMapper->GetProp() );
  if (volume)
  {
    return dynamic_cast<vtkAbstractMapper3D*>( volume->GetMapper() );
  }

  return 0;
}



vtkPointSet*
mitk::PointSetSliceMapper2D
::GetVtkPointSet(mitk::BaseRenderer* renderer)
{
  //std::cout << "GETVTKPOINTSET\n";
  vtkAbstractMapper3D * abstractMapper = GetVtkAbstractMapper3D(renderer);
  if ( abstractMapper == 0 )
  {
    // try to get data from the node
    mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
    if ( node.IsNull() )
      return 0;
    mitk::BaseData::Pointer data = node->GetData();
    mitk::UnstructuredGrid::Pointer grid = dynamic_cast<mitk::UnstructuredGrid*>(data.GetPointer());
    if (!grid.IsNull())
      return static_cast<vtkPointSet*>(grid->GetVtkUnstructuredGrid());

      return 0;
  }
  else
  {
    vtkMapper* mapper = dynamic_cast<vtkMapper*>(abstractMapper);
    if (mapper)
    {
      return dynamic_cast<vtkPointSet*>(mapper->GetInput());
    }
    vtkAbstractVolumeMapper* volMapper = dynamic_cast<vtkAbstractVolumeMapper*>(abstractMapper);
    if (volMapper)
    {
      return dynamic_cast<vtkPointSet*>(volMapper->GetDataSetInput());
    }
  }

  return 0;
}



vtkScalarsToColors* mitk::PointSetSliceMapper2D::GetVtkLUT(mitk::BaseRenderer* renderer)
{
  //std::cout << "GETVTKLUT\n";
  vtkMapper * mapper = dynamic_cast<vtkMapper*>(GetVtkAbstractMapper3D(renderer));
  if (mapper)
    return mapper->GetLookupTable();
  else
  {
    mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
    if ( node.IsNull() )
      return 0;

    mitk::BaseVtkMapper3D::Pointer mitkMapper = dynamic_cast< mitk::BaseVtkMapper3D* > ( node->GetMapper( 2 ) );
    if ( mitkMapper.IsNull() )
    {
      //std::cout << "mitkMapper is null\n";
      return 0;
    }

    mitkMapper->Update(renderer);

    vtkVolume* volume = dynamic_cast<vtkVolume*>( mitkMapper->GetProp() );
    if (volume)
    {
      //std::cout << "found volume prop\n";
      return static_cast<vtkScalarsToColors*>(volume->GetProperty()->GetRGBTransferFunction());
    }
    
    vtkAssembly* assembly = dynamic_cast<vtkAssembly*>(mitkMapper->GetProp());
    if (assembly)
    {
      //std::cout << "found assembly prop\n";
      mitk::TransferFunctionProperty::Pointer transferFuncProp;
      node->GetProperty(transferFuncProp, "TransferFunction", 0);
      if (transferFuncProp.IsNotNull())
      {
        std::cout << "return colortransferfunction\n";
        return static_cast<vtkScalarsToColors*>(transferFuncProp->GetValue()->GetColorTransferFunction());
      }
    }
    return 0;
  }
}


bool mitk::PointSetSliceMapper2D::IsConvertibleToVtkPointSet(mitk::BaseRenderer * renderer)
{
    return ( GetVtkPointSet(renderer) != 0 );
}

mitk::PointSetSliceMapper2D::PointSetSliceMapper2D()
{
    m_Plane = vtkPlane::New();
    m_Slicer = vtkPointSetSlicer::New();

    m_Slicer->SetSlicePlane( m_Plane );
    
    m_ScalarsToColors = 0;
    m_ScalarsToOpacity = 0;
    m_VtkPointSet = 0;

    //m_LUT = vtkLookupTable::New();
    //m_LUT->SetTableRange( 0, 255 );
    //m_LUT->SetNumberOfColors( 255 );
    //m_LUT->SetRampToLinear ();
    //m_LUT->Build();
}



mitk::PointSetSliceMapper2D::~PointSetSliceMapper2D()
{
  m_Slicer->Delete();
  m_Plane->Delete();
  
  if (m_ScalarsToOpacity != 0) m_ScalarsToOpacity->UnRegister(0);
  if (m_ScalarsToColors != 0) m_ScalarsToColors->UnRegister(0);
  m_VtkPointSet->UnRegister(0);
}

