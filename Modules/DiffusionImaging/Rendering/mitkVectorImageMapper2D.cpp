#include "mitkVectorImageMapper2D.h"

//vtk related includes
#include <vtkPlane.h>
#include <vtkLookupTable.h>
#include <vtkScalarsToColors.h>
#include <vtkImageReslice.h>
#include <vtkPolyData.h>
#include <vtkGlyph2D.h>
#include <vtkGlyphSource2D.h>
#include <vtkPolyData.h>
#include <vtkPolyDataSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
#include <vtkLinearTransform.h>
#include <vtkMatrixToLinearTransform.h>
#include <vtkLookupTable.h>
#include <vtkScalarsToColors.h>
#include <vtkTransform.h>
#include <vtkImageData.h>
#include <vtkDataSetWriter.h>
#include <vtkMaskedGlyph3D.h>
#include <vtkMaskedGlyph2D.h>
#include <vtkMatrix4x4.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkIndent.h>
#include <vtkDataObject.h>

#include <fstream>

//mitk related includes
#include "mitkGL.h"
#include "mitkBaseRenderer.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkAbstractTransformGeometry.h"
#include <mitkLookupTableProperty.h>

const mitk::Image * mitk::VectorImageMapper2D::GetInput( void )
{
  if ( m_Image.IsNotNull() )
    return m_Image;
  else
    return dynamic_cast<const mitk::Image*>( this->GetData() );
}


