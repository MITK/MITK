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


#include <mitkGL.h>

#include "mitkSurfaceGLMapper2D.h"
#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkSurface.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkVtkScalarModeProperty.h"
#include "mitkAbstractTransformGeometry.h"
#include "mitkLookupTableProperty.h"

#include <vtkPolyData.h>
#include <vtkPlane.h>
#include <vtkCutter.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkLinearTransform.h>
#include <vtkAbstractMapper.h>
#include <vtkPKdTree.h>
#include <vtkStripper.h>


mitk::SurfaceGLMapper2D::SurfaceGLMapper2D()
: m_Plane( vtkPlane::New() ),
  m_Cutter( vtkCutter::New() ),
  m_LUT( vtkLookupTable::New() ),
  m_PointLocator( vtkPKdTree::New() ),
  m_Stripper( vtkStripper::New() ),
  m_DrawNormals(false),
  m_FrontNormalLengthInPixels(10.0),
  m_BackNormalLengthInPixels(10.0)
{
  // default for normals on front side = green
  m_FrontSideColor[0] = 0.0;
  m_FrontSideColor[1] = 1.0;
  m_FrontSideColor[2] = 0.0;
  m_FrontSideColor[3] = 1.0;

  // default for normals on back side = red
  m_BackSideColor[0] = 1.0;
  m_BackSideColor[1] = 0.0;
  m_BackSideColor[2] = 0.0;
  m_BackSideColor[3] = 1.0;

  // default for line color = yellow
  m_LineColor[0] = 1.0;
  m_LineColor[1] = 1.0;
  m_LineColor[2] = 0.0;
  m_LineColor[3] = 1.0;

  m_Cutter->SetCutFunction(m_Plane);
  m_Cutter->GenerateValues(1,0,1);

  m_LUT->SetTableRange(0,255);
  m_LUT->SetNumberOfColors(255);
  m_LUT->SetRampToLinear();
  m_LUT->Build();
}

mitk::SurfaceGLMapper2D::~SurfaceGLMapper2D()
{
  m_Plane->Delete();
  m_Cutter->Delete();
  m_LUT->Delete();
  m_PointLocator->Delete();
  m_Stripper->Delete();
}

const mitk::Surface *mitk::SurfaceGLMapper2D::GetInput(void)
{
  if(m_Surface.IsNotNull())
    return m_Surface;

  return static_cast<const Surface * > ( GetDataNode()->GetData() );
}

void mitk::SurfaceGLMapper2D::SetDataNode( mitk::DataNode* node )
{
  Superclass::SetDataNode( node );

  bool useCellData;
  if (dynamic_cast<BoolProperty *>(node->GetProperty("deprecated useCellDataForColouring")) == NULL)
    useCellData = false;
  else
    useCellData = dynamic_cast<BoolProperty *>(node->GetProperty("deprecated useCellDataForColouring"))->GetValue();

  if (!useCellData)
  {
    // search min/max point scalars over all time steps
    double dataRange[2] = {0,0};
    double range[2];

    Surface::Pointer input  = const_cast< Surface* >(dynamic_cast<const Surface*>( this->GetDataNode()->GetData() ));
    if(input.IsNull()) return;
    const TimeGeometry::Pointer inputTimeGeometry = input->GetTimeGeometry();
    if(( inputTimeGeometry.IsNull() ) || ( inputTimeGeometry->CountTimeSteps() == 0 ) ) return;
    for (unsigned int timestep=0; timestep<inputTimeGeometry->CountTimeSteps(); timestep++)
    {
      vtkPolyData * vtkpolydata = input->GetVtkPolyData( timestep );
      if((vtkpolydata==NULL) || (vtkpolydata->GetNumberOfPoints() < 1 )) continue;
      vtkDataArray *vpointscalars = vtkpolydata->GetPointData()->GetScalars();
      if (vpointscalars)
      {
        if(vpointscalars->GetLookupTable())
        {
          // load vtk lookup table if there is one for the scalar data
          m_LUT->DeepCopy(vpointscalars->GetLookupTable());
        }
        else
        {
          vpointscalars->GetRange( range, 0 );
          if (dataRange[0]==0 && dataRange[1]==0)
          {
            dataRange[0] = range[0];
            dataRange[1] = range[1];
          }
          else
          {
            if (range[0] < dataRange[0]) dataRange[0] = range[0];
            if (range[1] > dataRange[1]) dataRange[1] = range[1];
          }
        }
      }
    }
    if (dataRange[1] - dataRange[0] > 0) {
      m_LUT->SetTableRange( dataRange );
      m_LUT->Build();
    }
  }
}


