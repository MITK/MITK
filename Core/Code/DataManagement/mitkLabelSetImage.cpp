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

#include "mitkLabelSetImage.h"

#include "mitkImageAccessByItk.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageReadAccessor.h"
#include "mitkLookupTableProperty.h"
#include "mitkPadImageFilter.h"

#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkCell.h>

#include <itkImageRegionIterator.h>
#include <itkQuadEdgeMesh.h>
#include <itkTriangleMeshToBinaryImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkBinaryMedianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

mitk::LabelSetImage::LabelSetImage() : mitk::Image(),
m_ActiveLayer(0)
{
  this->CreateDefaultLabelSet(m_ActiveLayer);
}

mitk::LabelSetImage::LabelSetImage(mitk::LabelSetImage* other) : mitk::Image(),
m_ActiveLayer(0)
{
  this->Initialize(other);
  m_LabelSetContainer.push_back( other->GetLabelSet() );
}

void mitk::LabelSetImage::Initialize(const mitk::Image* image)
{
  try
  {
    mitk::PixelType pixelType(mitk::MakeScalarPixelType<LabelSetPixelType>() );
    if (image->GetDimension() == 2)
    {
      const unsigned int dimensions[] = { image->GetDimension(0), image->GetDimension(1), 1 };
      Superclass::Initialize(pixelType, 3, dimensions);
    }
    else
    {
      Superclass::Initialize(pixelType, image->GetDimension(), image->GetDimensions());
    }

    mitk::TimeSlicedGeometry::Pointer originalGeometry = image->GetTimeSlicedGeometry()->Clone();
    this->SetGeometry( originalGeometry );
/*
    unsigned int byteSize = sizeof(LabelSetPixelType);
    for (unsigned int dim = 0; dim < this->GetDimension(); ++dim)
    {
      byteSize *= this->GetDimension(dim);
    }

    mitk::ImageWriteAccessor* accessor = new mitk::ImageWriteAccessor(static_cast<mitk::Image*>(this));
    memset( accessor->GetData(), 0, byteSize );
    delete accessor;
*/
//    LabelSetImageType::Pointer itkImage;
//    mitk::CastToItkImage(this, itkImage);

    VectorImageType::SizeType size;
    VectorImageType::IndexType index;
    VectorImageType::RegionType region;
    unsigned int* dimensions = this->GetDimensions();
    size[0] = dimensions[0];
    size[1] = dimensions[1];
    size[2] = dimensions[2];
    region.SetSize( size);
    index.Fill(0);
    region.SetIndex( index );

    m_VectorImage = VectorImageType::New();
    m_VectorImage->SetRegions( region );
    m_VectorImage->SetNumberOfComponentsPerPixel(1);
    m_VectorImage->Allocate();

    typedef itk::VariableLengthVector<LabelSetPixelType> VariableVectorType;
    VariableVectorType defaultValue;
    defaultValue.SetSize(1);
    defaultValue.Fill(0);
    m_VectorImage->FillBuffer(defaultValue);

    m_ImageToVectorAdaptor = ImageAdaptorType::New();
    m_ImageToVectorAdaptor->SetImage( m_VectorImage );
    m_ImageToVectorAdaptor->SetExtractComponentIndex( 0 );
    m_ImageToVectorAdaptor->Update();
    this->SetImportVolume(m_ImageToVectorAdaptor->GetBufferPointer(),0,0,Image::CopyMemory);
  }
  catch(mitk::Exception& e)
  {
    mitkReThrow(e) << "Could not generate a labelset image.";
  }
}

mitk::LabelSetImage::~LabelSetImage()
{
  m_LabelSetContainer.clear();
}

