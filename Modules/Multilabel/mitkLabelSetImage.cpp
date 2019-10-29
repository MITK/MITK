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
#include "mitkImageCast.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkImagePixelWriteAccessor.h"
#include "mitkInteractionConst.h"
#include "mitkLookupTableProperty.h"
#include "mitkPadImageFilter.h"
#include "mitkRenderingManager.h"
#include "mitkDICOMSegmentationPropertyHelper.h"
#include "mitkDICOMQIPropertyHelper.h"

#include <vtkCell.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <itkImageRegionIterator.h>
#include <itkQuadEdgeMesh.h>
#include <itkTriangleMeshToBinaryImageFilter.h>
//#include <itkRelabelComponentImageFilter.h>

#include <itkCommand.h>

template <typename TPixel, unsigned int VDimensions>
void SetToZero(itk::Image<TPixel, VDimensions> *source)
{
  source->FillBuffer(0);
}

template <unsigned int VImageDimension = 3>
void CreateLabelMaskProcessing(mitk::Image *layerImage, mitk::Image *mask, mitk::LabelSet::PixelType index)
{
  mitk::ImagePixelReadAccessor<mitk::LabelSet::PixelType, VImageDimension> readAccessor(layerImage);
  mitk::ImagePixelWriteAccessor<mitk::LabelSet::PixelType, VImageDimension> writeAccessor(mask);

  std::size_t numberOfPixels = 1;
  for (int dim = 0; dim < static_cast<int>(VImageDimension); ++dim)
    numberOfPixels *= static_cast<std::size_t>(readAccessor.GetDimension(dim));

  auto src = readAccessor.GetData();
  auto dest = writeAccessor.GetData();

  for (std::size_t i = 0; i < numberOfPixels; ++i)
  {
    if (index == *(src + i))
      *(dest + i) = 1;
  }
}