void mitk::SurfaceGLMapper2D::Paint(mitk::BaseRenderer * renderer)
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, "visible");
  if(!visible) return;

  Surface::Pointer input  = const_cast<Surface*>(this->GetInput());

  if(input.IsNull())
    return;

  //
  // get the TimeGeometry of the input object
  //
  const TimeGeometry* inputTimeGeometry = input->GetTimeGeometry();
  if(( inputTimeGeometry == NULL ) || ( inputTimeGeometry->CountTimeSteps() == 0 ) )
    return;

  m_LineWidth = 1;
  GetDataNode()->GetIntProperty("line width", m_LineWidth, renderer);

  //
  // get the world time
  //
  ScalarType time =renderer->GetTime();
  int timestep=0;

  if( time > itk::NumericTraits<mitk::ScalarType>::NonpositiveMin() )
    timestep = inputTimeGeometry->TimePointToTimeStep( time );

 // int timestep = this->GetTimestep();

  if( inputTimeGeometry->IsValidTimeStep( timestep ) == false )
    return;

  vtkPolyData * vtkpolydata = input->GetVtkPolyData( timestep );
  if((vtkpolydata==NULL) || (vtkpolydata->GetNumberOfPoints() < 1 ))
    return;

  //apply color and opacity read from the PropertyList
  this->ApplyAllProperties(renderer);

  if (m_DrawNormals)
  {
    m_PointLocator->SetDataSet( vtkpolydata );
    m_PointLocator->BuildLocatorFromPoints( vtkpolydata->GetPoints() );
  }

  if(vtkpolydata!=NULL)
  {
    Point3D point;
    Vector3D normal;

    //Check if Lookup-Table is already given, else use standard one.
    double* scalarLimits = m_LUT->GetTableRange();
    double scalarsMin = scalarLimits[0], scalarsMax = scalarLimits[1];

    vtkLookupTable *lut;

    LookupTableProperty::Pointer lookupTableProp;
    this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
    if (lookupTableProp.IsNotNull() )
    {
      lut = lookupTableProp->GetLookupTable()->GetVtkLookupTable();

      GetDataNode()->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);
      GetDataNode()->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

      // check if the scalar range has been changed, e.g. manually, for the data tree node, and rebuild the LUT if necessary.
      double* oldRange = lut->GetTableRange();
      if( oldRange[0] != scalarsMin || oldRange[1] != scalarsMax )
      {
        lut->SetTableRange(scalarsMin, scalarsMax);
        lut->Build();
      }
    }
    else
    {
      lut = m_LUT;
    }

    vtkLinearTransform * vtktransform = GetDataNode()->GetVtkTransform(timestep);
    PlaneGeometry::ConstPointer worldGeometry = renderer->GetCurrentWorldPlaneGeometry();
    assert( worldGeometry.IsNotNull() );
    if (worldGeometry.IsNotNull())
    {
      // set up vtkPlane according to worldGeometry
      point=worldGeometry->GetOrigin();
      normal=worldGeometry->GetNormal(); normal.Normalize();
      m_Plane->SetTransform((vtkAbstractTransform*)NULL);
    }
    else
    {
      AbstractTransformGeometry::ConstPointer worldAbstractGeometry = dynamic_cast<const AbstractTransformGeometry*>(renderer->GetCurrentWorldPlaneGeometry());
      if(worldAbstractGeometry.IsNotNull())
      {
        AbstractTransformGeometry::ConstPointer surfaceAbstractGeometry = dynamic_cast<const AbstractTransformGeometry*>(input->GetTimeGeometry()->GetGeometryForTimeStep(0).GetPointer());
        if(surfaceAbstractGeometry.IsNotNull()) //@todo substitude by operator== after implementation, see bug id 28
        {
          PaintCells(renderer, vtkpolydata, worldGeometry, renderer->GetDisplayGeometry(), vtktransform, lut);
          return;
        }
        else
        {
          //@FIXME: does not work correctly. Does m_Plane->SetTransform really transforms a "flat plane" into a "curved plane"?
          return;
          // set up vtkPlane according to worldGeometry
          point=const_cast<BoundingBox*>(worldAbstractGeometry->GetParametricBoundingBox())->GetMinimum();
          FillVector3D(normal, 0, 0, 1);
          m_Plane->SetTransform(worldAbstractGeometry->GetVtkAbstractTransform()->GetInverse());
        }
      }
      else
        return;
    }

    double vp[3], vnormal[3];

    vnl2vtk(point.GetVnlVector(), vp);
    vnl2vtk(normal.GetVnlVector(), vnormal);

    //normally, we would need to transform the surface and cut the transformed surface with the cutter.
    //This might be quite slow. Thus, the idea is, to perform an inverse transform of the plane instead.
    //@todo It probably does not work for scaling operations yet:scaling operations have to be
    //dealed with after the cut is performed by scaling the contour.
    vtkLinearTransform * inversetransform = vtktransform->GetLinearInverse();
    inversetransform->TransformPoint(vp, vp);
    inversetransform->TransformNormalAtPoint(vp, vnormal, vnormal);

    m_Plane->SetOrigin(vp);
    m_Plane->SetNormal(vnormal);

    //set data into cutter
    m_Cutter->SetInputData(vtkpolydata);
    m_Cutter->Update();
    //    m_Cutter->GenerateCutScalarsOff();
    //    m_Cutter->SetSortByToSortByCell();

    if (m_DrawNormals)
    {
      m_Stripper->SetInputData( m_Cutter->GetOutput() );
      // calculate the cut
      m_Stripper->Update();
      PaintCells(renderer, m_Stripper->GetOutput(), worldGeometry, renderer->GetDisplayGeometry(), vtktransform, lut, vtkpolydata);
    }
    else
    {
      PaintCells(renderer, m_Cutter->GetOutput(), worldGeometry, renderer->GetDisplayGeometry(), vtktransform, lut, vtkpolydata);
    }
  }
}