mitk::LabelSetImage::LabelSetPixelType* mitk::LabelSetImage::GetLayerBufferPointer(unsigned int layer)
{
  m_ImageToVectorAdaptor->SetExtractComponentIndex( layer );
  m_ImageToVectorAdaptor->Update();
  return m_ImageToVectorAdaptor->GetBufferPointer();
//  this->SetImportVolume(m_ImageToVectorAdaptor->GetBufferPointer(),0,0,Image::CopyMemory);
}
/*
mitk::ImageVtkAccessor* mitk::LabelSetImage::GetVtkImageData(int t, int n)
{
  if(m_Initialized==false)
  {
    if (GetSource().IsNull())
      return NULL;
    if (GetSource()->Updating()==false)
      GetSource()->UpdateOutputInformation();
  }


  unsigned int numberOfLayers = this->GetNumberOfLayers();

  for (int layer=0; layer<numberOfLayers; ++layer)
  {
      m_ImageToVectorAdaptor->SetExtractComponentIndex( layer );
      m_ImageToVectorAdaptor->Update();
      mitk::LabelSetPixelType* buffer = m_ImageToVectorAdaptor->GetBufferPointer();
  }

  if(m_VtkImageData==NULL)
    ConstructVtkImageData(iP);
  return m_VtkImageData;
}
*/
unsigned int mitk::LabelSetImage::GetActiveLayer()
{
  return m_ActiveLayer;
}

unsigned int mitk::LabelSetImage::GetNumberOfLayers()
{
  return m_LabelSetContainer.size();
}

void mitk::LabelSetImage::RemoveLayer()
{
  try
  {
    m_LabelSetContainer.erase( m_LabelSetContainer.begin() + m_ActiveLayer);
    this->SetActiveLayer( m_LabelSetContainer.size() - 1 );
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not remove the active layer. See error log for details.";
  }
}

void mitk::LabelSetImage::AddLayer()
{
  try
  {
    AccessByItk_1(this, ImageToVectorProcessing, m_ActiveLayer);

    // set the active layer to the latest available plus one
    m_ActiveLayer = m_LabelSetContainer.size();

    // push a new label set for the new layer
    this->CreateDefaultLabelSet(m_ActiveLayer);

    // create a new vector image
    VectorImageType::Pointer newVectorImage = VectorImageType::New();

    VectorImageType::SizeType size;
    VectorImageType::IndexType index;
    VectorImageType::RegionType region;
    unsigned int* dimensions = this->GetDimensions();
    size[0] = dimensions[0];
    size[1] = dimensions[1];
    size[2] = dimensions[2];
    region.SetSize( size );
    index.Fill(0);
    region.SetIndex( index );

    newVectorImage->SetRegions(region);
    newVectorImage->SetNumberOfComponentsPerPixel(m_LabelSetContainer.size());
    newVectorImage->Allocate();

    // fill inside
    VariableVectorType defaultValue;
    defaultValue.SetSize(m_LabelSetContainer.size());
    defaultValue.Fill(0);
    newVectorImage->FillBuffer(defaultValue);

    ImageAdaptorType::Pointer sourceAdaptor = ImageAdaptorType::New();
    sourceAdaptor->SetImage( m_VectorImage );

    ImageAdaptorType::Pointer targetAdaptor = ImageAdaptorType::New();
    targetAdaptor->SetImage( newVectorImage );

    // transfer vector components to new vector. Note that m_VectorImage has
    // one component less than newVectorImage
    for (int layer = 0; layer < m_LabelSetContainer.size() - 1; ++layer)
    {
      sourceAdaptor->SetExtractComponentIndex(layer);
      sourceAdaptor->Update();

      targetAdaptor->SetExtractComponentIndex(layer);
      targetAdaptor->Update();

      typedef itk::ImageRegionConstIterator< ImageAdaptorType > SourceIteratorType;
      typedef itk::ImageRegionIterator< ImageAdaptorType >      TargetIteratorType;

      SourceIteratorType sourceIter( sourceAdaptor, sourceAdaptor->GetLargestPossibleRegion() );
      sourceIter.GoToBegin();

      TargetIteratorType targetIter( targetAdaptor, targetAdaptor->GetLargestPossibleRegion() );
      targetIter.GoToBegin();

      while(!sourceIter.IsAtEnd())
      {
        targetIter.Set( sourceIter.Get() );
        ++sourceIter;
        ++targetIter;
      }
    }

    // take the new vector
    m_VectorImage = newVectorImage;

    AccessByItk_1(this, VectorToImageProcessing, m_ActiveLayer);
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not add a layer. See error log for details.";
  }

  this->Modified();
}