void mitk::VectorImageMapper2D::Paint( mitk::BaseRenderer * renderer )
{
  //std::cout << "2d vector mapping..." << std::endl;
  if ( IsVisible( renderer ) == false )
    return ;

  mitk::Image::Pointer input = const_cast<mitk::Image*>( this->GetInput() );

  if ( input.IsNull() )
    return ;


  mitk::PlaneGeometry::Pointer worldPlaneGeometry2D = dynamic_cast< mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>( renderer->GetCurrentWorldGeometry2D() ) );
  assert( worldPlaneGeometry2D.IsNotNull() );

  vtkImageData* vtkImage = input->GetVtkImageData( this->GetCurrentTimeStep( input, renderer ) );

  //
  // set up the cutter orientation according to the current geometry of
  // the renderers plane
  //
  Point3D point;
  Vector3D normal;
  Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
  PlaneGeometry::ConstPointer worldPlaneGeometry = dynamic_cast<const PlaneGeometry*>( worldGeometry.GetPointer() );

  if ( worldPlaneGeometry.IsNotNull() )
  {
    // set up vtkPlane according to worldGeometry
    point = worldPlaneGeometry->GetOrigin();
    normal = worldPlaneGeometry->GetNormal(); normal.Normalize();
    m_Plane->SetTransform( (vtkAbstractTransform*)NULL );
  }
  else
  {
    itkWarningMacro( << "worldPlaneGeometry is NULL!" );
    return ;
  }

  vtkFloatingPointType vp[ 3 ], vp_slice[ 3 ], vnormal[ 3 ];
  vnl2vtk( point.Get_vnl_vector(), vp );
  vnl2vtk( normal.Get_vnl_vector(), vnormal );
  //std::cout << "Origin: " << vp[0] <<" "<< vp[1] <<" "<< vp[2] << std::endl;
  //std::cout << "Normal: " << vnormal[0] <<" "<< vnormal[1] <<" "<< vnormal[2] << std::endl;

  //normally, we would need to transform the surface and cut the transformed surface with the cutter.
  //This might be quite slow. Thus, the idea is, to perform an inverse transform of the plane instead.
  //@todo It probably does not work for scaling operations yet:scaling operations have to be
  //dealed with after the cut is performed by scaling the contour.
  vtkLinearTransform * vtktransform = GetDataNode() ->GetVtkTransform();

  vtkTransform* world2vtk = vtkTransform::New();
  world2vtk->Identity();
  world2vtk->Concatenate(vtktransform->GetLinearInverse());
  double myscale[3];
  world2vtk->GetScale(myscale);
  world2vtk->PostMultiply();
  world2vtk->Scale(1/myscale[0],1/myscale[1],1/myscale[2]);
  world2vtk->TransformPoint( vp, vp );
  world2vtk->TransformNormalAtPoint( vp, vnormal, vnormal );
  world2vtk->Delete();

    // vtk works in axis align coords
  // thus the normal also must be axis align, since 
  // we do not allow arbitrary cutting through volume
  //
  // vnormal should already be axis align, but in order
  // to get rid of precision effects, we set the two smaller
  // components to zero here
  int dims[3];
  vtkImage->GetDimensions(dims);
  double spac[3];
  vtkImage->GetSpacing(spac);
  vp_slice[0] = vp[0];
  vp_slice[1] = vp[1];
  vp_slice[2] = vp[2];
  if(fabs(vnormal[0]) > fabs(vnormal[1]) && fabs(vnormal[0]) > fabs(vnormal[2]) )
  {
    if(fabs(vp_slice[0]/spac[0]) < 0.4)
      vp_slice[0] = 0.4*spac[0];
    if(fabs(vp_slice[0]/spac[0]) > (dims[0]-1)-0.4)
      vp_slice[0] = ((dims[0]-1)-0.4)*spac[0];
    vnormal[1] = 0;
    vnormal[2] = 0;
  }

  if(fabs(vnormal[1]) > fabs(vnormal[0]) && fabs(vnormal[1]) > fabs(vnormal[2]) )
  {
    if(fabs(vp_slice[1]/spac[1]) < 0.4)
      vp_slice[1] = 0.4*spac[1];
    if(fabs(vp_slice[1]/spac[1]) > (dims[1]-1)-0.4)
      vp_slice[1] = ((dims[1]-1)-0.4)*spac[1];
    vnormal[0] = 0;
    vnormal[2] = 0;
  }

  if(fabs(vnormal[2]) > fabs(vnormal[1]) && fabs(vnormal[2]) > fabs(vnormal[0]) )
  {
    if(fabs(vp_slice[2]/spac[2]) < 0.4)
      vp_slice[2] = 0.4*spac[2];
    if(fabs(vp_slice[2]/spac[2]) > (dims[2]-1)-0.4)
      vp_slice[2] = ((dims[2]-1)-0.4)*spac[2];
    vnormal[0] = 0;
    vnormal[1] = 0;
  }


  m_Plane->SetOrigin( vp_slice );
  m_Plane->SetNormal( vnormal );

  vtkPolyData* cuttedPlane;
  if(!( (dims[0] == 1 && vnormal[0] != 0) || 
        (dims[1] == 1 && vnormal[1] != 0) ||
        (dims[2] == 1 && vnormal[2] != 0) ))
  {
    m_Cutter->SetCutFunction( m_Plane );
    m_Cutter->SetInput( vtkImage );
    m_Cutter->GenerateCutScalarsOff();//!
    m_Cutter->Update();
    cuttedPlane = m_Cutter->GetOutput();
  }
  else
  {
    // cutting of a 2D-Volume does not work, 
    // so we have to build up our own polydata object
    cuttedPlane = vtkPolyData::New();
    vtkPoints* points = vtkPoints::New();
    points->SetNumberOfPoints(vtkImage->GetNumberOfPoints());
    for(int i=0; i<vtkImage->GetNumberOfPoints(); i++)
      points->SetPoint(i, vtkImage->GetPoint(i));
    cuttedPlane->SetPoints(points);
    vtkFloatArray* pointdata = vtkFloatArray::New();
    int comps  = vtkImage->GetPointData()->GetScalars()->GetNumberOfComponents();
    pointdata->SetNumberOfComponents(comps);
    int tuples = vtkImage->GetPointData()->GetScalars()->GetNumberOfTuples();
    pointdata->SetNumberOfTuples(tuples);
    for(int i=0; i<tuples; i++)
      pointdata->SetTuple(i,vtkImage->GetPointData()->GetScalars()->GetTuple(i));
    pointdata->SetName( "vector" );
    cuttedPlane->GetPointData()->AddArray(pointdata);
  }

  if ( cuttedPlane->GetNumberOfPoints() != 0)
  {
    //
    // make sure, that we have point data with more than 1 component (as vectors)
    //
    vtkPointData * pointData = cuttedPlane->GetPointData();
    if ( pointData == NULL )
    {
      itkWarningMacro( << "no point data associated with cutters result!" );
      return ;
    }
    if ( pointData->GetNumberOfArrays() == 0 )
    {
      itkWarningMacro( << "point data returned by cutter doesn't have any arrays associated!" );
      return ;
    }
    else if ( pointData->GetArray(0)->GetNumberOfComponents() <= 1)
    {
      itkWarningMacro( << "number of components <= 1!" );
      return;
    }
    else if ( pointData->GetArrayName( 0 ) == NULL )
    {
      pointData->GetArray( 0 ) ->SetName( "vector" );
      //std::cout << "array name = vectors now" << std::endl;
    }
    //std::cout << "  projecting..."<< std::endl;

    //
    // constrain the vectors to lie on the plane, which means to remove the vector component,
    // which is orthogonal to the plane.
    //
    vtkIdType numPoints, pointId;
    numPoints = cuttedPlane->GetNumberOfPoints();
    vtkDataArray* inVectors = cuttedPlane->GetPointData()->GetVectors( "vector" );
    assert( inVectors != NULL );
    vtkFloatArray* vectorMagnitudes = vtkFloatArray::New();
    vectorMagnitudes->SetName("vectorMagnitudes");
    vectorMagnitudes->SetNumberOfComponents(1);
    vectorMagnitudes->SetNumberOfValues(numPoints);
    vectorMagnitudes->SetNumberOfTuples(numPoints);
    vtkFloatingPointType inVector[ 3 ], outVector[3], wnormal[3]; //, tmpVector[ 3 ], outVector[ 3 ];
    vtkFloatingPointType k = 0.0;
    vnl2vtk( normal.Get_vnl_vector(), wnormal );
    vtkMath::Normalize( wnormal );
    bool normalizeVecs;
    m_DataNode->GetBoolProperty( "NormalizeVecs", normalizeVecs );
    for ( pointId = 0; pointId < numPoints; ++pointId )
    {
      inVectors->GetTuple( pointId, inVector );
      if(normalizeVecs)
      {
        vnl_vector<double> tmp(3);
        vtk2vnl(inVector, tmp);
        tmp.normalize();
        vnl2vtk(tmp, inVector);
      }
      k = vtkMath::Dot( wnormal, inVector );
      // Remove non orthogonal component.
      outVector[ 0 ] = inVector[ 0 ] - ( wnormal[ 0 ] * k );
      outVector[ 1 ] = inVector[ 1 ] - ( wnormal[ 1 ] * k );
      outVector[ 2 ] = inVector[ 2 ] - ( wnormal[ 2 ] * k );
      inVectors->SetTuple( pointId, outVector );

      // ?? this was set to norm(inVector) before, but outVector made more sense to me
      vectorMagnitudes->SetValue( pointId, vtkMath::Norm( outVector ) );


      //std::cout << "method old: " << inVector[0] <<", " << inVector[1] << ", "<<inVector[2] << ", method new: " << outVector[0] << ", "<< outVector[1] << ", "<< outVector[2] << std::endl;
    }
    pointData->AddArray(vectorMagnitudes);
    pointData->CopyAllOn();

    //pointData->PrintSelf(std::cout, vtkIndent(4));
    //std::cout << "  ...done!"<< std::endl;
    //std::cout << "  glyphing..."<< std::endl;

    // call glyph2D to generate 2D glyphs for each of the
    // vectors
    vtkGlyphSource2D* glyphSource = vtkGlyphSource2D::New();
    //glyphSource->SetGlyphTypeToDash();
    glyphSource->DashOn();
    //glyphSource->SetScale( 0.1 );
    //glyphSource->SetScale2( .5 );
    //glyphSource->SetCenter( 0.5, 0.5, 0.5 );
    glyphSource->CrossOff();
    //glyphSource->FilledOff();
    //glyphSource->Update();

    double spacing[3];
    vtkImage->GetSpacing(spacing);
    double min = spacing[0];
    min = min > spacing[1] ? spacing[1] : min;
    min = min > spacing[2] ? spacing[2] : min;

    float scale = 1;
    mitk::FloatProperty::Pointer mitkScaleProp = dynamic_cast<mitk::FloatProperty*>(GetDataNode()->GetProperty("Scale"));
    if (mitkScaleProp.IsNotNull())
    {
      scale = mitkScaleProp->GetValue();
    }

    vtkMaskedGlyph3D* glyphGenerator = vtkMaskedGlyph3D::New();
    glyphGenerator->SetSource( glyphSource->GetOutput() );
    glyphGenerator->SetInputConnection(cuttedPlane->GetProducerPort());
    glyphGenerator->SetInputArrayToProcess (1, 0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS , "vector");
    glyphGenerator->SetVectorModeToUseVector();
    glyphGenerator->OrientOn();
    glyphGenerator->SetScaleFactor( min*scale );
    glyphGenerator->SetUseMaskPoints( true );
    glyphGenerator->SetRandomMode( true );
    glyphGenerator->SetMaximumNumberOfPoints( 128*128 );

    glyphGenerator->Update();

    vtkLookupTable* vtkLut = NULL;
    mitk::LookupTableProperty::Pointer mitkLutProp = dynamic_cast<mitk::LookupTableProperty*>(GetDataNode()->GetProperty("LookupTable"));
    if (mitkLutProp.IsNotNull())
    {
      vtkLut = mitkLutProp->GetLookupTable()->GetVtkLookupTable();
    }

    mitk::Color color;
    mitk::ColorProperty::Pointer mitkColorProp = dynamic_cast<mitk::ColorProperty*>(GetDataNode()->GetProperty("color"));
    if (mitkColorProp.IsNotNull())
    {
      color = mitkColorProp->GetColor();
    }
    else
    {
      color.SetRed(0);
      color.SetBlue(1);
      color.SetGreen(0);
    }

    float lwidth = 1;
    mitk::FloatProperty::Pointer mitkLWidthProp = dynamic_cast<mitk::FloatProperty*>(GetDataNode()->GetProperty("LineWidth"));
    if (mitkLWidthProp.IsNotNull())
    {
      lwidth = mitkLWidthProp->GetValue();
    }

    vtkTransform* trafo = vtkTransform::New();
    trafo->Identity();
    trafo->Concatenate(vtktransform);
    trafo->PreMultiply();
    double myscale[3];
    trafo->GetScale(myscale);
    trafo->Scale(1/myscale[0],1/myscale[1],1/myscale[2]);
    
    this->PaintCells( glyphGenerator->GetOutput(), renderer->GetCurrentWorldGeometry2D(), renderer->GetDisplayGeometry(), trafo, renderer, NULL/*vtkLut*/, color, lwidth, spacing );

    vectorMagnitudes->Delete();
    glyphSource->Delete();
    glyphGenerator->Delete();
    trafo->Delete();
  }
  else
  {
    std::cout << "  no points cutted!"<< std::endl;
  }
  //std::cout << "...done!" << std::endl;
}