mitk::LabelSetImage::LabelSetImage()
  : mitk::Image(), m_ActiveLayer(0), m_activeLayerInvalid(false), m_ExteriorLabel(nullptr)
{
  // Iniitlaize Background Label
  mitk::Color color;
  color.Set(0, 0, 0);
  m_ExteriorLabel = mitk::Label::New();
  m_ExteriorLabel->SetColor(color);
  m_ExteriorLabel->SetName("Exterior");
  m_ExteriorLabel->SetOpacity(0.0);
  m_ExteriorLabel->SetLocked(false);
  m_ExteriorLabel->SetValue(0);

  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

mitk::LabelSetImage::LabelSetImage(const mitk::LabelSetImage &other)
  : Image(other),
    m_ActiveLayer(other.GetActiveLayer()),
    m_activeLayerInvalid(false),
    m_ExteriorLabel(other.GetExteriorLabel()->Clone())
{
  for (unsigned int i = 0; i < other.GetNumberOfLayers(); i++)
  {
    // Clone LabelSet data
    mitk::LabelSet::Pointer lsClone = other.GetLabelSet(i)->Clone();
    // add modified event listener to LabelSet (listen to LabelSet changes)
    itk::SimpleMemberCommand<Self>::Pointer command = itk::SimpleMemberCommand<Self>::New();
    command->SetCallbackFunction(this, &mitk::LabelSetImage::OnLabelSetModified);
    lsClone->AddObserver(itk::ModifiedEvent(), command);
    m_LabelSetContainer.push_back(lsClone);

    // clone layer Image data
    mitk::Image::Pointer liClone = other.GetLayerImage(i)->Clone();
    m_LayerContainer.push_back(liClone);
  }

  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

void mitk::LabelSetImage::OnLabelSetModified()
{
  Superclass::Modified();
}

void mitk::LabelSetImage::SetExteriorLabel(mitk::Label *label)
{
  m_ExteriorLabel = label;
}

mitk::Label *mitk::LabelSetImage::GetExteriorLabel()
{
  return m_ExteriorLabel;
}

const mitk::Label *mitk::LabelSetImage::GetExteriorLabel() const
{
  return m_ExteriorLabel;
}

void mitk::LabelSetImage::Initialize(const mitk::Image *other)
{
  mitk::PixelType pixelType(mitk::MakeScalarPixelType<LabelSetImage::PixelType>());
  if (other->GetDimension() == 2)
  {
    const unsigned int dimensions[] = {other->GetDimension(0), other->GetDimension(1), 1};
    Superclass::Initialize(pixelType, 3, dimensions);
  }
  else
  {
    Superclass::Initialize(pixelType, other->GetDimension(), other->GetDimensions());
  }

  auto originalGeometry = other->GetTimeGeometry()->Clone();
  this->SetTimeGeometry(originalGeometry);

  // initialize image memory to zero
  if (4 == this->GetDimension())
  {
    AccessFixedDimensionByItk(this, SetToZero, 4);
  }
  else
  {
    AccessByItk(this, SetToZero);
  }

  // Transfer some general DICOM properties from the source image to derived image (e.g. Patient information,...)
  DICOMQIPropertyHelper::DeriveDICOMSourceProperties(other, this);

  // Add a inital LabelSet ans corresponding image data to the stack
  AddLayer();
}

mitk::LabelSetImage::~LabelSetImage()
{
  m_LabelSetContainer.clear();
}

mitk::Image *mitk::LabelSetImage::GetLayerImage(unsigned int layer)
{
  return m_LayerContainer[layer];
}

const mitk::Image *mitk::LabelSetImage::GetLayerImage(unsigned int layer) const
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

void mitk::LabelSetImage::RemoveLayer()
{
  int layerToDelete = GetActiveLayer();
  // remove all observers from active label set
  GetLabelSet(layerToDelete)->RemoveAllObservers();

  // set the active layer to one below, if exists.
  if (layerToDelete != 0)
  {
    SetActiveLayer(layerToDelete - 1);
  }
  else
  {
    // we are deleting layer zero, it should not be copied back into the vector
    m_activeLayerInvalid = true;
  }

  // remove labelset and image data
  m_LabelSetContainer.erase(m_LabelSetContainer.begin() + layerToDelete);
  m_LayerContainer.erase(m_LayerContainer.begin() + layerToDelete);

  if (layerToDelete == 0)
  {
    this->SetActiveLayer(layerToDelete);
  }

  this->Modified();
}

unsigned int mitk::LabelSetImage::AddLayer(mitk::LabelSet::Pointer lset)
{
  mitk::Image::Pointer newImage = mitk::Image::New();
  newImage->Initialize(this->GetPixelType(),
                       this->GetDimension(),
                       this->GetDimensions(),
                       this->GetImageDescriptor()->GetNumberOfChannels());
  newImage->SetTimeGeometry(this->GetTimeGeometry()->Clone());

  if (newImage->GetDimension() < 4)
  {
    AccessByItk(newImage, SetToZero);
  }
  else
  {
    AccessFixedDimensionByItk(newImage, SetToZero, 4);
  }

  unsigned int newLabelSetId = this->AddLayer(newImage, lset);

  return newLabelSetId;
}

unsigned int mitk::LabelSetImage::AddLayer(mitk::Image::Pointer layerImage, mitk::LabelSet::Pointer lset)
{
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
  // mitk::Label::Pointer exteriorLabel = CreateExteriorLabel();

  // push a new working image for the new layer
  m_LayerContainer.push_back(layerImage);

  // push a new labelset for the new layer
  m_LabelSetContainer.push_back(ls);

  // add modified event listener to LabelSet (listen to LabelSet changes)
  itk::SimpleMemberCommand<Self>::Pointer command = itk::SimpleMemberCommand<Self>::New();
  command->SetCallbackFunction(this, &mitk::LabelSetImage::OnLabelSetModified);
  ls->AddObserver(itk::ModifiedEvent(), command);

  SetActiveLayer(newLabelSetId);
  // MITK_INFO << GetActiveLayer();
  this->Modified();
  return newLabelSetId;
}

void mitk::LabelSetImage::AddLabelSetToLayer(const unsigned int layerIdx, const mitk::LabelSet::Pointer labelSet)
{
  if (m_LayerContainer.size() <= layerIdx)
  {
    mitkThrow() << "Trying to add labelSet to non-existing layer.";
  }

  if (layerIdx < m_LabelSetContainer.size())
  {
    m_LabelSetContainer[layerIdx] = labelSet;
  }
  else
  {
    while (layerIdx >= m_LabelSetContainer.size())
    {
      mitk::LabelSet::Pointer defaultLabelSet = mitk::LabelSet::New();
      defaultLabelSet->AddLabel(GetExteriorLabel());
      defaultLabelSet->SetActiveLabel(0 /*Exterior Label*/);
      defaultLabelSet->SetLayer(m_LabelSetContainer.size());
      m_LabelSetContainer.push_back(defaultLabelSet);
    }
    m_LabelSetContainer.push_back(labelSet);
  }
}

void mitk::LabelSetImage::SetActiveLayer(unsigned int layer)
{
  try
  {
    if (4 == this->GetDimension())
    {
      if ((layer != GetActiveLayer() || m_activeLayerInvalid) && (layer < this->GetNumberOfLayers()))
      {
        BeforeChangeLayerEvent.Send();

        if (m_activeLayerInvalid)
        {
          // We should not write the invalid layer back to the vector
          m_activeLayerInvalid = false;
        }
        else
        {
          AccessFixedDimensionByItk_n(this, ImageToLayerContainerProcessing, 4, (GetActiveLayer()));
        }
        m_ActiveLayer = layer; // only at this place m_ActiveLayer should be manipulated!!! Use Getter and Setter
        AccessFixedDimensionByItk_n(this, LayerContainerToImageProcessing, 4, (GetActiveLayer()));

        AfterChangeLayerEvent.Send();
      }
    }
    else
    {
      if ((layer != GetActiveLayer() || m_activeLayerInvalid) && (layer < this->GetNumberOfLayers()))
      {
        BeforeChangeLayerEvent.Send();

        if (m_activeLayerInvalid)
        {
          // We should not write the invalid layer back to the vector
          m_activeLayerInvalid = false;
        }
        else
        {
          AccessByItk_1(this, ImageToLayerContainerProcessing, GetActiveLayer());
        }
        m_ActiveLayer = layer; // only at this place m_ActiveLayer should be manipulated!!! Use Getter and Setter
        AccessByItk_1(this, LayerContainerToImageProcessing, GetActiveLayer());

        AfterChangeLayerEvent.Send();
      }
    }
  }
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  this->Modified();
}

void mitk::LabelSetImage::Concatenate(mitk::LabelSetImage *other)
{
  const unsigned int *otherDims = other->GetDimensions();
  const unsigned int *thisDims = this->GetDimensions();
  if ((otherDims[0] != thisDims[0]) || (otherDims[1] != thisDims[1]) || (otherDims[2] != thisDims[2]))
    mitkThrow() << "Dimensions do not match.";

  try
  {
    int numberOfLayers = other->GetNumberOfLayers();
    for (int layer = 0; layer < numberOfLayers; ++layer)
    {
      this->SetActiveLayer(layer);
      AccessByItk_1(this, ConcatenateProcessing, other);
      mitk::LabelSet *ls = other->GetLabelSet(layer);
      auto it = ls->IteratorConstBegin();
      auto end = ls->IteratorConstEnd();
      it++; // skip exterior
      while (it != end)
      {
        GetLabelSet()->AddLabel((it->second));
        // AddLabelEvent.Send();
        it++;
      }
    }
  }
  catch (itk::ExceptionObject &e)
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
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
}

bool mitk::LabelSetImage::ExistLabel(PixelType pixelValue) const
{
  bool exist = false;
  for (unsigned int lidx = 0; lidx < GetNumberOfLayers(); lidx++)
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

void mitk::LabelSetImage::MergeLabel(PixelType pixelValue, PixelType sourcePixelValue, unsigned int layer)
{
  try
  {
    AccessByItk_2(this, MergeLabelProcessing, pixelValue, sourcePixelValue);
  }
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  GetLabelSet(layer)->SetActiveLabel(pixelValue);
  Modified();
}

void mitk::LabelSetImage::MergeLabels(PixelType pixelValue, std::vector<PixelType>& vectorOfSourcePixelValues, unsigned int layer)
{
  try
  {
    for (unsigned int idx = 0; idx < vectorOfSourcePixelValues.size(); idx++)
    {
      AccessByItk_2(this, MergeLabelProcessing, pixelValue, vectorOfSourcePixelValues[idx]);
    }
  }
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  GetLabelSet(layer)->SetActiveLabel(pixelValue);
  Modified();
}

void mitk::LabelSetImage::RemoveLabels(std::vector<PixelType> &VectorOfLabelPixelValues, unsigned int layer)
{
  for (unsigned int idx = 0; idx < VectorOfLabelPixelValues.size(); idx++)
  {
    GetLabelSet(layer)->RemoveLabel(VectorOfLabelPixelValues[idx]);
    EraseLabel(VectorOfLabelPixelValues[idx], layer);
  }
}

void mitk::LabelSetImage::EraseLabels(std::vector<PixelType> &VectorOfLabelPixelValues, unsigned int layer)
{
  for (unsigned int i = 0; i < VectorOfLabelPixelValues.size(); i++)
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
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  Modified();
}

mitk::Label *mitk::LabelSetImage::GetActiveLabel(unsigned int layer)
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer]->GetActiveLabel();;
}

mitk::Label *mitk::LabelSetImage::GetLabel(PixelType pixelValue, unsigned int layer) const
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer]->GetLabel(pixelValue);
}