void mitk::LabelSetImage::SetActiveLayer(unsigned int layer)
{
  try
  {
    if ( (layer != m_ActiveLayer) && (layer >= 0) && (layer < m_LabelSetContainer.size()) )
    {
      AccessByItk_1(this, ImageToVectorProcessing, m_ActiveLayer);
      m_ActiveLayer = layer;
      AccessByItk_1(this, VectorToImageProcessing, m_ActiveLayer);

      this->Modified();
    }
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not set the active layer. See error log for details.";
  }
}

void mitk::LabelSetImage::CreateDefaultLabelSet(int layer)
{
  mitk::LabelSet::Pointer ls = mitk::LabelSet::New(layer);
  mitk::Color color;
  color.SetRed(0.0);
  color.SetGreen(0.0);
  color.SetBlue(0.0);
  mitk::Label::Pointer label = mitk::Label::New();
  label->SetColor(color);
  label->SetName("Exterior");
  label->SetExterior(true);
  label->SetOpacity(0.0);
  label->SetLocked(false);
  ls->AddLabel(*label);

  m_LabelSetContainer.push_back(ls);
}

void mitk::LabelSetImage::SetName(const std::string& name)
{
  m_LabelSetContainer[m_ActiveLayer]->SetName(name); // todo: fixme (why a name for the labelset?)
}

std::string mitk::LabelSetImage::GetLabelSetName()
{
  return m_LabelSetContainer[m_ActiveLayer]->GetName(); // todo: fixme
}

void mitk::LabelSetImage::SetLabelSetLastModified(const std::string& name)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLastModified(name);
}

std::string mitk::LabelSetImage::GetLabelSetLastModified()
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLastModified();
}

void mitk::LabelSetImage::CalculateLabelVolume(int index)
{
// todo
}

void mitk::LabelSetImage::SmoothLabel(int index)
{
  try
  {
    AccessByItk_1(this, SmoothLabelProcessing, index);
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not smooth the active label. See error log for details.";
  }
}

void mitk::LabelSetImage::EraseLabel(int index, bool reorder)
{
  try
  {
    AccessByItk_2(this, EraseLabelProcessing, index, reorder);
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not erase the active label. See error log for details.";
  }
}

void mitk::LabelSetImage::Concatenate(mitk::LabelSetImage* other)
{
  const unsigned int* otherDims = other->GetDimensions();
  const unsigned int* thisDims = this->GetDimensions();
  if ( (otherDims[0] != thisDims[0]) || (otherDims[1] != thisDims[1]) || (otherDims[2] != thisDims[2]) )
      mitkThrow() << "Dimensions do not match.";

  try
  {
    AccessByItk_1(this, ConcatenateProcessing, other);

    mitk::LabelSet::ConstPointer ls = other->GetConstLabelSet();
    for( int i=1; i<ls->GetNumberOfLabels(); i++) // skip exterior
    {
      this->AddLabel( *ls->GetLabel(i) );
    }
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not concatenate the two labelset images. See error log for details.";
  }
}

void mitk::LabelSetImage::ClearBuffer()
{
  try
  {
    AccessByItk(this, ClearBufferProcessing);
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    mitkThrow() << "Could not clear the internal buffer. See error log for details.";
  }
}

void mitk::LabelSetImage::AddLabel(const mitk::Label& label)
{
  m_LabelSetContainer[m_ActiveLayer]->AddLabel(label);
  this->Modified();
  AddLabelEvent.Send();
}

void mitk::LabelSetImage::AddLabel(const std::string& name, const mitk::Color& color)
{
  m_LabelSetContainer[m_ActiveLayer]->AddLabel(name, color);
  this->Modified();
  AddLabelEvent.Send();
}

const mitk::Color& mitk::LabelSetImage::GetLabelColor(int index)
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelColor(index);
}

void mitk::LabelSetImage::SetLabelColor(int index, const mitk::Color& color)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

float mitk::LabelSetImage::GetLabelOpacity(int index)
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelOpacity(index);
}

float mitk::LabelSetImage::GetLabelVolume(int index)
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelVolume(index);
}