void mitk::VectorImageMapper2D::PaintCells( vtkPolyData* glyphs, const Geometry2D* worldGeometry, const DisplayGeometry* displayGeometry, vtkLinearTransform* vtktransform, mitk::BaseRenderer*  /*renderer*/, vtkScalarsToColors *lut, mitk::Color color, float lwidth, vtkFloatingPointType *spacing )
{

  vtkPoints * points = glyphs->GetPoints();
  vtkPointData * vpointdata = glyphs->GetPointData();
  vtkDataArray* vpointscalars = vpointdata->GetArray("vectorMagnitudes");
  //vtkDataArray* vpointpositions = vpointdata->GetArray("pointPositions");
  assert(vpointscalars != NULL);
  //std::cout << "  Scalars range 2d:" << vpointscalars->GetRange()[0] << " " << vpointscalars->GetRange()[0] << std::endl;

  Point3D p;
  Point2D p2d;
  vtkIdList* idList;
  vtkCell* cell;

  vtkFloatingPointType offset[3]; 
  for (unsigned int i = 0; i < 3; ++i) 
  {
    offset[i] = 0; 
  }

  vtkIdType numCells = glyphs->GetNumberOfCells();
  for ( vtkIdType cellId = 0; cellId < numCells; ++cellId )
  {
    vtkFloatingPointType vp[ 3 ];

    cell = glyphs->GetCell( cellId );
    idList = cell->GetPointIds();

    int numPoints = idList->GetNumberOfIds();

    if(numPoints == 1)
    {
      //take transformation via vtktransform into account
      vtkFloatingPointType pos[ 3 ],vp_raster[3];
      points->GetPoint( idList->GetId( 0 ), vp );
      vp_raster[0] = vtkMath::Round(vp[0]/spacing[0])*spacing[0];
      vp_raster[1] = vtkMath::Round(vp[1]/spacing[1])*spacing[1];
      vp_raster[2] = vtkMath::Round(vp[2]/spacing[2])*spacing[2];
      vtktransform->TransformPoint( vp_raster, pos );
      offset[0] = pos[0] - vp[0];
      offset[1] = pos[1] - vp[1];
      offset[2] = pos[2] - vp[2];
    }
    else
    {
      glLineWidth(lwidth);
      glBegin ( GL_LINE_LOOP );
      
      for ( int pointNr = 0; pointNr < numPoints ;++pointNr )
      {
        points->GetPoint( idList->GetId( pointNr ), vp );

        vp[0] = vp[0] + offset[0];
        vp[1] = vp[1] + offset[1];
        vp[2] = vp[2] + offset[2];

        vtkFloatingPointType tmp[ 3 ];
        vtktransform->TransformPoint( vp,tmp );

        vtk2itk( vp, p );

        //convert 3D point (in mm) to 2D point on slice (also in mm)
        worldGeometry->Map( p, p2d );

        //convert point (until now mm and in worldcoordinates) to display coordinates (units )
        displayGeometry->WorldToDisplay( p2d, p2d );

        if ( lut != NULL )
        {
          // color each point according to point data
          vtkFloatingPointType * color;

          if ( vpointscalars != NULL )
          {
            vpointscalars->GetComponent( pointNr, 0 );
            color = lut->GetColor( vpointscalars->GetComponent( idList->GetId( pointNr ), 0 ) );
            glColor3f( color[ 0 ], color[ 1 ], color[ 2 ] );
          }
        }
        else
        {
          glColor3f( color.GetRed(), color.GetGreen(), color.GetBlue() );
        }

        //std::cout <<  idList->GetId( pointNr )<< ": " << p2d[0]<< " "<< p2d[1] << std::endl;
        //draw the line
        glVertex2f( p2d[ 0 ], p2d[ 1 ] );



      }
      glEnd ();
    }
  }
}