void mitk::SurfaceGLMapper2D::PaintCells(mitk::BaseRenderer* renderer, vtkPolyData* contour,
                                       const PlaneGeometry* worldGeometry,
                                       const DisplayGeometry* displayGeometry,
                                       vtkLinearTransform * vtktransform,
                                       vtkLookupTable *lut,
                                       vtkPolyData* original3DObject)
{
  // deprecated settings
  bool usePointData = false;

  bool useCellData = false;
  this->GetDataNode()->GetBoolProperty("deprecated useCellDataForColouring", useCellData, renderer);

  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility, renderer);

  if(scalarVisibility)
  {
    VtkScalarModeProperty* scalarMode;
    if(this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer))
    {
      if( (scalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_POINT_DATA) ||
        (scalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_DEFAULT) )
      {
        usePointData = true;
      }
      if(scalarMode->GetVtkScalarMode() == VTK_SCALAR_MODE_USE_CELL_DATA)
      {
        useCellData = true;
      }
    }
    else
    {
      usePointData = true;
    }
  }

  vtkPoints    *vpoints = contour->GetPoints();
  vtkDataArray *vpointscalars = contour->GetPointData()->GetScalars();

  vtkCellArray *vlines  = contour->GetLines();
  vtkDataArray* vcellscalars = contour->GetCellData()->GetScalars();

  Point3D p; Point2D p2d, last;
  int i, j;
  int numberOfLines = vlines->GetNumberOfCells();

  glLineWidth( m_LineWidth );
  glBegin (GL_LINES);

  glColor4fv(m_LineColor);

  double distanceSinceLastNormal(0.0);

  vlines->InitTraversal();
  for(i=0;i<numberOfLines;++i)
  {
    vtkIdType *cell(NULL);
    vtkIdType cellSize(0);
    double vp[3];

    vlines->GetNextCell(cellSize, cell);

    vpoints->GetPoint(cell[0], vp);
    //take transformation via vtktransform into account
    vtktransform->TransformPoint(vp, vp);
    vtk2itk(vp, p);

    //convert 3D point (in mm) to 2D point on slice (also in mm)
    worldGeometry->Map(p, p2d);

    //convert point (until now mm and in world coordinates) to display coordinates (units )
    displayGeometry->WorldToDisplay(p2d, p2d);
    last=p2d;

    for(j=1; j<cellSize; ++j)
    {
      vpoints->GetPoint(cell[j], vp);
      Point3D originalPoint;
      vtk2itk(vp, originalPoint);
      //take transformation via vtktransform into account
      vtktransform->TransformPoint(vp, vp);
      vtk2itk(vp, p);

      //convert 3D point (in mm) to 2D point on slice (also in mm)
      worldGeometry->Map(p, p2d);

      //convert point (until now mm and in world coordinates) to display coordinates (units )
      displayGeometry->WorldToDisplay(p2d, p2d);

      double color[3];
      if (useCellData && vcellscalars != NULL )
      {
        // color each cell according to cell data
        lut->GetColor( vcellscalars->GetComponent(i,0),color);
        glColor3f(color[0],color[1],color[2]);
        glVertex2f(last[0], last[1]);
        glVertex2f(p2d[0], p2d[1]);
      }
      else if (usePointData && vpointscalars != NULL )
      {
        lut->GetColor( vpointscalars->GetComponent(cell[j-1],0),color);
        glColor3f(color[0],color[1],color[2]);
        glVertex2f(last[0], last[1]);
        lut->GetColor( vpointscalars->GetComponent(cell[j],0),color);
        glColor3f(color[0],color[1],color[2]);
        glVertex2f(p2d[0], p2d[1]);
      }
      else
      {
        glVertex2f(last[0], last[1]);
        glVertex2f(p2d[0], p2d[1]);

        // draw normals ?
        if (m_DrawNormals && original3DObject)
        {
          distanceSinceLastNormal += sqrt((p2d[0]-last[0])*(p2d[0]-last[0]) + (p2d[1]-last[1])*(p2d[1]-last[1]));
          if (distanceSinceLastNormal >= 5.0)
          {
            distanceSinceLastNormal = 0.0;

            vtkPointData* pointData = original3DObject->GetPointData();
            if (!pointData) break;

            vtkDataArray* normalsArray = pointData->GetNormals();
            if (!normalsArray) break;

            // find 3D point closest to the currently drawn point
            double distance(0.0);
            vtkIdType closestPointId = m_PointLocator->FindClosestPoint(originalPoint[0], originalPoint[1], originalPoint[2], distance);
            if (closestPointId >= 0)
            {
              // find normal of 3D object at this 3D point
              double* normal = normalsArray->GetTuple3(closestPointId);
              double transformedNormal[3];
              vtktransform->TransformNormal(normal, transformedNormal);

              Vector3D normalITK;
              vtk2itk(transformedNormal, normalITK);
              normalITK.Normalize();

              // calculate a point (point from the cut 3D object) + (normal vector of closest point)
              Point3D tip3D = p + normalITK;

              // map this point into our 2D coordinate system
              Point2D tip2D;
              worldGeometry->Map(tip3D, tip2D);

              displayGeometry->WorldToDisplay(tip2D, tip2D);

              // calculate 2D vector from point to point+normal, normalize it to standard length
              Vector2D tipVectorGLFront = tip2D - p2d;
              tipVectorGLFront.Normalize();
              tipVectorGLFront *= m_FrontNormalLengthInPixels;

              Vector2D tipVectorGLBack = p2d - tip2D;
              tipVectorGLBack.Normalize();
              tipVectorGLBack *= m_BackNormalLengthInPixels;

              Point2D tipPoint2D = p2d + tipVectorGLFront;
              Point2D backTipPoint2D = p2d + tipVectorGLBack;

              // draw normalized mapped normal vector
              glColor4f(m_BackSideColor[0], m_BackSideColor[1], m_BackSideColor[2], m_BackSideColor[3]); // red backside
              glVertex2f(p2d[0], p2d[1]);
              glVertex2f(tipPoint2D[0], tipPoint2D[1]);
              glColor4f(m_FrontSideColor[0], m_FrontSideColor[1], m_FrontSideColor[2], m_FrontSideColor[3]); // green backside
              glVertex2f(p2d[0], p2d[1]);
              glVertex2f(backTipPoint2D[0], backTipPoint2D[1]);
              glColor4fv(m_LineColor); // back to line color
            }
          }
        }
      }
      last=p2d;
    }
  }

  glEnd();
  glLineWidth(1.0);
}

