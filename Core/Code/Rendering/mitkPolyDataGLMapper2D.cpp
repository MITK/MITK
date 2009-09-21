/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include <mitkGL.h>
#include "mitkPolyDataGLMapper2D.h"

#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkSurface.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkBaseVtkMapper3D.h"

#include <vtkPolyData.h>
#include <vtkPolyDataSource.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPolyData.h>
#include <vtkLinearTransform.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkScalarsToColors.h>

#include <itkProcessObject.h>

void mitk::PolyDataGLMapper2D::Paint( mitk::BaseRenderer * renderer )
{
    if ( IsVisible( renderer ) == false )
        return ;

    // ok, das ist aus GenerateData kopiert
    mitk::BaseData::Pointer input = const_cast<mitk::BaseData*>( GetData() );

    assert( input );

    input->Update();

    vtkPolyData * vtkpolydata = this->GetVtkPolyData();
    assert( vtkpolydata );


    vtkLinearTransform * vtktransform = GetDataTreeNode() ->GetVtkTransform();

    if (vtktransform)
    {
      vtkLinearTransform * inversetransform = vtktransform->GetLinearInverse();

      Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
      PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>( worldGeometry.GetPointer() );

      if ( vtkpolydata != NULL )
      {
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
          m_Cutter->SetInput( vtkpolydata );
          //    m_Cutter->GenerateCutScalarsOff();
          //    m_Cutter->SetSortByToSortByCell();

          // calculate the cut
          m_Cutter->Update();

          // fetch geometry
          mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
          assert( displayGeometry );
          //  float toGL=displayGeometry->GetSizeInDisplayUnits()[1];

          //apply color and opacity read from the PropertyList
          ApplyProperties( renderer );

          // traverse the cut contour
          vtkPolyData * contour = m_Cutter->GetOutput();

          vtkPoints *vpoints = contour->GetPoints();
          vtkCellArray *vpolys = contour->GetLines();
          vtkPointData *vpointdata = contour->GetPointData();
          vtkDataArray* vscalars = vpointdata->GetScalars();

          vtkCellData *vcelldata = contour->GetCellData();
          vtkDataArray* vcellscalars = vcelldata->GetScalars();

          int i, numberOfCells = vpolys->GetNumberOfCells();

          Point3D p;
          Point2D p2d, last, first;

          vpolys->InitTraversal();
          vtkScalarsToColors* lut = GetVtkLUT();
          assert ( lut != NULL );

          for ( i = 0;i < numberOfCells;++i )
          {
              vtkIdType *cell(NULL);
              vtkIdType cellSize(0);

              vpolys->GetNextCell( cellSize, cell );

              if ( m_ColorByCellData )
              {  // color each cell according to cell data
                vtkFloatingPointType* color = lut->GetColor( vcellscalars->GetComponent( i, 0 ) );
                glColor3f( color[ 0 ], color[ 1 ], color[ 2 ] );
              }
              if ( m_ColorByPointData )
              {
                vtkFloatingPointType* color = lut->GetColor( vscalars->GetComponent( cell[0], 0 ) );
                glColor3f( color[ 0 ], color[ 1 ], color[ 2 ] );
              }

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
      }
    }
}





vtkPolyDataMapper* mitk::PolyDataGLMapper2D::GetVtkPolyDataMapper()
{
    return NULL;
    /*

    mitk::DataTreeNode::ConstPointer node = this->GetDataTreeNode();
    if ( node.IsNull() )
        return NULL;

    mitk::BaseVtkMapper3D::Pointer mitkMapper = dynamic_cast< mitk::BaseVtkMapper3D* > ( node->GetMapper( 2 ) );
    if ( mitkMapper.IsNull() )
        return NULL;

    mitkMapper->Update(NULL);

    vtkActor* actor = dynamic_cast<vtkActor*>( mitkMapper->GetVtkProp(0) );

    if ( actor == NULL )
        return NULL;

    return dynamic_cast<vtkPolyDataMapper*>( actor->GetMapper() );
  */
}



vtkPolyData* mitk::PolyDataGLMapper2D::GetVtkPolyData( )
{
    vtkPolyDataMapper * polyDataMapper = GetVtkPolyDataMapper();
    if ( polyDataMapper == NULL )
        return NULL;
    else
        return polyDataMapper->GetInput();
}



vtkScalarsToColors* mitk::PolyDataGLMapper2D::GetVtkLUT( )
{
    vtkPolyDataMapper * polyDataMapper = GetVtkPolyDataMapper();
    if ( polyDataMapper == NULL )
        return NULL;
    else
        return polyDataMapper->GetLookupTable();
}


bool mitk::PolyDataGLMapper2D::IsConvertibleToVtkPolyData()
{
    return ( GetVtkPolyDataMapper() != NULL );
}

mitk::PolyDataGLMapper2D::PolyDataGLMapper2D()
{
    m_Plane = vtkPlane::New();
    m_Cutter = vtkCutter::New();

    m_Cutter->SetCutFunction( m_Plane );
    m_Cutter->GenerateValues( 1, 0, 1 );

    m_ColorByCellData = false;
    m_ColorByPointData = false;

    //m_LUT = vtkLookupTable::New();
    //m_LUT->SetTableRange( 0, 255 );
    //m_LUT->SetNumberOfColors( 255 );
    //m_LUT->SetRampToLinear ();
    //m_LUT->Build();
}



mitk::PolyDataGLMapper2D::~PolyDataGLMapper2D()
{}