mitk::VectorImageMapper2D::VectorImageMapper2D()
{
  m_LUT = NULL;
  m_Plane = vtkPlane::New();
  m_Cutter = vtkCutter::New();

  m_Cutter->SetCutFunction( m_Plane );
  m_Cutter->GenerateValues( 1, 0, 1 );
}


mitk::VectorImageMapper2D::~VectorImageMapper2D()
{
  if ( m_LUT != NULL )
    m_LUT->Delete();
  if ( m_Plane != NULL )
    m_Plane->Delete();
  if ( m_Cutter != NULL )
    m_Cutter->Delete();
}

int mitk::VectorImageMapper2D::GetCurrentTimeStep( mitk::BaseData* data, mitk::BaseRenderer* renderer )
{
  //
  // get the TimeSlicedGeometry of the input object
  //
  const TimeSlicedGeometry * dataTimeGeometry = data->GetUpdatedTimeSlicedGeometry();
  if ( ( dataTimeGeometry == NULL ) || ( dataTimeGeometry->GetTimeSteps() == 0 ) )
  {
    itkWarningMacro( << "geometry of the given data object isn't a mitk::TimeSlicedGeometry, or the number of time steps is 0!" );
    return 0;
  }

  //
  // get the world time
  //
  Geometry2D::ConstPointer worldGeometry = renderer->GetCurrentWorldGeometry2D();
  assert( worldGeometry.IsNotNull() );
  ScalarType time = worldGeometry->GetTimeBounds() [ 0 ];

  //
  // convert the world time to time steps of the input object
  //
  int timestep = 0;
  if ( time > ScalarTypeNumericTraits::NonpositiveMin() )
    timestep = dataTimeGeometry->MSToTimeStep( time );
  if ( dataTimeGeometry->IsValidTime( timestep ) == false )
  {
    itkWarningMacro( << timestep << " is not a valid time of the given data object!" );
    return 0;
  }
  return timestep;
}