void mitk::LabelSetImage::SetLabelOpacity(int index, float value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelOpacity(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelVolume(int index, float value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelVolume(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::RenameLabel(int index, const std::string& name, const mitk::Color& color)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelName(index, name);
  m_LabelSetContainer[m_ActiveLayer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelName(int index, const std::string& name)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelName(index, name);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetAllLabelsLocked(bool value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetAllLabelsLocked(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::SetAllLabelsVisible(bool value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetAllLabelsVisible(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::RemoveLabel(int index)
{
  this->EraseLabel(index, true);
  m_LabelSetContainer[m_ActiveLayer]->RemoveLabel(index);
  this->ResetLabels();
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::ResetLabels()
{
  m_LabelSetContainer[m_ActiveLayer]->ResetLabels();
}

void mitk::LabelSetImage::MergeLabels(std::vector<int>& indexes, int index)
{
  this->SetActiveLabel( index, false );
  for (int idx=0; idx<indexes.size(); idx++)
  {
    AccessByItk_1(this, MergeLabelsProcessing, indexes[idx]);
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::RemoveLabels(std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());

  for (int i=0; i<indexes.size(); i++)
  {
    this->EraseLabel(indexes[i], false);
  }

  for (int i=0; i<indexes.size(); i++)
  {
//    this->EraseLabel(indexes[i]-i, true);
    m_LabelSetContainer[m_ActiveLayer]->RemoveLabel(indexes[i]-i);
    this->ResetLabels();
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::EraseLabels(std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());

  for (int i=0; i<indexes.size(); i++)
  {
    this->EraseLabel(indexes[i], false);
  }
}

void mitk::LabelSetImage::SmoothLabels(std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());

  for (int i=0; i<indexes.size(); i++)
  {
    this->SmoothLabel(indexes[i]);
  }
}

std::string mitk::LabelSetImage::GetLabelName(int index)
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelName(index);
}

unsigned int mitk::LabelSetImage::GetActiveLabelIndex() const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel()->GetIndex();
}

unsigned int mitk::LabelSetImage::GetLabelLayer(int index) const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelLayer(index);
}

unsigned int mitk::LabelSetImage::GetActiveLabelLayer() const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel()->GetLayer();
}

const mitk::Label* mitk::LabelSetImage::GetActiveLabel() const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel();
}

const mitk::Color& mitk::LabelSetImage::GetActiveLabelColor() const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel()->GetColor();
}

bool mitk::LabelSetImage::GetLabelLocked(int index) const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelLocked(index);
}

bool mitk::LabelSetImage::GetLabelSelected(int index) const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelSelected(index);
}

mitk::LabelSet::ConstPointer mitk::LabelSetImage::GetConstLabelSet() const
{
  return m_LabelSetContainer[m_ActiveLayer].GetPointer();
}

mitk::LabelSet* mitk::LabelSetImage::GetLabelSet()
{
  return m_LabelSetContainer[m_ActiveLayer].GetPointer();
}

const mitk::LookupTable* mitk::LabelSetImage::GetLookupTable()
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLookupTable();
}

void mitk::LabelSetImage::SetLabelSet(mitk::LabelSet* labelset)
{
  if (m_LabelSetContainer[m_ActiveLayer] != labelset)
  {
    m_LabelSetContainer[m_ActiveLayer] = labelset;
    this->ResetLabels();
  }
}

bool mitk::LabelSetImage::IsLabelSelected(mitk::Label::Pointer label)
{
  return label->GetSelected();
}

bool mitk::LabelSetImage::GetLabelVisible(int index) const
{
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelVisible(index);
}

void mitk::LabelSetImage::SetLabelVisible(int index, bool value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelVisible(index, value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassIndex(int index, bool forceUpdate)
{
  if (forceUpdate)
  {
    AccessByItk_1( this, CalculateCenterOfMassProcessing, index );
  }
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelCenterOfMassIndex(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassCoordinates(int index, bool forceUpdate)
{
  if (forceUpdate)
  {
    AccessByItk_1( this, CalculateCenterOfMassProcessing, index );
  }
  return m_LabelSetContainer[m_ActiveLayer]->GetLabelCenterOfMassCoordinates(index);
}

void mitk::LabelSetImage::SetActiveLabel(int index, bool sendEvent)
{
  m_LabelSetContainer[m_ActiveLayer]->SetActiveLabel(index);

  if (sendEvent)
  {
    ModifyLabelEvent.Send(index);
  }
  this->Modified();
}

void mitk::LabelSetImage::SetLabelLocked(int index, bool value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelLocked(index, value);
  this->Modified();
}

void mitk::LabelSetImage::SetLabelSelected(int index, bool value)
{
  m_LabelSetContainer[m_ActiveLayer]->SetLabelSelected(index, value);
  this->Modified();
}

void mitk::LabelSetImage::RemoveAllLabels()
{
  m_LabelSetContainer[m_ActiveLayer]->RemoveAllLabels();
  this->ClearBuffer();
  this->Modified();
  RemoveLabelEvent.Send();
}

unsigned int mitk::LabelSetImage::GetNumberOfLabels()
{
  return m_LabelSetContainer[m_ActiveLayer]->GetNumberOfLabels();
}

void mitk::LabelSetImage::MaskStamp(mitk::Image* mask, bool forceOverwrite)
{
  if (!mask)
  {
    MITK_ERROR << "Input mask is NULL.";
    return;
  }

  try
  {
    mitk::PadImageFilter::Pointer padImageFilter = mitk::PadImageFilter::New();
    padImageFilter->SetInput(0, mask);
    padImageFilter->SetInput(1, this);
    padImageFilter->SetPadConstant(0);
    padImageFilter->SetBinaryFilter(false);
    padImageFilter->SetLowerThreshold(0);
    padImageFilter->SetUpperThreshold(1);

    padImageFilter->Update();

    mitk::Image::Pointer paddedMask = padImageFilter->GetOutput();

    if (paddedMask.IsNull()) return;

    AccessByItk_2(this, MaskStampProcessing, paddedMask, forceOverwrite);
  }
  catch(...)
  {
    mitkThrow() << "Could not stamp the provided mask on the selected label.";
  }
}

mitk::Image::Pointer mitk::LabelSetImage::CreateLabelMask(int index)
{
  mitk::Image::Pointer mask = mitk::Image::New();
  try
  {
    mask->Initialize(this);

    unsigned int byteSize = sizeof(LabelSetPixelType);
    for (unsigned int dim = 0; dim < mask->GetDimension(); ++dim)
    {
      byteSize *= mask->GetDimension(dim);
    }

    mitk::ImageWriteAccessor* accessor = new mitk::ImageWriteAccessor(static_cast<mitk::Image*>(mask));
    memset( accessor->GetData(), 0, byteSize );
    delete accessor;

    mitk::TimeSlicedGeometry::Pointer geometry = this->GetTimeSlicedGeometry()->Clone();
    mask->SetGeometry( geometry );

    AccessByItk_2(this, CreateLabelMaskProcessing, mask, index);
  }
  catch(...)
  {
    mitkThrow() << "Could not create a mask out of the selected label.";
  }

  return mask;
}

void mitk::LabelSetImage::SurfaceStamp(mitk::Surface* surface, bool forceOverwrite)
{
  if (!surface)
  {
    MITK_ERROR << "Input surface is NULL.";
    return;
  }

  try
  {
    LabelSetImageType::Pointer itkImage;
    mitk::CastToItkImage(this, itkImage);

    vtkPolyData *polydata = surface->GetVtkPolyData();

    vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
    transform->SetMatrix( surface->GetGeometry()->GetVtkTransform()->GetMatrix() );
    transform->Update();

    vtkSmartPointer<vtkTransformPolyDataFilter> transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    transformer->SetInput(polydata);
    transformer->SetTransform(transform);
    transformer->Update();

    typedef double Coord;
    typedef itk::QuadEdgeMesh< Coord, 3 > MeshType;

    MeshType::Pointer mesh = MeshType::New();
    mesh->SetCellsAllocationMethod( MeshType::CellsAllocatedDynamicallyCellByCell );
    unsigned int numberOfPoints = polydata->GetNumberOfPoints();
    mesh->GetPoints()->Reserve( numberOfPoints );

    vtkPoints* points = polydata->GetPoints();

    MeshType::PointType point;
    for( int i=0; i < numberOfPoints; i++ )
    {
      double* aux = points->GetPoint(i);
      point[0] = aux[0];
      point[1] = aux[1];
      point[2] = aux[2];
      mesh->SetPoint( i, point );
    }

    // Load the polygons into the itk::Mesh
    typedef MeshType::CellAutoPointer     CellAutoPointerType;
    typedef MeshType::CellType            CellType;
    typedef itk::TriangleCell< CellType > TriangleCellType;
    typedef MeshType::PointIdentifier     PointIdentifierType;
    typedef MeshType::CellIdentifier      CellIdentifierType;

    // Read the number of polygons
    CellIdentifierType numberOfPolygons = 0;
    numberOfPolygons = polydata->GetNumberOfPolys();
    vtkCellArray* polys = polydata->GetPolys();

    PointIdentifierType numberOfCellPoints = 3;
    CellIdentifierType i = 0;

    for (i=0; i<numberOfPolygons; i++)
    {
      vtkIdList *cellIds;
      vtkCell *vcell = polydata->GetCell(i);
      cellIds = vcell->GetPointIds();

      CellAutoPointerType cell;
      TriangleCellType * triangleCell = new TriangleCellType;
      PointIdentifierType k;
      for( k = 0; k < numberOfCellPoints; k++ )
      {
        triangleCell->SetPointId( k, cellIds->GetId(k) );
      }

      cell.TakeOwnership( triangleCell );
      mesh->SetCell( i, cell );
    }

    typedef itk::TriangleMeshToBinaryImageFilter<MeshType, LabelSetImageType> TriangleMeshToBinaryImageFilterType;

    TriangleMeshToBinaryImageFilterType::Pointer filter = TriangleMeshToBinaryImageFilterType::New();
    filter->SetInput(mesh);
    filter->SetInfoImage(itkImage);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);
    filter->Update();

    LabelSetImageType::Pointer resultImage = filter->GetOutput();
    resultImage->DisconnectPipeline();

    typedef itk::ImageRegionConstIterator< LabelSetImageType > SourceIteratorType;
    typedef itk::ImageRegionIterator< LabelSetImageType > TargetIteratorType;

    SourceIteratorType sourceIter( resultImage, resultImage->GetLargestPossibleRegion() );
    sourceIter.GoToBegin();

    TargetIteratorType targetIter( itkImage, itkImage->GetLargestPossibleRegion() );
    targetIter.GoToBegin();

    int activeLabel = this->GetActiveLabelIndex();

    while ( !sourceIter.IsAtEnd() )
    {
      int sourceValue = static_cast<int>(sourceIter.Get());
      int targetValue =  static_cast<int>(targetIter.Get());

      if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabelLocked(targetValue)) ) // skip exterior and locked labels
      {
        targetIter.Set( activeLabel );
      }
      ++sourceIter;
      ++targetIter;
    }

    this->Modified();
  }
  catch(...)
  {
    mitkThrow() << "Could not stamp the provided surface.";
  }
}

void mitk::LabelSetImage::InitializeByLabeledImage(mitk::Image::Pointer image)
{
  if (image.IsNull() || image->IsEmpty() || !image->IsInitialized())
    mitkThrow() << "Invalid labeled image.";

  try
  {
    this->Initialize(image);

    unsigned int byteSize = sizeof(LabelSetPixelType);
    for (unsigned int dim = 0; dim < image->GetDimension(); ++dim)
    {
      byteSize *= image->GetDimension(dim);
    }

    mitk::ImageWriteAccessor* accessor = new mitk::ImageWriteAccessor(static_cast<mitk::Image*>(this));
    memset( accessor->GetData(), 0, byteSize );
    delete accessor;

    mitk::TimeSlicedGeometry::Pointer geometry = image->GetTimeSlicedGeometry()->Clone();
    this->SetGeometry( geometry );

    if (m_LabelSetContainer.size())
      m_LabelSetContainer.clear();

    m_ActiveLayer = 0;

    this->CreateDefaultLabelSet(m_ActiveLayer);

    AccessTwoImagesFixedDimensionByItk(this, image, InitializeByLabeledImageProcessing,3);
  }
  catch(...)
  {
    mitkThrow() << "Could not intialize by provided labeled image.";
  }
  this->Modified();
}

template < typename ImageType, typename LabeledImageType >
void mitk::LabelSetImage::InitializeByLabeledImageProcessing(ImageType* input, LabeledImageType* labeled)
{
  typedef itk::ImageRegionConstIterator< LabeledImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;
  typedef itk::RelabelComponentImageFilter<LabeledImageType, LabeledImageType> FilterType;

  typename FilterType::Pointer relabelFilter = FilterType::New();
  relabelFilter->SetInput(labeled);
  relabelFilter->Update();

  TargetIteratorType targetIter( input, input->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  SourceIteratorType sourceIter( relabelFilter->GetOutput(), relabelFilter->GetOutput()->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  int numberOfObjects = relabelFilter->GetNumberOfObjects();

  for (int i=0; i<numberOfObjects; ++i)
  {
    std::stringstream name;
    name << "object-" << i+1;
    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName( name.str().c_str() );
    double rgba[4];
    m_LabelSetContainer[m_ActiveLayer]->GetLookupTable()->GetTableValue(i+1, rgba );
    mitk::Color newColor;
    newColor.SetRed(rgba[0]);
    newColor.SetGreen(rgba[1]);
    newColor.SetBlue(rgba[2]);
    label->SetColor( newColor );
    label->SetOpacity( rgba[3] );

    this->AddLabel(*label);
  }

  while ( !sourceIter.IsAtEnd() )
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    targetIter.Set( sourceValue );
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::MaskStampProcessing(ImageType* itkImage, mitk::Image* mask, bool forceOverwrite)
{
  typedef itk::ImageRegionIterator< ImageType > IteratorType;

  typename ImageType::Pointer itkMask;
  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  SourceIteratorType sourceIter( itkMask, itkMask->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( itkImage, itkImage->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  unsigned int activeLabel = this->GetActiveLabelIndex();

  while ( !sourceIter.IsAtEnd() )
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    int targetValue =  static_cast<int>(targetIter.Get());

    if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabelLocked(targetValue)) ) // skip exterior and locked labels
    {
      targetIter.Set( activeLabel );
    }
    ++sourceIter;
    ++targetIter;
  }

  this->Modified();
}

template < typename ImageType >
void mitk::LabelSetImage::CreateLabelMaskProcessing(ImageType* itkImage, mitk::Image* mask, int index)
{
  typedef itk::ImageRegionIterator< ImageType > IteratorType;

  typename ImageType::Pointer itkMask;
  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  SourceIteratorType sourceIter( itkImage, itkImage->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( itkMask, itkMask->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while ( !sourceIter.IsAtEnd() )
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    if ( sourceValue == index )
    {
      targetIter.Set( 1 );
    }
    ++sourceIter;
    ++targetIter;
  }
}


template < typename ImageType >
void mitk::LabelSetImage::CalculateCenterOfMassProcessing(ImageType* itkImage, int index)
{
  // for now, we just retrieve the voxel in the middle
  typename typedef itk::ImageRegionConstIterator< ImageType > IteratorType;
  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  typename std::vector< ImageType::IndexType > indexVector;

  while ( !iter.IsAtEnd() )
  {
    if ( iter.Get() == static_cast<int>(index) )
    {
      indexVector.push_back(iter.GetIndex());
    }
    ++iter;
  }

  mitk::Point3D pos;
  pos.Fill(0.0);

  if (!indexVector.empty())
  {
   typename itk::ImageRegionConstIteratorWithIndex< ImageType >::IndexType centerIndex;
   centerIndex = indexVector.at(indexVector.size()/2);
   pos[0] = centerIndex[0];
   pos[1] = centerIndex[1];
   pos[2] = centerIndex[2];
  }

  m_LabelSetContainer[m_ActiveLayer]->SetLabelCenterOfMassIndex(index, pos);
  this->GetSlicedGeometry()->IndexToWorld(pos, pos);
  m_LabelSetContainer[m_ActiveLayer]->SetLabelCenterOfMassCoordinates(index, pos);
}

template < typename ImageType >
void mitk::LabelSetImage::ClearBufferProcessing(ImageType* itkImage)
{
  itkImage->FillBuffer(0);
}

template < typename ImageType >
void mitk::LabelSetImage::ConcatenateProcessing(ImageType* itkTarget, mitk::LabelSetImage* other)
{
  typename ImageType::Pointer itkSource = ImageType::New();
  mitk::CastToItkImage( other, itkSource );

  typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      IteratorType;

  ConstIteratorType sourceIter( itkSource, itkSource->GetLargestPossibleRegion() );
  IteratorType      targetIter( itkTarget, itkTarget->GetLargestPossibleRegion() );

  int numberOfTargetLabels = this->GetNumberOfLabels() - 1; // skip exterior
  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    int targetValue =  static_cast<int>(targetIter.Get());
    if ( (sourceValue != 0) && !this->GetLabelLocked(targetValue) ) // skip exterior and locked labels
    {
        targetIter.Set( sourceValue + numberOfTargetLabels );
    }
    ++sourceIter;
    ++targetIter;
  }
}

template < typename PixelType, unsigned int VImageDimension >
void mitk::LabelSetImage::VectorToImageProcessing( itk::Image< PixelType, VImageDimension>* input, int layer)
{
  typedef itk::Image< PixelType, VImageDimension> ImageType;
  ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
  adaptor->SetExtractComponentIndex(layer);
  adaptor->SetImage( m_VectorImage );
  adaptor->Update();

  typedef itk::ImageRegionConstIterator< ImageAdaptorType >  SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >              TargetIteratorType;

  SourceIteratorType sourceIter( adaptor, adaptor->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( input, input->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while(!sourceIter.IsAtEnd())
  {
   targetIter.Set( sourceIter.Get() );
   ++sourceIter;
   ++targetIter;
  }
}

template < typename PixelType, unsigned int VImageDimension >
void mitk::LabelSetImage::ImageToVectorProcessing( itk::Image< PixelType, VImageDimension>* input, int layer)
{
  typedef itk::Image< PixelType, VImageDimension> ImageType;

  ImageAdaptorType::Pointer adaptor = ImageAdaptorType::New();
  adaptor->SetExtractComponentIndex(layer);
  adaptor->SetImage( m_VectorImage );
  adaptor->Update();

  typedef itk::ImageRegionConstIterator< ImageType >   SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageAdaptorType > TargetIteratorType;

  SourceIteratorType sourceIter( input, input->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( adaptor, adaptor->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while(!sourceIter.IsAtEnd())
  {
   targetIter.Set( sourceIter.Get() );
   ++sourceIter;
   ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::SmoothLabelProcessing(ImageType* input, int index)
{
  typedef itk::BinaryThresholdImageFilter< ImageType, ImageType > ThresholdFilterType;
  typedef itk::BinaryMedianImageFilter< ImageType, ImageType > MedianFilterType;

  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput(input);
  thresholdFilter->SetLowerThreshold(index);
  thresholdFilter->SetUpperThreshold(index);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(index);
  thresholdFilter->Update();

  typename ImageType::SizeType radius;
  radius.Fill(1);

  typename MedianFilterType::Pointer medianFilter = MedianFilterType::New();
  medianFilter->SetInput( thresholdFilter->GetOutput() );
  medianFilter->SetRadius( radius );
  medianFilter->SetBackgroundValue(0);
  medianFilter->SetForegroundValue(index);

  medianFilter->Update();

  ImageType::Pointer result = medianFilter->GetOutput();
  result->DisconnectPipeline();

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  SourceIteratorType sourceIter( result, result->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( input, input->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while ( !sourceIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if ( (targetValue == index) || ( sourceValue && (!this->GetLabelLocked(targetValue))) )
    {
      targetIter.Set( sourceValue );
    }

    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::EraseLabelProcessing(ImageType* itkImage, int index, bool reorder)
{
 typedef itk::ImageRegionIterator< ImageType > IteratorType;

 IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
 iter.GoToBegin();

 int nLabels = this->GetNumberOfLabels();

 while (!iter.IsAtEnd())
 {
  int value = static_cast<int>(iter.Get());

  if (value == index)
  {
    iter.Set( 0 );
  }
  else if (reorder)
  {
    for (int i=index+1; i<nLabels; ++i) // for each index up-to the end
    {
      if (value == i)
      {
        iter.Set( i-1 );
      }
    }
  }
  ++iter;
 }
}

template < typename ImageType >
void mitk::LabelSetImage::MergeLabelsProcessing(ImageType* itkImage, int index)
{
 typedef itk::ImageRegionIterator< ImageType > IteratorType;

 const mitk::Label* activeLabel = m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel();

 IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
 iter.GoToBegin();

 while (!iter.IsAtEnd())
 {
   if (static_cast<int>(iter.Get()) == index)
   {
     iter.Set( activeLabel->GetIndex() );
   }
   ++iter;
 }
}