void mitk::SurfaceGLMapper2D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "line width", IntProperty::New(2), renderer, overwrite );
  node->AddProperty( "scalar mode", VtkScalarModeProperty::New(), renderer, overwrite );
  node->AddProperty( "draw normals 2D", BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "invert normals", BoolProperty::New(false), renderer, overwrite );
  node->AddProperty( "front color", ColorProperty::New(0.0, 1.0, 0.0), renderer, overwrite );
  node->AddProperty( "back color", ColorProperty::New(1.0, 0.0, 0.0), renderer, overwrite );
  node->AddProperty( "front normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite );
  node->AddProperty( "back normal lenth (px)", FloatProperty::New(10.0), renderer, overwrite );
  node->AddProperty( "layer", mitk::IntProperty::New(100), renderer, overwrite);
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::SurfaceGLMapper2D::ApplyAllProperties(mitk::BaseRenderer* renderer)
{
  ApplyColorAndOpacityProperties(renderer);

  DataNode * node = GetDataNode();

  if(node == NULL)
  {
    return;
  }

  node->GetBoolProperty("draw normals 2D", m_DrawNormals, renderer);

  // check for color and opacity properties, use it for rendering if they exists
  node->GetColor(m_LineColor, renderer, "color");
  node->GetOpacity(m_LineColor[3], renderer, "opacity");

  bool invertNormals(false);
  node->GetBoolProperty("invert normals", invertNormals, renderer);

  if (!invertNormals)
  {
    node->GetColor(m_FrontSideColor, renderer, "front color");
    node->GetOpacity(m_FrontSideColor[3], renderer, "opacity");

    node->GetColor(m_BackSideColor, renderer, "back color");
    node->GetOpacity(m_BackSideColor[3], renderer, "opacity");

    node->GetFloatProperty( "front normal lenth (px)", m_FrontNormalLengthInPixels, renderer );
    node->GetFloatProperty( "back normal lenth (px)", m_BackNormalLengthInPixels, renderer );
  }
  else
  {
    node->GetColor(m_FrontSideColor, renderer, "back color");
    node->GetOpacity(m_FrontSideColor[3], renderer, "opacity");

    node->GetColor(m_BackSideColor, renderer, "front color");
    node->GetOpacity(m_BackSideColor[3], renderer, "opacity");

    node->GetFloatProperty( "back normal lenth (px)", m_FrontNormalLengthInPixels, renderer );
    node->GetFloatProperty( "front normal lenth (px)", m_BackNormalLengthInPixels, renderer );
  }
}
