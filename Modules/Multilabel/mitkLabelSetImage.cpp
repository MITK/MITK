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
//#include <itkRelabelComponentImageFilter.h>

#include <itkCommand.h>

mitk::LabelSetImage::LabelSetImage() :
  mitk::Image(),
  m_ActiveLayer(0),
  m_ExteriorLabel(nullptr)
{
  // Iniitlaize Background Label
  mitk::Color color;
  color.Set(0,0,0);
  m_ExteriorLabel = mitk::Label::New();
  m_ExteriorLabel->SetColor(color);
  m_ExteriorLabel->SetName("Exterior");
  m_ExteriorLabel->SetOpacity(0.0);
  m_ExteriorLabel->SetLocked(false);
  m_ExteriorLabel->SetValue(0);
}

mitk::LabelSetImage::LabelSetImage(const mitk::LabelSetImage & other):
  Image(other),
  m_ActiveLayer(other.GetActiveLayer()),
  m_ExteriorLabel(other.GetExteriorLabel()->Clone())
{
  for(unsigned int i = 0 ; i < other.GetNumberOfLayers(); i++)
  {
    // Clone LabelSet data
    mitk::LabelSet::Pointer lsClone = other.GetLabelSet(i)->Clone();
    // add modified event listener to LabelSet (listen to LabelSet changes)
    itk::SimpleMemberCommand<Self>::Pointer command = itk::SimpleMemberCommand<Self>::New();
    command->SetCallbackFunction(this,&mitk::LabelSetImage::OnLabelSetModified);
    lsClone->AddObserver(itk::ModifiedEvent(), command);
    m_LabelSetContainer.push_back( lsClone );

    // clone layer Image data
    mitk::Image::Pointer liClone = other.GetLayerImage(i)->Clone();
    m_LayerContainer.push_back( liClone );
  }
}

void mitk::LabelSetImage::OnLabelSetModified()
{
  Superclass::Modified();
}

void mitk::LabelSetImage::SetExteriorLabel(mitk::Label * label)
{
  m_ExteriorLabel = label;
}

mitk::Label* mitk::LabelSetImage::GetExteriorLabel()
{
  return m_ExteriorLabel;
}

const mitk::Label* mitk::LabelSetImage::GetExteriorLabel() const
{
  return m_ExteriorLabel;
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

  // Add a inital LabelSet ans corresponding image data to the stack
  AddLayer();
}

mitk::LabelSetImage::~LabelSetImage()
{
  m_LabelSetContainer.clear();
}

mitk::Image* mitk::LabelSetImage::GetLayerImage(unsigned int layer)
{
  return m_LayerContainer[layer];
}

const mitk::Image* mitk::LabelSetImage::GetLayerImage(unsigned int layer) const
{
  return m_LayerContainer[layer];
}

unsigned int mitk::LabelSetImage::GetActiveLayer() const
{
  return m_ActiveLayer;
}

unsigned int mitk::LabelSetImage::GetNumberOfLayers() const
{
  return m_LabelSetContainer.size();
}