mitk::LabelSet *mitk::LabelSetImage::GetLabelSet(unsigned int layer)
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer].GetPointer();
}

const mitk::LabelSet *mitk::LabelSetImage::GetLabelSet(unsigned int layer) const
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer].GetPointer();
}

mitk::LabelSet *mitk::LabelSetImage::GetActiveLabelSet()
{
  if (m_LabelSetContainer.size() == 0)
    return nullptr;
  else
    return m_LabelSetContainer[GetActiveLayer()].GetPointer();
}

void mitk::LabelSetImage::UpdateCenterOfMass(PixelType pixelValue, unsigned int layer)
{
  if (4 == this->GetDimension())
  {
    AccessFixedDimensionByItk_2(this, CalculateCenterOfMassProcessing, 4, pixelValue, layer);
  }
  else
  {
    AccessByItk_2(this, CalculateCenterOfMassProcessing, pixelValue, layer);
  }
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

void mitk::LabelSetImage::MaskStamp(mitk::Image *mask, bool forceOverwrite)
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

    if (paddedMask.IsNull())
      return;

    AccessByItk_2(this, MaskStampProcessing, paddedMask, forceOverwrite);
  }
  catch (...)
  {
    mitkThrow() << "Could not stamp the provided mask on the selected label.";
  }
}

