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
  int numberOfLayers = other->GetNumberOfLayers();
  for (int layer=0; layer<numberOfLayers; ++layer)
  {
    m_LabelSetContainer.push_back( other->GetLabelSet(layer) );
  }
}

void mitk::LabelSetImage::Initialize(const mitk::Image* other)
{
  mitk::PixelType pixelType(mitk::MakeScalarPixelType<LabelSetImage::PixelType>() );
  if (other->GetDimension() == 2)
  {
    const unsigned int dimensions[] = { other->GetDimension(0), other->GetDimension(1), 1 };
    Superclass::Initialize(pixelType, 3, dimensions);
  }
  else
  {
    Superclass::Initialize(pixelType, other->GetDimension(), other->GetDimensions());
  }

  mitk::TimeSlicedGeometry::Pointer originalGeometry = other->GetTimeSlicedGeometry()->Clone();
  this->SetGeometry( originalGeometry );

  LabelSetImageType::Pointer itkImage;
  mitk::CastToItkImage(other, itkImage);

  try
  {
    m_VectorImage = VectorImageType::New();
    m_VectorImage->SetSpacing( itkImage->GetSpacing() );
    m_VectorImage->SetOrigin( itkImage->GetOrigin() );
    m_VectorImage->SetDirection( itkImage->GetDirection() );
    m_VectorImage->SetRegions( itkImage->GetLargestPossibleRegion());
    m_VectorImage->SetLargestPossibleRegion( itkImage->GetLargestPossibleRegion());
    m_VectorImage->SetBufferedRegion( itkImage->GetLargestPossibleRegion());
    m_VectorImage->SetNumberOfComponentsPerPixel(1);
    m_VectorImage->Allocate();

    typedef itk::VariableLengthVector<LabelSetImage::PixelType> VariableVectorType;
    VariableVectorType defaultValue;
    defaultValue.SetSize(1);
    defaultValue.Fill(0);
    m_VectorImage->FillBuffer(defaultValue);

    m_ActiveLayer = 0;

    m_ImageToVectorAdaptor = ImageAdaptorType::New();
    m_ImageToVectorAdaptor->SetImage( m_VectorImage );
    m_ImageToVectorAdaptor->SetExtractComponentIndex( m_ActiveLayer );
    m_ImageToVectorAdaptor->Update();

    this->SetImportVolume(m_ImageToVectorAdaptor->GetBufferPointer(),0,0,Image::CopyMemory);
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
}

mitk::LabelSetImage::~LabelSetImage()
{
  m_LabelSetContainer.clear();
}

mitk::LabelSetImage::PixelType* mitk::LabelSetImage::GetLayerBufferPointer(unsigned int layer)
{
  m_ImageToVectorAdaptor->SetExtractComponentIndex( layer );
  m_ImageToVectorAdaptor->Update();
  return m_ImageToVectorAdaptor->GetBufferPointer();
//  this->SetImportVolume(m_ImageToVectorAdaptor->GetBufferPointer(),0,0,Image::CopyMemory);
}

unsigned int mitk::LabelSetImage::GetActiveLayer() const
{
  return m_ActiveLayer;
}

unsigned int mitk::LabelSetImage::GetNumberOfLayers()
{
  return m_LabelSetContainer.size();
}

mitk::LabelSetImage::VectorImageType::Pointer mitk::LabelSetImage::GetVectorImage(bool forceUpdate)
{
  try
  {
    if (forceUpdate)
    {
      AccessByItk_1(this, ImageToVectorProcessing, m_ActiveLayer);
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  return m_VectorImage;
}

void mitk::LabelSetImage::SetVectorImage(VectorImageType::Pointer image )
{
  try
  {
    m_VectorImage = image;
    m_ActiveLayer = 0;
    AccessByItk_1(this, VectorToImageProcessing, m_ActiveLayer);
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::LabelSetImage::RemoveLayer()
{
  try
  {
    m_LabelSetContainer.erase( m_LabelSetContainer.begin() + m_ActiveLayer);
    this->SetActiveLayer( m_LabelSetContainer.size() - 1 );
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::LabelSetImage::AddLayer()
{
  try
  {
    // transfer the active layer image to the vector image
    AccessByItk_1(this, ImageToVectorProcessing, m_ActiveLayer);

    // set the active layer to the maximum available plus one
    m_ActiveLayer = m_LabelSetContainer.size();

    // push a new label set for the new layer
    this->CreateDefaultLabelSet(m_ActiveLayer);

    // create a new vector image
    VectorImageType::Pointer newVectorImage = VectorImageType::New();
    newVectorImage->SetSpacing( m_VectorImage->GetSpacing() );
    newVectorImage->SetOrigin( m_VectorImage->GetOrigin() );
    newVectorImage->SetDirection( m_VectorImage->GetDirection() );
    newVectorImage->SetLargestPossibleRegion( m_VectorImage->GetLargestPossibleRegion());
    newVectorImage->SetBufferedRegion( m_VectorImage->GetLargestPossibleRegion());
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
    mitkThrow() << e.GetDescription();
  }

  this->Modified();
}

void mitk::LabelSetImage::SetActiveLayer(unsigned int layer)
{
  try
  {
    if ( layer != m_ActiveLayer )
    {
      AccessByItk_1(this, ImageToVectorProcessing, m_ActiveLayer);
      m_ActiveLayer = layer;
      AccessByItk_1(this, VectorToImageProcessing, m_ActiveLayer);
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
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

void mitk::LabelSetImage::SetLastModificationTime(const std::string& name)
{
  m_LastModificationTime = name;
}

const std::string& mitk::LabelSetImage::GetLastModificationTime()
{
  return m_LastModificationTime;
}

void mitk::LabelSetImage::SetName(const std::string& name)
{
  m_Name = name;
}

const std::string& mitk::LabelSetImage::GetName()
{
  return m_Name;
}

void mitk::LabelSetImage::CalculateLabelVolume(int layer, int index)
{
// todo
}

void mitk::LabelSetImage::SmoothLabel(int layer, int index)
{
  try
  {
    AccessByItk_2(this, SmoothLabelProcessing, layer, index);
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
}

void mitk::LabelSetImage::EraseLabel(int layer, int index, bool reorder)
{
  try
  {
    AccessByItk_2(this, EraseLabelProcessing, index, reorder);
    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
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
    int numberOfLayers = other->GetNumberOfLayers();
    for (int layer=0; layer<numberOfLayers; ++layer)
    {
      this->SetActiveLayer(layer);
      AccessByItk_1(this, ConcatenateProcessing, other);
      mitk::LabelSet::ConstPointer ls = other->GetLabelSet(layer);
      for( int label=1; label<ls->GetNumberOfLabels(); label++ ) // skip exterior
      {
        this->AddLabel(*ls->GetLabel(label) );
      }
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
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
    mitkThrow() << e.GetDescription();
  }
}

void mitk::LabelSetImage::AddLabel(const mitk::Label& label)
{
  if (label.GetLayer() > m_LabelSetContainer.size() - 1)
  {
    m_ActiveLayer = label.GetLayer();
    this->CreateDefaultLabelSet(m_ActiveLayer);
  }
  m_LabelSetContainer[label.GetLayer()]->AddLabel(label);
  this->Modified();
  AddLabelEvent.Send();
}

void mitk::LabelSetImage::AddLabel(const std::string& name, const mitk::Color& color)
{
  m_LabelSetContainer[m_ActiveLayer]->AddLabel(name, color);
  this->Modified();
  AddLabelEvent.Send();
}

const mitk::Color& mitk::LabelSetImage::GetLabelColor(int layer, int index)
{
  return m_LabelSetContainer[layer]->GetLabelColor(index);
}

void mitk::LabelSetImage::SetLabelColor(int layer, int index, const mitk::Color& color)
{
  m_LabelSetContainer[layer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

float mitk::LabelSetImage::GetLabelOpacity(int layer, int index)
{
  return m_LabelSetContainer[layer]->GetLabelOpacity(index);
}

float mitk::LabelSetImage::GetLabelVolume(int layer, int index)
{
  return m_LabelSetContainer[layer]->GetLabelVolume(index);
}

void mitk::LabelSetImage::SetLabelOpacity(int layer, int index, float value)
{
  m_LabelSetContainer[layer]->SetLabelOpacity(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelVolume(int layer, int index, float value)
{
  m_LabelSetContainer[layer]->SetLabelVolume(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::RenameLabel(int layer, int index, const std::string& name, const mitk::Color& color)
{
  m_LabelSetContainer[layer]->SetLabelName(index, name);
  m_LabelSetContainer[layer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelName(int layer, int index, const std::string& name)
{
  m_LabelSetContainer[layer]->SetLabelName(index, name);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetAllLabelsLocked(int layer, bool value)
{
  m_LabelSetContainer[layer]->SetAllLabelsLocked(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::SetAllLabelsVisible(int layer, bool value)
{
  m_LabelSetContainer[layer]->SetAllLabelsVisible(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::RemoveLabel(int layer, int index)
{
  this->EraseLabel(layer, index, true);
  m_LabelSetContainer[layer]->RemoveLabel(index);
  this->ResetLabels(layer);
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::ResetLabels(int layer)
{
  m_LabelSetContainer[layer]->ResetLabels();
}

void mitk::LabelSetImage::MergeLabels(int layer, std::vector<int>& indexes, int index)
{
  this->SetActiveLabel( layer, index, false );
  for (int idx=0; idx<indexes.size(); idx++)
  {
    AccessByItk_1(this, MergeLabelsProcessing, indexes[idx]);
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::RemoveLabels(int layer, std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());
  for (int idx=0; idx<indexes.size(); idx++)
  {
    this->EraseLabel(layer, indexes[idx], false);
  }
  for (int idx=0; idx<indexes.size(); idx++)
  {
//    this->EraseLabel(indexes[i]-i, true);
    m_LabelSetContainer[layer]->RemoveLabel(indexes[idx]-idx);
    this->ResetLabels(layer);
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::EraseLabels(int layer, std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());
  for (int i=0; i<indexes.size(); i++)
  {
    this->EraseLabel(layer, indexes[i], false);
  }
}

void mitk::LabelSetImage::SmoothLabels(int layer, std::vector<int>& indexes)
{
  std::sort(indexes.begin(), indexes.end());
  for (int i=0; i<indexes.size(); i++)
  {
    this->SmoothLabel(layer, indexes[i]);
  }
}

std::string mitk::LabelSetImage::GetLabelName(int layer, int index)
{
  return m_LabelSetContainer[layer]->GetLabelName(index);
}

bool mitk::LabelSetImage::GetLabelLocked(int layer, int index) const
{
  return m_LabelSetContainer[layer]->GetLabelLocked(index);
}

bool mitk::LabelSetImage::GetLabelSelected(int layer, int index) const
{
  return m_LabelSetContainer[layer]->GetLabelSelected(index);
}

const mitk::Label* mitk::LabelSetImage::GetActiveLabel(int layer) const
{
  return m_LabelSetContainer[layer]->GetActiveLabel();
}

unsigned int mitk::LabelSetImage::GetActiveLabelIndex(int layer) const
{
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetIndex();
}

const mitk::Color& mitk::LabelSetImage::GetActiveLabelColor(int layer) const
{
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetColor();
}

mitk::LabelSet::ConstPointer mitk::LabelSetImage::GetLabelSet(int layer) const
{
  return m_LabelSetContainer[layer].GetPointer();
}

mitk::LabelSet* mitk::LabelSetImage::GetLabelSet(int layer)
{
  return m_LabelSetContainer[layer].GetPointer();
}

const mitk::LookupTable* mitk::LabelSetImage::GetLookupTable(int layer)
{
  return m_LabelSetContainer[layer]->GetLookupTable();
}

void mitk::LabelSetImage::SetLabelSet(int layer, mitk::LabelSet* labelset)
{
  if (m_LabelSetContainer[layer] != labelset)
  {
    m_LabelSetContainer[layer] = labelset;
    this->ResetLabels(layer);
  }
}

bool mitk::LabelSetImage::GetLabelVisible(int layer, int index) const
{
  return m_LabelSetContainer[layer]->GetLabelVisible(index);
}

void mitk::LabelSetImage::SetLabelVisible(int layer, int index, bool value)
{
  m_LabelSetContainer[layer]->SetLabelVisible(index, value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassIndex(int layer, int index, bool forceUpdate)
{
  if (forceUpdate)
  {
    if (layer != m_ActiveLayer)
      this->SetActiveLayer(layer);
    AccessByItk_1( this, CalculateCenterOfMassProcessing, index );
  }
  return m_LabelSetContainer[layer]->GetLabelCenterOfMassIndex(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassCoordinates(int layer, int index, bool forceUpdate)
{
  if (forceUpdate)
  {
    if (layer != m_ActiveLayer)
      this->SetActiveLayer(layer);
    AccessByItk_1( this, CalculateCenterOfMassProcessing, index );
  }
  return m_LabelSetContainer[layer]->GetLabelCenterOfMassCoordinates(index);
}

void mitk::LabelSetImage::SetActiveLabel(int layer, int index, bool sendEvent)
{
  m_LabelSetContainer[layer]->SetActiveLabel(index);
  if (sendEvent)
    ModifyLabelEvent.Send(index);
  this->Modified();
}

void mitk::LabelSetImage::SetLabelLocked(int layer, int index, bool value)
{
  m_LabelSetContainer[layer]->SetLabelLocked(index, value);
  this->Modified();
}

void mitk::LabelSetImage::SetLabelSelected(int layer, int index, bool value)
{
  m_LabelSetContainer[layer]->SetLabelSelected(index, value);
  this->Modified();
}

void mitk::LabelSetImage::RemoveAllLabels(int layer)
{
  m_LabelSetContainer[layer]->RemoveAllLabels();
  this->ClearBuffer();
  this->Modified();
  RemoveLabelEvent.Send();
}

unsigned int mitk::LabelSetImage::GetNumberOfLabels(int layer) const
{
  return m_LabelSetContainer[layer]->GetNumberOfLabels();
}

unsigned int mitk::LabelSetImage::GetTotalNumberOfLabels() const
{
  unsigned int totalLabels(0);
  std::vector< LabelSet::Pointer >::const_iterator layerIter = m_LabelSetContainer.begin();
  for (; layerIter != m_LabelSetContainer.end(); ++layerIter)
    totalLabels += (*layerIter)->GetNumberOfLabels();
  return totalLabels;
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

    unsigned int byteSize = sizeof(LabelSetImage::PixelType);
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

    int activeLabel = this->GetActiveLabelIndex(m_ActiveLayer);

    while ( !sourceIter.IsAtEnd() )
    {
      int sourceValue = static_cast<int>(sourceIter.Get());
      int targetValue =  static_cast<int>(targetIter.Get());

      if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabelLocked(m_ActiveLayer, targetValue)) ) // skip exterior and locked labels
      {
        targetIter.Set( activeLabel );
      }
      ++sourceIter;
      ++targetIter;
    }

    this->Modified();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
}

void mitk::LabelSetImage::InitializeByLabeledImage(mitk::Image::Pointer image)
{
  if (image.IsNull() || image->IsEmpty() || !image->IsInitialized())
    mitkThrow() << "Invalid labeled image.";

  try
  {
    this->Initialize(image);

    unsigned int byteSize = sizeof(LabelSetImage::PixelType);
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

template < typename ImageType1, typename ImageType2 >
void mitk::LabelSetImage::InitializeByLabeledImageProcessing(ImageType1* input, ImageType2* labeled)
{
  typedef itk::ImageRegionConstIterator< ImageType2 > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType1 > TargetIteratorType;
  typedef itk::RelabelComponentImageFilter<ImageType2, ImageType2> FilterType;

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

  unsigned int activeLabel = this->GetActiveLabelIndex(m_ActiveLayer);

  while ( !sourceIter.IsAtEnd() )
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    int targetValue =  static_cast<int>(targetIter.Get());

    if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabelLocked(m_ActiveLayer, targetValue)) ) // skip exterior and locked labels
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

// todo: concatenate all layers and not just the active one
template < typename ImageType >
void mitk::LabelSetImage::ConcatenateProcessing(ImageType* itkTarget, mitk::LabelSetImage* other)
{
  typename ImageType::Pointer itkSource = ImageType::New();
  mitk::CastToItkImage( other, itkSource );

  typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      IteratorType;

  ConstIteratorType sourceIter( itkSource, itkSource->GetLargestPossibleRegion() );
  IteratorType      targetIter( itkTarget, itkTarget->GetLargestPossibleRegion() );

  int numberOfTargetLabels = this->GetNumberOfLabels(m_ActiveLayer) - 1; // skip exterior
  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    int sourceValue = static_cast<int>(sourceIter.Get());
    int targetValue =  static_cast<int>(targetIter.Get());
    if ( (sourceValue != 0) && !this->GetLabelLocked(m_ActiveLayer, targetValue) ) // skip exterior and locked labels
    {
      targetIter.Set( sourceValue + numberOfTargetLabels );
    }
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::VectorToImageProcessing( ImageType* input, int layer)
{
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

template < typename ImageType >
void mitk::LabelSetImage::ImageToVectorProcessing( ImageType* input, int layer)
{
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
void mitk::LabelSetImage::SmoothLabelProcessing(ImageType* input, int layer, int index)
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

    if ( (targetValue == index) || ( sourceValue && (!this->GetLabelLocked(layer, targetValue))) )
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

 int numberOfLabels = this->GetNumberOfLabels(m_ActiveLayer);

 while (!iter.IsAtEnd())
 {
  int value = static_cast<int>(iter.Get());

  if (value == index)
  {
    iter.Set( 0 );
  }
  else if (reorder)
  {
    for (int i=index+1; i<numberOfLabels; ++i) // for each index up-to the end
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
