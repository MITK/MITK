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

mitk::LabelSetImage::LabelSetImage() : mitk::Image(),
m_ActiveLayer(0)
{
  mitk::LabelSet::Pointer ls = this->CreateDefaultLabelSet();
  ls->SetLayer(m_ActiveLayer);
  m_LabelSetContainer.push_back(ls);
}

mitk::LabelSetImage::LabelSetImage(mitk::LabelSetImage* other) : mitk::Image(),
m_ActiveLayer(0)
{
  this->Initialize(other);
  int numberOfLayers = other->GetNumberOfLayers();
  for(int lidx=0; lidx<numberOfLayers; ++lidx)
  {
    m_LabelSetContainer.push_back( other->GetLabelSet(lidx) );
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

  mitk::TimeGeometry::Pointer originalGeometry = other->GetTimeGeometry()->Clone();
  this->SetTimeGeometry( originalGeometry );

  try
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    //newImage->Initialize(other);

    LabelSetImageType::Pointer itkImage;
    mitk::CastToItkImage(other, itkImage);

    LabelSetImageType::Pointer newItkImage = LabelSetImageType::New();
    newItkImage->SetSpacing( itkImage->GetSpacing() );
    newItkImage->SetOrigin( itkImage->GetOrigin() );
    newItkImage->SetDirection( itkImage->GetDirection() );
    newItkImage->SetRegions( itkImage->GetLargestPossibleRegion());
    newItkImage->Allocate();
    newItkImage->FillBuffer(0);

    newImage->InitializeByItk<LabelSetImageType>(newItkImage);
    newImage->SetImportVolume(newItkImage->GetBufferPointer(),0,0,Image::CopyMemory);

    m_LayerContainer.push_back(newImage);

    AccessByItk_1(this, LayerContainerToImageProcessing, m_ActiveLayer);
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

mitk::Image* mitk::LabelSetImage::GetLayerImage(int layer)
{
  return m_LayerContainer[layer];
}

int mitk::LabelSetImage::GetActiveLayer() const
{
  return m_ActiveLayer;
}

int mitk::LabelSetImage::GetNumberOfLayers()
{
  return m_LabelSetContainer.size();
}

mitk::LabelSetImage::VectorImageType::Pointer mitk::LabelSetImage::GetVectorImage(bool forceUpdate)
{
  try
  {
    if (forceUpdate)
    {
      AccessByItk_1(this, ImageToLayerContainerProcessing, m_ActiveLayer);
    }

    LabelSetImageType::Pointer itkImage;
    mitk::CastToItkImage(this,itkImage);

    VectorImageType::Pointer newVectorImage = VectorImageType::New();
    newVectorImage->SetSpacing( itkImage->GetSpacing() );
    newVectorImage->SetOrigin( itkImage->GetOrigin() );
    newVectorImage->SetDirection( itkImage->GetDirection() );
    newVectorImage->SetRegions( itkImage->GetLargestPossibleRegion());
    newVectorImage->SetNumberOfComponentsPerPixel( m_LayerContainer.size() );
    newVectorImage->Allocate();

    itkImage = NULL;

    // fill inside
    VariableVectorType defaultValue;
    defaultValue.SetSize( m_LayerContainer.size() - 1);
    defaultValue.Fill(0);
    newVectorImage->FillBuffer(defaultValue);

    ImageAdaptorType::Pointer targetAdaptor = ImageAdaptorType::New();
    targetAdaptor->SetImage( newVectorImage );

    // transfer layers to a vector image.
    for (int lidx = 0; lidx < m_LayerContainer.size(); ++lidx)
    {
      LabelSetImageType::Pointer itkImageLayer;
      mitk::CastToItkImage(m_LayerContainer[lidx],itkImageLayer);

      targetAdaptor->SetExtractComponentIndex(lidx);
      targetAdaptor->Update();

      typedef itk::ImageRegionConstIterator< LabelSetImageType > SourceIteratorType;
      typedef itk::ImageRegionIterator< ImageAdaptorType >       TargetIteratorType;

      SourceIteratorType sourceIter( itkImageLayer, itkImageLayer->GetLargestPossibleRegion() );
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

    return newVectorImage;
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
    return NULL;
  }
}

void mitk::LabelSetImage::SetVectorImage( VectorImageType::Pointer vectorImage )
{
  try
  {
    int numberOfLayers = vectorImage->GetNumberOfComponentsPerPixel();
    ImageAdaptorType::Pointer vectorAdaptor = ImageAdaptorType::New();
    vectorAdaptor->SetImage( vectorImage );

    for (int lidx = 0; lidx < numberOfLayers; ++lidx)
    {
      LabelSetImageType::Pointer auxImg = LabelSetImageType::New();
      auxImg->SetSpacing( vectorImage->GetSpacing() );
      auxImg->SetOrigin( vectorImage->GetOrigin() );
      auxImg->SetDirection( vectorImage->GetDirection() );
      auxImg->SetRegions( vectorImage->GetLargestPossibleRegion() );
      auxImg->Allocate();

      vectorAdaptor->SetExtractComponentIndex(lidx);

      typedef itk::ImageRegionConstIterator< ImageAdaptorType > SourceIteratorType;
      typedef itk::ImageRegionIterator< LabelSetImageType >     TargetIteratorType;

      SourceIteratorType sourceIter( vectorAdaptor, vectorAdaptor->GetLargestPossibleRegion() );
      sourceIter.GoToBegin();

      TargetIteratorType targetIter( auxImg, auxImg->GetLargestPossibleRegion() );
      targetIter.GoToBegin();

      while(!sourceIter.IsAtEnd())
      {
        targetIter.Set( sourceIter.Get() );
        ++sourceIter;
        ++targetIter;
      }

      mitk::Image::Pointer newLayerImage = mitk::Image::New();
      newLayerImage->Initialize(this);
      newLayerImage->SetVolume(auxImg->GetBufferPointer());

      m_LayerContainer.push_back(newLayerImage);
    }

    // reset active layer
    m_ActiveLayer = 0;
    AccessByItk_1(this, LayerContainerToImageProcessing, m_ActiveLayer);
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
    m_LayerContainer.erase( m_LayerContainer.begin() + m_ActiveLayer);

    // set the active layer to one belo, if exists.
    --m_ActiveLayer;

    // make sure a valid one is set
    if (m_ActiveLayer<0)
      m_ActiveLayer = 0;

    AccessByItk_1(this, LayerContainerToImageProcessing, m_ActiveLayer);
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
    AccessByItk_1(this, ImageToLayerContainerProcessing, m_ActiveLayer);

    mitk::Image::Pointer newImage = mitk::Image::New();
    newImage->Initialize(this);

    LabelSetImageType::Pointer itkImage;
    mitk::CastToItkImage(this, itkImage);
    itkImage->FillBuffer(0);

    m_LayerContainer.push_back(newImage);

    // set the active layer to the maximum available plus one
    m_ActiveLayer = m_LabelSetContainer.size();

    // push a new labelset for the new layer
    mitk::LabelSet::Pointer ls = this->CreateDefaultLabelSet();
    ls->SetLayer(m_ActiveLayer);
    m_LabelSetContainer.push_back(ls);

    itkImage = NULL;
    AccessByItk_1(this, LayerContainerToImageProcessing, m_ActiveLayer);
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }

  this->Modified();
}

void mitk::LabelSetImage::SetActiveLayer(int layer)
{
  try
  {
    if ( (layer != m_ActiveLayer) && (layer>=0) && (layer<this->GetNumberOfLayers()) )
    {

      AccessByItk_1(this, ImageToLayerContainerProcessing, m_ActiveLayer);
      m_ActiveLayer = layer;
      AccessByItk_1(this, LayerContainerToImageProcessing, m_ActiveLayer);
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

mitk::LabelSet::Pointer mitk::LabelSetImage::CreateDefaultLabelSet()
{
  mitk::LabelSet::Pointer ls = mitk::LabelSet::New();
  Color color;
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

  return ls;
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

void mitk::LabelSetImage::CalculateLabelVolume(int index, int layer)
{
// todo
}

void mitk::LabelSetImage::EraseLabel(int index, bool reorder, int layer)
{
  try
  {
    AccessByItk_2(this, EraseLabelProcessing, index, layer);
    if (reorder)
      AccessByItk_2(this, ReorderLabelProcessing, index, layer);
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
    mitk::LabelSet::Pointer ls = this->CreateDefaultLabelSet();
    ls->SetLayer(m_ActiveLayer);
    m_LabelSetContainer.push_back(ls);
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

const mitk::Color& mitk::LabelSetImage::GetLabelColor(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelColor(index);
}

void mitk::LabelSetImage::SetLabelColor(int index, const mitk::Color& color, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

float mitk::LabelSetImage::GetLabelOpacity(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelOpacity(index);
}

float mitk::LabelSetImage::GetLabelVolume(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelVolume(index);
}

void mitk::LabelSetImage::SetLabelOpacity(int index, float value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelOpacity(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelVolume(int index, float value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelVolume(index,value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::RenameLabel(int index, const std::string& name, const mitk::Color& color, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelName(index, name);
  m_LabelSetContainer[layer]->SetLabelColor(index, color);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelName(int index, const std::string& name, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelName(index, name);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetAllLabelsLocked(bool value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetAllLabelsLocked(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::SetAllLabelsVisible(bool value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetAllLabelsVisible(value);
  this->Modified();
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::RemoveLabel(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  this->EraseLabel(index, true, layer);
  m_LabelSetContainer[layer]->RemoveLabel(index);
  this->ResetLabels(layer);
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::ResetLabels(int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->ResetLabels();
}

void mitk::LabelSetImage::MergeLabels(std::vector<int>& indexes, int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  this->SetActiveLabel(index, layer);
  // const mitk::Label* activeLabel = m_LabelSetContainer[m_ActiveLayer]->GetActiveLabel();
  int pixelValue = this->GetActiveLabelIndex();

  for (int idx=0; idx<indexes.size(); idx++)
  {
    AccessByItk_2(this, MergeLabelProcessing, pixelValue, indexes[idx]);
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::MergeLabel(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  int pixelValue = this->GetActiveLabelIndex();
  AccessByItk_2(this, MergeLabelProcessing, pixelValue, index);
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::RemoveLabels(std::vector<int>& indexes, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  std::sort(indexes.begin(), indexes.end());
  for (int idx=0; idx<indexes.size(); idx++)
  {
    this->EraseLabel(indexes[idx], false, layer);
  }
  for (int idx=0; idx<indexes.size(); idx++)
  {
//    this->EraseLabel(indexes[i]-i, true);
    m_LabelSetContainer[layer]->RemoveLabel(indexes[idx]-idx);
    this->ResetLabels(layer);
  }
  RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::EraseLabels(std::vector<int>& indexes, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  std::sort(indexes.begin(), indexes.end());
  for (int i=0; i<indexes.size(); i++)
  {
    this->EraseLabel(indexes[i], false, layer);
  }
}

std::string mitk::LabelSetImage::GetLabelName(int index, int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelName(index);
}

bool mitk::LabelSetImage::GetLabelLocked(int index, int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelLocked(index);
}

bool mitk::LabelSetImage::GetLabelSelected(int index, int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelSelected(index);
}

const mitk::Label* mitk::LabelSetImage::GetActiveLabel(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetActiveLabel();
}

int mitk::LabelSetImage::GetActiveLabelIndex(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetIndex();
}

const char* mitk::LabelSetImage::GetActiveLabelName(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetName();
}

const mitk::Color& mitk::LabelSetImage::GetActiveLabelColor(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetColor();
}

double mitk::LabelSetImage::GetActiveLabelOpacity(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetActiveLabel()->GetOpacity();
}

mitk::LabelSet::ConstPointer mitk::LabelSetImage::GetLabelSet(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer].GetPointer();
}

mitk::LabelSet* mitk::LabelSetImage::GetLabelSet(int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
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

bool mitk::LabelSetImage::GetLabelVisible(int index, int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetLabelVisible(index);
}

void mitk::LabelSetImage::SetLabelVisible(int index, bool value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelVisible(index, value);
  this->Modified();
  ModifyLabelEvent.Send(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassIndex(int index, bool forceUpdate, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;

  if (forceUpdate)
  {
    AccessByItk_2( this, CalculateCenterOfMassProcessing, index, layer );
  }
  return m_LabelSetContainer[layer]->GetLabelCenterOfMassIndex(index);
}

const mitk::Point3D& mitk::LabelSetImage::GetLabelCenterOfMassCoordinates(int index, bool forceUpdate, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;

  if (forceUpdate)
  {
    AccessByItk_2( this, CalculateCenterOfMassProcessing, index, layer );
  }

  return m_LabelSetContainer[layer]->GetLabelCenterOfMassCoordinates(index);
}

void mitk::LabelSetImage::SetActiveLabel(int index, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetActiveLabel(index);

  ActiveLabelEvent.Send(index);

  this->Modified();
}

void mitk::LabelSetImage::SetLabelLocked(int index, bool value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelLocked(index, value);

  ModifyLabelEvent.Send(index);

  this->Modified();

}

void mitk::LabelSetImage::SetLabelSelected(int index, bool value, int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->SetLabelSelected(index, value);

  ModifyLabelEvent.Send(index);

  this->Modified();
}

void mitk::LabelSetImage::RemoveAllLabels(int layer)
{
  if (layer < 0) layer = m_ActiveLayer;
  m_LabelSetContainer[layer]->RemoveAllLabels();
  this->ClearBuffer();
  this->Modified();
  RemoveLabelEvent.Send();
}

int mitk::LabelSetImage::GetNumberOfLabels(int layer) const
{
  if (layer < 0) layer = m_ActiveLayer;
  return m_LabelSetContainer[layer]->GetNumberOfLabels();
}

int mitk::LabelSetImage::GetTotalNumberOfLabels() const
{
  int totalLabels(0);
  std::vector< LabelSet::Pointer >::const_iterator layerIter = m_LabelSetContainer.begin();
  for (; layerIter != m_LabelSetContainer.end(); ++layerIter)
    totalLabels += (*layerIter)->GetNumberOfLabels();
  return totalLabels;
}

void mitk::LabelSetImage::MaskStamp(mitk::Image* mask, bool forceOverwrite)
{
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

    mitk::SlicedGeometry3D::Pointer geometry = this->GetSlicedGeometry()->Clone();
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
    int numberOfPoints = polydata->GetNumberOfPoints();
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

    for (CellIdentifierType i=0; i<numberOfPolygons; i++)
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

      if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabelLocked(targetValue)) ) // skip exterior and locked labels
      {
        targetIter.Set( activeLabel );
      }
      ++sourceIter;
      ++targetIter;
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
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

    mitk::SlicedGeometry3D::Pointer geometry = image->GetSlicedGeometry()->Clone();
    this->SetGeometry( geometry );

    if (m_LabelSetContainer.size())
      m_LabelSetContainer.clear();

    m_ActiveLayer = 0;

    mitk::LabelSet::Pointer ls = this->CreateDefaultLabelSet();
    ls->SetLayer(m_ActiveLayer);
    m_LabelSetContainer.push_back(ls);

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

  int activeLabel = this->GetActiveLabelIndex(m_ActiveLayer);

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
void mitk::LabelSetImage::CalculateCenterOfMassProcessing(ImageType* itkImage, int index, int layer)
{
  // for now, we just retrieve the voxel in the middle
  typedef itk::ImageRegionConstIterator< ImageType > IteratorType;
  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  std::vector< typename ImageType::IndexType > indexVector;

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

  m_LabelSetContainer[layer]->SetLabelCenterOfMassIndex(index, pos);
  this->GetSlicedGeometry()->IndexToWorld(pos, pos);
  m_LabelSetContainer[layer]->SetLabelCenterOfMassCoordinates(index, pos);
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
    if ( (sourceValue != 0) && !this->GetLabelLocked(targetValue) ) // skip exterior and locked labels
    {
      targetIter.Set( sourceValue + numberOfTargetLabels );
    }
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::LayerContainerToImageProcessing( ImageType* input, int layer)
{
  typename ImageType::Pointer itkSource;
  mitk::CastToItkImage(m_LayerContainer[m_ActiveLayer], itkSource);
  typedef itk::ImageRegionConstIterator< ImageType >  SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >       TargetIteratorType;

  SourceIteratorType sourceIter( itkSource, itkSource->GetLargestPossibleRegion() );
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
void mitk::LabelSetImage::ImageToLayerContainerProcessing( ImageType* input, int layer)
{
  typename ImageType::Pointer itkTarget;
  mitk::CastToItkImage(m_LayerContainer[m_ActiveLayer],itkTarget);

  typedef itk::ImageRegionConstIterator< ImageType >   SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >        TargetIteratorType;

  SourceIteratorType sourceIter( input, input->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( itkTarget, itkTarget->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while(!sourceIter.IsAtEnd())
  {
   targetIter.Set( sourceIter.Get() );
   ++sourceIter;
   ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::EraseLabelProcessing(ImageType* itkImage, int index, int layer)
{
 typedef itk::ImageRegionIterator< ImageType > IteratorType;

 IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
 iter.GoToBegin();

 int numberOfLabels = this->GetNumberOfLabels(layer);

 while (!iter.IsAtEnd())
 {
  int value = static_cast<int>(iter.Get());

  if (value == index)
  {
    iter.Set( 0 );
  }
  ++iter;
 }
}

template < typename ImageType >
void mitk::LabelSetImage::ReorderLabelProcessing(ImageType* itkImage, int index, int layer)
{
 typedef itk::ImageRegionIterator< ImageType > IteratorType;

 IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
 iter.GoToBegin();

 int numberOfLabels = this->GetNumberOfLabels(layer);

 while (!iter.IsAtEnd())
 {
  int value = static_cast<int>(iter.Get());

  if (value != index)
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
void mitk::LabelSetImage::MergeLabelProcessing(ImageType* itkImage, int pixelValue, int index)
{
 typedef itk::ImageRegionIterator< ImageType > IteratorType;

 IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
 iter.GoToBegin();

 while (!iter.IsAtEnd())
 {
   if (static_cast<int>(iter.Get()) == index)
   {
     iter.Set( pixelValue );
   }
   ++iter;
 }
}