mitk::Image::Pointer mitk::LabelSetImage::CreateLabelMask(PixelType index, bool useActiveLayer, unsigned int layer)
{
  auto previousActiveLayer = this->GetActiveLayer();
  auto mask = mitk::Image::New();

  try
  {
    mask->Initialize(this);

    auto byteSize = sizeof(LabelSetImage::PixelType);
    for (unsigned int dim = 0; dim < mask->GetDimension(); ++dim)
      byteSize *= mask->GetDimension(dim);

    {
      ImageWriteAccessor accessor(mask);
      memset(accessor.GetData(), 0, byteSize);
    }

    if (!useActiveLayer)
      this->SetActiveLayer(layer);

    if (4 == this->GetDimension())
    {
      ::CreateLabelMaskProcessing<4>(this, mask, index);
    }
    else if (3 == this->GetDimension())
    {
      ::CreateLabelMaskProcessing(this, mask, index);
    }
    else
    {
      mitkThrow();
    }
  }
  catch (...)
  {
    if (!useActiveLayer)
      this->SetActiveLayer(previousActiveLayer);

    mitkThrow() << "Could not create a mask out of the selected label.";
  }

  if (!useActiveLayer)
    this->SetActiveLayer(previousActiveLayer);

  return mask;
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

    mitk::ImageWriteAccessor *accessor = new mitk::ImageWriteAccessor(static_cast<mitk::Image *>(this));
    memset(accessor->GetData(), 0, byteSize);
    delete accessor;

    auto geometry = image->GetTimeGeometry()->Clone();
    this->SetTimeGeometry(geometry);

    if (image->GetDimension() == 3)
    {
      AccessTwoImagesFixedDimensionByItk(this, image, InitializeByLabeledImageProcessing, 3);
    }
    else if (image->GetDimension() == 4)
    {
      AccessTwoImagesFixedDimensionByItk(this, image, InitializeByLabeledImageProcessing, 4);
    }
    else
    {
      mitkThrow() << image->GetDimension() << "-dimensional label set images not yet supported";
    }
  }
  catch (...)
  {
    mitkThrow() << "Could not intialize by provided labeled image.";
  }
  this->Modified();
}