mitk::LabelSetImage::VectorImageType::Pointer mitk::LabelSetImage::GetVectorImage(bool forceUpdate) const
{
  try
  {
    if (forceUpdate)
    {
      AccessByItk_1(this, ImageToLayerContainerProcessing, GetActiveLayer());
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

    itkImage = nullptr;

    // fill inside
    VariableVectorType defaultValue;
    defaultValue.SetSize( m_LayerContainer.size());
    defaultValue.Fill(0);
    newVectorImage->FillBuffer(defaultValue);

    ImageAdaptorType::Pointer targetAdaptor = ImageAdaptorType::New();
    targetAdaptor->SetImage( newVectorImage );

    // transfer layers to a vector image.
    for (unsigned int lidx = 0; lidx < m_LayerContainer.size(); ++lidx)
    {
      MITK_INFO << lidx ;
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
    return nullptr;
  }
}

void mitk::LabelSetImage::SetVectorImage(VectorImageType::Pointer vectorImage )
{
  try
  {
    int numberOfLayers = vectorImage->GetNumberOfComponentsPerPixel();

    MITK_INFO << numberOfLayers;

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
      m_LayerContainer[lidx] = newLayerImage;
      AccessByItk_1(this, LayerContainerToImageProcessing, lidx);
    }
    // reset active layer
    SetActiveLayer(0);

  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::LabelSetImage::RemoveLayer()
{

  int layerToDelete = GetActiveLayer();
  // remove all observers from active label set
  GetLabelSet(layerToDelete)->RemoveAllObservers();

  // set the active layer to one belo, if exists.
  SetActiveLayer(layerToDelete-1);

  // remove labelset and image data
  m_LabelSetContainer.erase( m_LabelSetContainer.begin() + layerToDelete);
  m_LayerContainer.erase( m_LayerContainer.begin() + layerToDelete);

  this->Modified();
}

unsigned int mitk::LabelSetImage::AddLayer(mitk::LabelSet::Pointer lset)
{
  mitk::Image::Pointer newImage = mitk::Image::New();
  newImage->Initialize(this);

  LabelSetImageType::Pointer itkImage;
  mitk::CastToItkImage(newImage, itkImage);
  itkImage->FillBuffer(0);

  // get upcoming new labelset id
  unsigned int newLabelSetId = m_LayerContainer.size();

  // Add labelset to layer
  mitk::LabelSet::Pointer ls;
  if (lset.IsNotNull())
  {
    ls = lset;
  }
  else
  {
    ls = mitk::LabelSet::New();
    ls->AddLabel(GetExteriorLabel());
    ls->SetActiveLabel(0 /*Exterior Label*/);
  }

  ls->SetLayer(newLabelSetId);
  // Add exterior Label to label set
  //mitk::Label::Pointer exteriorLabel = CreateExteriorLabel();

  // push a new working image for the new layer
  m_LayerContainer.push_back(newImage);

  // push a new labelset for the new layer
  m_LabelSetContainer.push_back(ls);

  // add modified event listener to LabelSet (listen to LabelSet changes)
  itk::SimpleMemberCommand<Self>::Pointer command = itk::SimpleMemberCommand<Self>::New();
  command->SetCallbackFunction(this,&mitk::LabelSetImage::OnLabelSetModified);
  ls->AddObserver(itk::ModifiedEvent(), command);

  //  important to release the itk image
  itkImage = nullptr;

  SetActiveLayer(newLabelSetId);
  //MITK_INFO << GetActiveLayer();
  this->Modified();
  return newLabelSetId;
}

void mitk::LabelSetImage::SetActiveLayer(unsigned int layer)
{
  try
  {
    if ( (layer != GetActiveLayer()) && (layer<this->GetNumberOfLayers()) )
    {
      BeforeChangeLayerEvent.Send();

      AccessByItk_1(this, ImageToLayerContainerProcessing, GetActiveLayer());
      m_ActiveLayer = layer; // only at this place m_ActiveLayer should be manipulated!!! Use Getter and Setter
      AccessByItk_1(this, LayerContainerToImageProcessing, GetActiveLayer());

      AfterchangeLayerEvent.Send();
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
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
      mitk::LabelSet * ls = other->GetLabelSet(layer);
      auto it = ls->IteratorConstBegin();
      auto end = ls->IteratorConstEnd();
      it++;// skip exterior
      while(it != end)
      {
        GetLabelSet()->AddLabel((it->second));
        //AddLabelEvent.Send();
        it++;
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

bool mitk::LabelSetImage::ExistLabel(PixelType pixelValue) const
{
  bool exist = false;
  for(unsigned int lidx = 0 ; lidx < GetNumberOfLayers(); lidx++)
    exist |= m_LabelSetContainer[lidx]->ExistLabel(pixelValue);
  return exist;
}

bool mitk::LabelSetImage::ExistLabel(PixelType pixelValue, unsigned int layer) const
{
  bool exist = m_LabelSetContainer[layer]->ExistLabel(pixelValue);
  return exist;
}

bool mitk::LabelSetImage::ExistLabelSet(unsigned int layer) const
{
  return layer < m_LabelSetContainer.size();
}


void mitk::LabelSetImage::MergeLabel(PixelType pixelValue, unsigned int /*layer*/)
{
  int targetPixelValue = GetActiveLabel()->GetValue();
  try
  {
    AccessByItk_2(this, MergeLabelProcessing, targetPixelValue, pixelValue);
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  Modified();
}

void mitk::LabelSetImage::MergeLabels(std::vector<PixelType> &VectorOfLablePixelValues, PixelType pixelValue, unsigned int layer)
{
  GetLabelSet(layer)->SetActiveLabel(pixelValue);
  try
  {
    for (unsigned int idx=0; idx<VectorOfLablePixelValues.size(); idx++)
    {
      AccessByItk_2(this, MergeLabelProcessing, pixelValue, VectorOfLablePixelValues[idx]);
    }
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }

}

void mitk::LabelSetImage::RemoveLabels(std::vector<PixelType>& VectorOfLabelPixelValues, unsigned int layer)
{
  for (unsigned int idx=0; idx<VectorOfLabelPixelValues.size(); idx++)
  {
    GetLabelSet(layer)->RemoveLabel(VectorOfLabelPixelValues[idx]);
    EraseLabel(VectorOfLabelPixelValues[idx],layer);
  }
}

void mitk::LabelSetImage::EraseLabels(std::vector<PixelType> &VectorOfLabelPixelValues, unsigned int layer)
{
  for (unsigned int i=0; i<VectorOfLabelPixelValues.size(); i++)
  {
    this->EraseLabel(VectorOfLabelPixelValues[i], layer);
  }
}

void mitk::LabelSetImage::EraseLabel(PixelType pixelValue, unsigned int layer)
{
  try
  {
    AccessByItk_2(this, EraseLabelProcessing, pixelValue, layer);
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  Modified();
}

mitk::Label *mitk::LabelSetImage::GetActiveLabel(unsigned int layer)
{
  if (m_LabelSetContainer.size() > layer)
    return m_LabelSetContainer[layer]->GetActiveLabel();
  else
    return nullptr;
}

mitk::Label *mitk::LabelSetImage::GetLabel(PixelType pixelValue, unsigned int layer) const
{
  if(m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer]->GetLabel(pixelValue);
}

mitk::LabelSet* mitk::LabelSetImage::GetLabelSet(unsigned int layer)
{
  if(m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer].GetPointer();
}

const mitk::LabelSet* mitk::LabelSetImage::GetLabelSet(unsigned int layer) const
{
  if(m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer].GetPointer();
}

mitk::LabelSet* mitk::LabelSetImage::GetActiveLabelSet()
{
  return m_LabelSetContainer[GetActiveLayer()].GetPointer();
}

void mitk::LabelSetImage::UpdateCenterOfMass(PixelType pixelValue, unsigned int layer)
{
  AccessByItk_2( this, CalculateCenterOfMassProcessing, pixelValue, layer );
}

unsigned int mitk::LabelSetImage::GetNumberOfLabels(unsigned int layer) const
{
  return m_LabelSetContainer[layer]->GetNumberOfLabels();
}

unsigned int mitk::LabelSetImage::GetTotalNumberOfLabels() const
{
  unsigned int totalLabels(0);
  auto layerIter = m_LabelSetContainer.begin();
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

mitk::Image::Pointer mitk::LabelSetImage::CreateLabelMask(PixelType index)
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
    transformer->SetInputData(polydata);
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

    PointIdentifierType numberOfCellPoints = 3;

    for (CellIdentifierType i=0; i<numberOfPolygons; i++)
    {
      vtkIdList *cellIds;
      vtkCell *vcell = polydata->GetCell(i);
      cellIds = vcell->GetPointIds();

      CellAutoPointerType cell;
      auto   triangleCell = new TriangleCellType;
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

    int activeLabel = GetActiveLabel(GetActiveLayer())->GetValue();

    while ( !sourceIter.IsAtEnd() )
    {
      int sourceValue = static_cast<int>(sourceIter.Get());
      int targetValue =  static_cast<int>(targetIter.Get());

      if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabel(targetValue)->GetLocked()) ) // skip exterior and locked labels
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

    AccessTwoImagesFixedDimensionByItk(this, image, InitializeByLabeledImageProcessing,3);
  }
  catch(...)
  {
    mitkThrow() << "Could not intialize by provided labeled image.";
  }
  this->Modified();
}

template < typename ImageType1, typename ImageType2 >
void mitk::LabelSetImage::InitializeByLabeledImageProcessing(ImageType1* output, ImageType2* input)
{
  typedef itk::ImageRegionConstIterator< ImageType2 > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType1 > TargetIteratorType;
//  typedef itk::RelabelComponentImageFilter<ImageType2, ImageType2> FilterType;

  TargetIteratorType targetIter( output, output->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  //SourceIteratorType sourceIter( relabelFilter->GetOutput(), relabelFilter->GetOutput()->GetLargestPossibleRegion() );
  SourceIteratorType sourceIter( input, input->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  while ( !sourceIter.IsAtEnd() )
  {
    PixelType sourceValue = static_cast<PixelType>(sourceIter.Get());
    targetIter.Set( sourceValue );

    if(!ExistLabel(sourceValue))
    {
      std::stringstream name;
      name << "object-" << sourceValue;
      mitk::Label::Pointer label = mitk::Label::New();
      label->SetName( name.str().c_str() );
      double rgba[4];
      m_LabelSetContainer[GetActiveLayer()]->GetLookupTable()->GetTableValue(sourceValue, rgba );
      mitk::Color newColor;
      newColor.SetRed(rgba[0]);
      newColor.SetGreen(rgba[1]);
      newColor.SetBlue(rgba[2]);
      label->SetColor( newColor );
      label->SetOpacity( rgba[3] );
      label->SetValue( sourceValue );

      GetLabelSet()->AddLabel(label);

      if(GetActiveLabelSet()->GetNumberOfLabels() >= mitk::Label::MAX_LABEL_VALUE || sourceValue >= mitk::Label::MAX_LABEL_VALUE)
      {
        AddLayer();
      }

    }

    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::MaskStampProcessing(ImageType* itkImage, mitk::Image* mask, bool forceOverwrite)
{

  typename ImageType::Pointer itkMask;
  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  SourceIteratorType sourceIter( itkMask, itkMask->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( itkImage, itkImage->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  int activeLabel = this->GetActiveLabel(GetActiveLayer())->GetValue();

  while ( !sourceIter.IsAtEnd() )
  {
    PixelType sourceValue = sourceIter.Get();
    PixelType targetValue = targetIter.Get();

    if ( (sourceValue != 0) && (forceOverwrite || !this->GetLabel(targetValue)->GetLocked()) ) // skip exterior and locked labels
    {
      targetIter.Set( activeLabel );
    }
    ++sourceIter;
    ++targetIter;
  }

  this->Modified();
}

template < typename ImageType >
void mitk::LabelSetImage::CreateLabelMaskProcessing(ImageType* itkImage, mitk::Image* mask, PixelType index)
{

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
    PixelType sourceValue = sourceIter.Get();
    if ( sourceValue == index )
    {
      targetIter.Set( 1 );
    }
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::CalculateCenterOfMassProcessing(ImageType* itkImage, PixelType pixelValue, unsigned int layer)
{
  // for now, we just retrieve the voxel in the middle
  typedef itk::ImageRegionConstIterator< ImageType > IteratorType;
  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  std::vector< typename ImageType::IndexType > indexVector;

  while ( !iter.IsAtEnd() )
  {
    // TODO fix comparison warning more effective
    if ( iter.Get() == pixelValue )
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
    if (centerIndex.GetIndexDimension() == 3)
    {
      pos[0] = centerIndex[0];
      pos[1] = centerIndex[1];
      pos[2] = centerIndex[2];
    }
    else
      return;
  }

  GetLabelSet(layer)->GetLabel(pixelValue)->SetCenterOfMassIndex(pos);
  this->GetSlicedGeometry()->IndexToWorld(pos, pos);
  GetLabelSet(layer)->GetLabel(pixelValue)->SetCenterOfMassCoordinates(pos);
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

  int numberOfTargetLabels = this->GetNumberOfLabels(GetActiveLayer()) - 1; // skip exterior
  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    PixelType sourceValue = sourceIter.Get();
    PixelType targetValue = targetIter.Get();
    if ( (sourceValue != 0) && !this->GetLabel(targetValue)->GetLocked() ) // skip exterior and locked labels
    {
      targetIter.Set( sourceValue + numberOfTargetLabels );
    }
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::LayerContainerToImageProcessing(ImageType* target, unsigned int layer)
{
  typename ImageType::Pointer itkSource;
  mitk::CastToItkImage(m_LayerContainer[layer], itkSource);
  typedef itk::ImageRegionConstIterator< ImageType >  SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >       TargetIteratorType;

  SourceIteratorType sourceIter( itkSource, itkSource->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( target, target->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while(!sourceIter.IsAtEnd())
  {
    targetIter.Set( sourceIter.Get() );
    ++sourceIter;
    ++targetIter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::ImageToLayerContainerProcessing(ImageType* source, unsigned int layer) const
{
  typename ImageType::Pointer itkTarget;
  mitk::CastToItkImage(m_LayerContainer[layer], itkTarget);

  typedef itk::ImageRegionConstIterator< ImageType >   SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >        TargetIteratorType;

  SourceIteratorType sourceIter( source, source->GetLargestPossibleRegion() );
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
void mitk::LabelSetImage::EraseLabelProcessing(ImageType* itkImage, PixelType pixelValue, unsigned int /*layer*/)
{
  typedef itk::ImageRegionIterator< ImageType > IteratorType;

  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  while (!iter.IsAtEnd())
  {
    PixelType value = iter.Get();

    if (value == pixelValue)
    {
      iter.Set( 0 );
    }
    ++iter;
  }
}

template < typename ImageType >
void mitk::LabelSetImage::MergeLabelProcessing(ImageType* itkImage, PixelType pixelValue, PixelType index)
{
  typedef itk::ImageRegionIterator< ImageType > IteratorType;

  IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
  iter.GoToBegin();

  while (!iter.IsAtEnd())
  {
    if (iter.Get() == index)
    {
      iter.Set( pixelValue );
    }
    ++iter;
  }
}

bool mitk::Equal(const mitk::LabelSetImage& leftHandSide, const mitk::LabelSetImage& rightHandSide, ScalarType eps, bool verbose)
{
  bool returnValue = true;

  /* LabelSetImage members */

  MITK_INFO(verbose) << "--- LabelSetImage Equal ---";

  // number layers
  returnValue = leftHandSide.GetNumberOfLayers() == rightHandSide.GetNumberOfLayers();
  if(!returnValue)
  {
    MITK_INFO(verbose) << "Number of layers not equal.";
    return false;
  }

  // total number labels
  returnValue = leftHandSide.GetTotalNumberOfLabels() == rightHandSide.GetTotalNumberOfLabels();
  if(!returnValue)
  {
    MITK_INFO(verbose) << "Total number of labels not equal.";
    return false;
  }

  // active layer
  returnValue = leftHandSide.GetActiveLayer() == rightHandSide.GetActiveLayer();
  if(!returnValue)
  {
    MITK_INFO(verbose) << "Active layer not equal.";
    return false;
  }

  // working image data
  returnValue = mitk::Equal((const mitk::Image&)leftHandSide,(const mitk::Image&)rightHandSide,eps,verbose);
  if(!returnValue)
  {
    MITK_INFO(verbose) << "Working image data not equal.";
    return false;
  }


  for(unsigned int layerIndex = 0 ; layerIndex < leftHandSide.GetNumberOfLayers(); layerIndex++)
  {
    // layer image data
    returnValue = mitk::Equal(*leftHandSide.GetLayerImage(layerIndex),*rightHandSide.GetLayerImage(layerIndex),eps,verbose);
    if(!returnValue)
    {
      MITK_INFO(verbose) << "Layer image data not equal.";
      return false;
    }
    // layer labelset data

    returnValue = mitk::Equal(*leftHandSide.GetLabelSet(layerIndex),*rightHandSide.GetLabelSet(layerIndex),eps,verbose);
    if(!returnValue)
    {
      MITK_INFO(verbose) << "Layer labelset data not equal.";
      return false;
    }

  }

  return returnValue;
}