template <typename LabelSetImageType, typename ImageType>
void mitk::LabelSetImage::InitializeByLabeledImageProcessing(LabelSetImageType *labelSetImage, ImageType *image)
{
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> SourceIteratorType;
  typedef itk::ImageRegionIterator<LabelSetImageType> TargetIteratorType;

  TargetIteratorType targetIter(labelSetImage, labelSetImage->GetRequestedRegion());
  targetIter.GoToBegin();

  SourceIteratorType sourceIter(image, image->GetRequestedRegion());
  sourceIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    auto sourceValue = static_cast<PixelType>(sourceIter.Get());
    targetIter.Set(sourceValue);

    if (!this->ExistLabel(sourceValue))
    {
      std::stringstream name;
      name << "object-" << sourceValue;

      double rgba[4];
      m_LabelSetContainer[this->GetActiveLayer()]->GetLookupTable()->GetTableValue(sourceValue, rgba);

      mitk::Color color;
      color.SetRed(rgba[0]);
      color.SetGreen(rgba[1]);
      color.SetBlue(rgba[2]);

      auto label = mitk::Label::New();
      label->SetName(name.str().c_str());
      label->SetColor(color);
      label->SetOpacity(rgba[3]);
      label->SetValue(sourceValue);

      this->GetLabelSet()->AddLabel(label);

      if (GetActiveLabelSet()->GetNumberOfLabels() >= mitk::Label::MAX_LABEL_VALUE ||
          sourceValue >= mitk::Label::MAX_LABEL_VALUE)
        this->AddLayer();
    }

    ++sourceIter;
    ++targetIter;
  }
}

template <typename ImageType>
void mitk::LabelSetImage::MaskStampProcessing(ImageType *itkImage, mitk::Image *mask, bool forceOverwrite)
{
  typename ImageType::Pointer itkMask;
  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageRegionConstIterator<ImageType> SourceIteratorType;
  typedef itk::ImageRegionIterator<ImageType> TargetIteratorType;

  SourceIteratorType sourceIter(itkMask, itkMask->GetLargestPossibleRegion());
  sourceIter.GoToBegin();

  TargetIteratorType targetIter(itkImage, itkImage->GetLargestPossibleRegion());
  targetIter.GoToBegin();

  int activeLabel = this->GetActiveLabel(GetActiveLayer())->GetValue();

  while (!sourceIter.IsAtEnd())
  {
    PixelType sourceValue = sourceIter.Get();
    PixelType targetValue = targetIter.Get();

    if ((sourceValue != 0) &&
        (forceOverwrite || !this->GetLabel(targetValue)->GetLocked())) // skip exterior and locked labels
    {
      targetIter.Set(activeLabel);
    }
    ++sourceIter;
    ++targetIter;
  }

  this->Modified();
}

template <typename ImageType>
void mitk::LabelSetImage::CalculateCenterOfMassProcessing(ImageType *itkImage, PixelType pixelValue, unsigned int layer)
{
  // for now, we just retrieve the voxel in the middle
  typedef itk::ImageRegionConstIterator<ImageType> IteratorType;
  IteratorType iter(itkImage, itkImage->GetLargestPossibleRegion());
  iter.GoToBegin();

  std::vector<typename ImageType::IndexType> indexVector;

  while (!iter.IsAtEnd())
  {
    // TODO fix comparison warning more effective
    if (iter.Get() == pixelValue)
    {
      indexVector.push_back(iter.GetIndex());
    }
    ++iter;
  }

  mitk::Point3D pos;
  pos.Fill(0.0);

  if (!indexVector.empty())
  {
    typename itk::ImageRegionConstIteratorWithIndex<ImageType>::IndexType centerIndex;
    centerIndex = indexVector.at(indexVector.size() / 2);
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
  this->GetSlicedGeometry()->IndexToWorld(pos, pos); // TODO: TimeGeometry?
  GetLabelSet(layer)->GetLabel(pixelValue)->SetCenterOfMassCoordinates(pos);
}

template <typename ImageType>
void mitk::LabelSetImage::ClearBufferProcessing(ImageType *itkImage)
{
  itkImage->FillBuffer(0);
}

// todo: concatenate all layers and not just the active one
template <typename ImageType>
void mitk::LabelSetImage::ConcatenateProcessing(ImageType *itkTarget, mitk::LabelSetImage *other)
{
  typename ImageType::Pointer itkSource = ImageType::New();
  mitk::CastToItkImage(other, itkSource);

  typedef itk::ImageRegionConstIterator<ImageType> ConstIteratorType;
  typedef itk::ImageRegionIterator<ImageType> IteratorType;

  ConstIteratorType sourceIter(itkSource, itkSource->GetLargestPossibleRegion());
  IteratorType targetIter(itkTarget, itkTarget->GetLargestPossibleRegion());

  int numberOfTargetLabels = this->GetNumberOfLabels(GetActiveLayer()) - 1; // skip exterior
  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    PixelType sourceValue = sourceIter.Get();
    PixelType targetValue = targetIter.Get();
    if ((sourceValue != 0) && !this->GetLabel(targetValue)->GetLocked()) // skip exterior and locked labels
    {
      targetIter.Set(sourceValue + numberOfTargetLabels);
    }
    ++sourceIter;
    ++targetIter;
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::LabelSetImage::LayerContainerToImageProcessing(itk::Image<TPixel, VImageDimension> *target,
                                                          unsigned int layer)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typename ImageType::Pointer itkSource;
  // mitk::CastToItkImage(m_LayerContainer[layer], itkSource);
  itkSource = ImageToItkImage<TPixel, VImageDimension>(m_LayerContainer[layer]);
  typedef itk::ImageRegionConstIterator<ImageType> SourceIteratorType;
  typedef itk::ImageRegionIterator<ImageType> TargetIteratorType;

  SourceIteratorType sourceIter(itkSource, itkSource->GetLargestPossibleRegion());
  sourceIter.GoToBegin();

  TargetIteratorType targetIter(target, target->GetLargestPossibleRegion());
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    targetIter.Set(sourceIter.Get());
    ++sourceIter;
    ++targetIter;
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::LabelSetImage::ImageToLayerContainerProcessing(itk::Image<TPixel, VImageDimension> *source,
                                                          unsigned int layer) const
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typename ImageType::Pointer itkTarget;
  // mitk::CastToItkImage(m_LayerContainer[layer], itkTarget);
  itkTarget = ImageToItkImage<TPixel, VImageDimension>(m_LayerContainer[layer]);

  typedef itk::ImageRegionConstIterator<ImageType> SourceIteratorType;
  typedef itk::ImageRegionIterator<ImageType> TargetIteratorType;

  SourceIteratorType sourceIter(source, source->GetLargestPossibleRegion());
  sourceIter.GoToBegin();

  TargetIteratorType targetIter(itkTarget, itkTarget->GetLargestPossibleRegion());
  targetIter.GoToBegin();

  while (!sourceIter.IsAtEnd())
  {
    targetIter.Set(sourceIter.Get());
    ++sourceIter;
    ++targetIter;
  }
}

template <typename ImageType>
void mitk::LabelSetImage::EraseLabelProcessing(ImageType *itkImage, PixelType pixelValue, unsigned int /*layer*/)
{
  typedef itk::ImageRegionIterator<ImageType> IteratorType;

  IteratorType iter(itkImage, itkImage->GetLargestPossibleRegion());
  iter.GoToBegin();

  while (!iter.IsAtEnd())
  {
    PixelType value = iter.Get();

    if (value == pixelValue)
    {
      iter.Set(0);
    }
    ++iter;
  }
}

template <typename ImageType>
void mitk::LabelSetImage::MergeLabelProcessing(ImageType *itkImage, PixelType pixelValue, PixelType index)
{
  typedef itk::ImageRegionIterator<ImageType> IteratorType;

  IteratorType iter(itkImage, itkImage->GetLargestPossibleRegion());
  iter.GoToBegin();

  while (!iter.IsAtEnd())
  {
    if (iter.Get() == index)
    {
      iter.Set(pixelValue);
    }
    ++iter;
  }
}

bool mitk::Equal(const mitk::LabelSetImage &leftHandSide,
                 const mitk::LabelSetImage &rightHandSide,
                 ScalarType eps,
                 bool verbose)
{
  bool returnValue = true;

  /* LabelSetImage members */

  MITK_INFO(verbose) << "--- LabelSetImage Equal ---";

  // number layers
  returnValue = leftHandSide.GetNumberOfLayers() == rightHandSide.GetNumberOfLayers();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Number of layers not equal.";
    return false;
  }

  // total number labels
  returnValue = leftHandSide.GetTotalNumberOfLabels() == rightHandSide.GetTotalNumberOfLabels();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Total number of labels not equal.";
    return false;
  }

  // active layer
  returnValue = leftHandSide.GetActiveLayer() == rightHandSide.GetActiveLayer();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Active layer not equal.";
    return false;
  }

  if (4 == leftHandSide.GetDimension())
  {
    MITK_INFO(verbose) << "Can not compare image data for 4D images - skipping check.";
  }
  else
  {
    // working image data
    returnValue = mitk::Equal((const mitk::Image &)leftHandSide, (const mitk::Image &)rightHandSide, eps, verbose);
    if (!returnValue)
    {
      MITK_INFO(verbose) << "Working image data not equal.";
      return false;
    }
  }

  for (unsigned int layerIndex = 0; layerIndex < leftHandSide.GetNumberOfLayers(); layerIndex++)
  {
    if (4 == leftHandSide.GetDimension())
    {
      MITK_INFO(verbose) << "Can not compare image data for 4D images - skipping check.";
    }
    else
    {
      // layer image data
      returnValue =
        mitk::Equal(*leftHandSide.GetLayerImage(layerIndex), *rightHandSide.GetLayerImage(layerIndex), eps, verbose);
      if (!returnValue)
      {
        MITK_INFO(verbose) << "Layer image data not equal.";
        return false;
      }
    }
    // layer labelset data

    returnValue =
      mitk::Equal(*leftHandSide.GetLabelSet(layerIndex), *rightHandSide.GetLabelSet(layerIndex), eps, verbose);
    if (!returnValue)
    {
      MITK_INFO(verbose) << "Layer labelset data not equal.";
      return false;
    }
  }

  return returnValue;
}
