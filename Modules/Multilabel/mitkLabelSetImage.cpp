/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
#include <itkLabelGeometryImageFilter.h>
//#include <itkRelabelComponentImageFilter.h>

#include <itkCommand.h>

#include <itkBinaryFunctorImageFilter.h>


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
  : mitk::Image(), m_UnlabeledLabelLock(false), m_ActiveLayer(0), m_activeLayerInvalid(false)
{
  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

mitk::LabelSetImage::LabelSetImage(const mitk::LabelSetImage &other)
  : Image(other),
    m_UnlabeledLabelLock(other.m_UnlabeledLabelLock),
    m_ActiveLayer(other.GetActiveLayer()),
    m_activeLayerInvalid(false)
{
  for (unsigned int i = 0; i < other.GetNumberOfLayers(); i++)
  {
    // Clone LabelSet data
    mitk::LabelSet::Pointer lsClone = other.GetLabelSet(i)->Clone();

    this->RegisterLabelSet(lsClone);

    m_LabelSetContainer.push_back(lsClone);

    // clone layer Image data
    mitk::Image::Pointer liClone = other.GetLayerImage(i)->Clone();
    m_LayerContainer.push_back(liClone);
  }

  this->ReinitMaps();

  // Add some DICOM Tags as properties to segmentation image
  DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(this);
}

void mitk::LabelSetImage::OnLabelSetModified()
{
  Superclass::Modified();
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
  if (this->GetNumberOfLayers() == 0)
  {
    AddLayer();
  }
}

mitk::LabelSetImage::~LabelSetImage()
{
  for (auto ls : m_LabelSetContainer)
  {
    this->ReleaseLabelSet(ls);
  }
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

void mitk::LabelSetImage::RegisterLabelSet(mitk::LabelSet* ls)
{
  // add modified event listener to LabelSet (listen to LabelSet changes)
  itk::SimpleMemberCommand<Self>::Pointer command = itk::SimpleMemberCommand<Self>::New();
  command->SetCallbackFunction(this, &mitk::LabelSetImage::OnLabelSetModified);
  ls->AddObserver(itk::ModifiedEvent(), command);

  ls->AddLabelEvent.AddListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelAdded));
  ls->ModifyLabelEvent.AddListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelModified));
  ls->RemoveLabelEvent.AddListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelRemoved));

  ls->m_ReservedLabelValuesFunctor = [this]() {return this->GetUsedLabelValues(); };
}

void mitk::LabelSetImage::ReleaseLabelSet(mitk::LabelSet* ls)
{
  ls->RemoveAllObservers();

  ls->AddLabelEvent.RemoveListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelAdded));
  ls->ModifyLabelEvent.RemoveListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelModified));
  ls->RemoveLabelEvent.RemoveListener(mitk::MessageDelegate1<LabelSetImage, LabelValueType>(
    this, &LabelSetImage::OnLabelRemoved));

  ls->m_ReservedLabelValuesFunctor = nullptr;
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

  this->OnGroupRemoved(layerToDelete);
  this->Modified();
}

void mitk::LabelSetImage::RemoveGroup(GroupIndexType indexToDelete)
{
  const auto activeIndex = GetActiveLayer();

  // remove all observers from active label set
  GetLabelSet(indexToDelete)->RemoveAllObservers();

  // set the active layer to one below, if exists.
  if (activeIndex>indexToDelete)
  {
    SetActiveLayer(activeIndex - 1);
  }
  else if (activeIndex==indexToDelete)
  {
    // we are deleting layer zero, it should not be copied back into the vector
    m_activeLayerInvalid = true;
  }

  // remove labelset and image data
  m_LabelSetContainer.erase(m_LabelSetContainer.begin() + indexToDelete);
  m_LayerContainer.erase(m_LayerContainer.begin() + indexToDelete);

  if (indexToDelete == activeIndex)
  { //enforces the new active layer to be set and copied
    auto newActiveIndex = indexToDelete < GetNumberOfLayers() ? indexToDelete : GetNumberOfLayers() - 1;
    this->SetActiveLayer(newActiveIndex);
  }

  this->OnGroupRemoved(indexToDelete);
  this->Modified();
}

mitk::LabelSetImage::LabelValueVectorType mitk::LabelSetImage::GetUsedLabelValues() const
{
  LabelValueVectorType result = { UnlabeledValue };

  for (auto [value, label] : m_LabelMap)
  {
    result.emplace_back(value);
  }
  return result;
}

unsigned int mitk::LabelSetImage::AddLayer(mitk::LabelSet::Pointer labelSet)
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

  return this->AddLayer(newImage, labelSet);
}

unsigned int mitk::LabelSetImage::AddLayer(mitk::Image::Pointer layerImage, mitk::LabelSet::Pointer labelSet)
{
  unsigned int newLabelSetId = m_LayerContainer.size();

  // Add labelset to layer
  mitk::LabelSet::Pointer ls;
  if (labelSet.IsNotNull())
  {
    ls = labelSet;
  }
  else
  {
    ls = mitk::LabelSet::New();
    ls->SetActiveLabel(UnlabeledValue);
  }

  ls->SetLayer(newLabelSetId);

  // push a new working image for the new layer
  m_LayerContainer.push_back(layerImage);

  // push a new labelset for the new layer
  m_LabelSetContainer.push_back(ls);

  RegisterLabelSet(ls);
  this->ReinitMaps();

  SetActiveLayer(newLabelSetId);
  this->Modified();
  this->OnGroupAdded(newLabelSetId);

  return newLabelSetId;
}

void mitk::LabelSetImage::AddLabelSetToLayer(const unsigned int layerIdx, const mitk::LabelSet* labelSet)
{
  if (m_LayerContainer.size() <= layerIdx)
  {
    mitkThrow() << "Trying to add labelSet to non-existing layer.";
  }

  auto clonedLabelSet = labelSet->Clone();

  this->RegisterLabelSet(clonedLabelSet);

  std::vector<GroupIndexType> addedGroups;

  if (layerIdx < m_LabelSetContainer.size())
  {
    if (m_LabelSetContainer[layerIdx].IsNotNull())
    {
      this->ReleaseLabelSet(m_LabelSetContainer[layerIdx]);
    }

    m_LabelSetContainer[layerIdx] = clonedLabelSet;
  }
  else
  {
    while (layerIdx >= m_LabelSetContainer.size())
    {
      mitk::LabelSet::Pointer defaultLabelSet = mitk::LabelSet::New();
      defaultLabelSet->SetActiveLabel(UnlabeledValue);
      defaultLabelSet->SetLayer(m_LabelSetContainer.size());
      this->RegisterLabelSet(defaultLabelSet);
      this->ReinitMaps();
      m_LabelSetContainer.push_back(defaultLabelSet);
      addedGroups.emplace_back(m_LabelSetContainer.size() - 1);
    }
    m_LabelSetContainer.push_back(clonedLabelSet);
    addedGroups.emplace_back(m_LabelSetContainer.size() - 1);
  }

  this->ReinitMaps();

  for (auto groupID : addedGroups)
  {
    this->m_GroupAddedMessage.Send(groupID);
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

void mitk::LabelSetImage::ClearBuffer()
{
  try
  {
    if (this->GetDimension() == 4)
    { //remark: this extra branch was added, because LabelSetImage instances can be
      //dynamic (4D), but AccessByItk by support only supports 2D and 3D.
      //The option to change the CMake default dimensions for AccessByItk was
      //dropped (for details see discussion in T28756)
      AccessFixedDimensionByItk(this, ClearBufferProcessing,4);
    }
    else
    {
      AccessByItk(this, ClearBufferProcessing);
    }
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
  this->m_LabelModifiedMessage.Send(sourcePixelValue);
  this->m_LabelModifiedMessage.Send(pixelValue);
  this->m_LabelsChangedMessage.Send({ sourcePixelValue, pixelValue });
  Modified();
}

void mitk::LabelSetImage::MergeLabels(PixelType pixelValue, const std::vector<PixelType>& vectorOfSourcePixelValues, unsigned int layer)
{
  try
  {
    for (unsigned int idx = 0; idx < vectorOfSourcePixelValues.size(); idx++)
    {
      AccessByItk_2(this, MergeLabelProcessing, pixelValue, vectorOfSourcePixelValues[idx]);
      this->m_LabelModifiedMessage.Send(vectorOfSourcePixelValues[idx]);
    }
  }
  catch (itk::ExceptionObject &e)
  {
    mitkThrow() << e.GetDescription();
  }
  GetLabelSet(layer)->SetActiveLabel(pixelValue);
  this->m_LabelModifiedMessage.Send(pixelValue);
  auto modifiedValues = vectorOfSourcePixelValues;
  modifiedValues.push_back(pixelValue);
  this->m_LabelsChangedMessage.Send(modifiedValues);

  Modified();
}

void mitk::LabelSetImage::RemoveLabel(LabelValueType pixelValue)
{
  auto groupID = this->GetGroupIndexOfLabel(pixelValue);

  //first erase the pixel content (also triggers a LabelModified event)
  this->EraseLabel(pixelValue);
  //now remove the label entry itself
  this->GetLabelSet(groupID)->RemoveLabel(pixelValue);
  // in the interim version triggered by label set events: this->m_LabelRemovedMessage.Send(pixelValue);
  this->m_LabelsChangedMessage.Send({ pixelValue });
  this->m_GroupModifiedMessage.Send(groupID);
}

void mitk::LabelSetImage::RemoveLabels(const std::vector<PixelType>& VectorOfLabelPixelValues)
{
  for (unsigned int idx = 0; idx < VectorOfLabelPixelValues.size(); idx++)
  {
    this->RemoveLabel(VectorOfLabelPixelValues[idx]);
    this->m_LabelsChangedMessage.Send({ VectorOfLabelPixelValues[idx] });
  }
}

void mitk::LabelSetImage::EraseLabel(PixelType pixelValue)
{
  try
  {
    auto groupID = this->GetGroupIndexOfLabel(pixelValue);

    mitk::Image* groupImage = this->GetActiveLayer() != groupID
       ? this->GetLayerImage(groupID)
       : this;

    if (4 == this->GetDimension())
    {
      AccessFixedDimensionByItk_1(groupImage, EraseLabelProcessing, 4, pixelValue);
    }
    else
    {
      AccessByItk_1(groupImage, EraseLabelProcessing, pixelValue);
    }
  }
  catch (const itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }

  this->m_LabelModifiedMessage.Send(pixelValue);
  this->m_LabelsChangedMessage.Send({ pixelValue });
  Modified();
}

void mitk::LabelSetImage::EraseLabels(const std::vector<PixelType>& VectorOfLabelPixelValues)
{
  for (unsigned int idx = 0; idx < VectorOfLabelPixelValues.size(); idx++)
  {
    this->EraseLabel(VectorOfLabelPixelValues[idx]);
  }
}

mitk::Label *mitk::LabelSetImage::GetActiveLabel(unsigned int layer)
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer]->GetActiveLabel();
}

const mitk::Label* mitk::LabelSetImage::GetActiveLabel(unsigned int layer) const
{
  if (m_LabelSetContainer.size() <= layer)
    return nullptr;
  else
    return m_LabelSetContainer[layer]->GetActiveLabel();
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

const mitk::LabelSet* mitk::LabelSetImage::GetActiveLabelSet() const
{
  if (m_LabelSetContainer.size() == 0)
    return nullptr;
  else
    return m_LabelSetContainer[GetActiveLayer()].GetPointer();
}

void mitk::LabelSetImage::UpdateCenterOfMass(PixelType pixelValue)
{
  this->UpdateCenterOfMass(pixelValue, this->GetGroupIndexOfLabel(pixelValue));
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
    // mask->Initialize(this) does not work here if this label set image has a single slice,
    // since the mask would be automatically flattened to a 2-d image, whereas we expect the
    // original dimension of this label set image. Hence, initialize the mask more explicitly:
    mask->Initialize(this->GetPixelType(), this->GetDimension(), this->GetDimensions());
    mask->SetTimeGeometry(this->GetTimeGeometry()->Clone());

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

    if (LabelSetImage::UnlabeledValue!=sourceValue && !this->ExistLabel(sourceValue))
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

    if ((sourceValue != UnlabeledValue) &&
        (forceOverwrite || !this->IsLabelLocked(targetValue))) // skip unlabeled pixels and locked labels
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
  if (ImageType::GetImageDimension() != 3)
  {
    return;
  }

  auto labelGeometryFilter = itk::LabelGeometryImageFilter<ImageType>::New();
  labelGeometryFilter->SetInput(itkImage);
  labelGeometryFilter->Update();
  auto centroid = labelGeometryFilter->GetCentroid(pixelValue);

  mitk::Point3D pos;
  pos[0] = centroid[0];
  pos[1] = centroid[1];
  pos[2] = centroid[2];

  GetLabelSet(layer)->GetLabel(pixelValue)->SetCenterOfMassIndex(pos);
  this->GetSlicedGeometry()->IndexToWorld(pos, pos); // TODO: TimeGeometry?
  GetLabelSet(layer)->GetLabel(pixelValue)->SetCenterOfMassCoordinates(pos);
}

template <typename ImageType>
void mitk::LabelSetImage::ClearBufferProcessing(ImageType *itkImage)
{
  itkImage->FillBuffer(0);
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
void mitk::LabelSetImage::EraseLabelProcessing(ImageType *itkImage, PixelType pixelValue)
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


void mitk::LabelSetImage::OnLabelAdded(LabelValueType labelValue)
{
  Label* label = nullptr;

  unsigned int layerID = 0;
  for (; layerID < this->GetNumberOfLayers(); ++layerID)
  {
    label = this->GetLabel(labelValue, layerID);
    if (nullptr != label) break;
  }

  if (!label) mitkThrow() << "Wrong internal state. OnLabelAdded was triggered, but label cannot be found. Invalid label: " << labelValue;

  AddLabelToMap(labelValue, label, layerID);

  this->m_LabelAddedMessage.Send(labelValue);
}

void mitk::LabelSetImage::AddLabelToMap(LabelValueType labelValue, mitk::Label* label, GroupIndexType groupID)
{
  if (m_LabelMap.find(labelValue)!=m_LabelMap.end())
    mitkThrow() << "Segmentation is in an invalid state: Label value collision. A label was added with a LabelValue already in use. LabelValue: " << labelValue;

  m_LabelMap[labelValue] = label;
  m_LabelToGroupMap[labelValue] = groupID;
  auto groupFinding = m_GroupToLabelMap.find(groupID);
  if (groupFinding == m_GroupToLabelMap.end())
  {
    m_GroupToLabelMap[groupID] = { labelValue };
  }
  else
  {
    m_GroupToLabelMap[groupID].push_back(labelValue);
  }
}

void mitk::LabelSetImage::OnLabelModified(LabelValueType labelValue)
{
  this->m_LabelModifiedMessage.Send(labelValue);
}

void mitk::LabelSetImage::OnLabelRemoved(LabelValueType labelValue)
{
  m_LabelMap.erase(labelValue);
  auto finding = m_LabelToGroupMap.find(labelValue);
  if (finding != m_LabelToGroupMap.end())
  {
    auto labelsInGroup = m_GroupToLabelMap[finding->second];
    auto labelFinding = std::find(labelsInGroup.begin(), labelsInGroup.end(),finding->second);
    if (labelFinding != labelsInGroup.end())
    {
      labelsInGroup.erase(labelFinding);
    }
    m_LabelToGroupMap.erase(labelValue);
  }

  this->m_LabelRemovedMessage.Send(labelValue);
}

void mitk::LabelSetImage::OnGroupAdded(GroupIndexType groupIndex)
{
  this->m_GroupToLabelMap.insert(std::make_pair(groupIndex, LabelValueVectorType()));

  this->m_GroupAddedMessage.Send(groupIndex);
}

void mitk::LabelSetImage::OnGroupModified(GroupIndexType groupIndex)
{
  this->m_GroupModifiedMessage.Send(groupIndex);
}

void mitk::LabelSetImage::OnGroupRemoved(GroupIndexType groupIndex)
{
  this->ReinitMaps();
  this->m_GroupRemovedMessage.Send(groupIndex);
}

// future implementation for T28524
//bool mitk::LabelSetImage::ExistLabel(LabelValueType value, GroupIndexType groupIndex) const
//{
//  auto finding = m_LabelToGroupMap.find(value);
//  if (m_LabelToGroupMap.end() != finding)
//  {
//    return finding->second == groupIndex;
//  }
//  return false;
//}
//
//bool mitk::LabelSetImage::ExistGroup(GroupIndexType index) const
//{
//  return index < m_LabelSetContainer.size();
//}

bool mitk::LabelSetImage::ExistGroup(GroupIndexType index) const
{
  return index < m_LabelSetContainer.size();
}

bool mitk::LabelSetImage::IsLabelInGroup(LabelValueType value) const
{
  GroupIndexType dummy;
  return this->IsLabelInGroup(value, dummy);
}

bool mitk::LabelSetImage::IsLabelInGroup(LabelValueType value, GroupIndexType& groupIndex) const
{
  auto finding = m_LabelToGroupMap.find(value);
  if (m_LabelToGroupMap.end() != finding)
  {
    groupIndex = finding->second;
    return true;
  }
  return false;
}

mitk::LabelSetImage::GroupIndexType mitk::LabelSetImage::GetGroupIndexOfLabel(LabelValueType value) const
{
  auto finding = m_LabelToGroupMap.find(value);
  if (m_LabelToGroupMap.end() == finding)
  {
    mitkThrow()<< "Cannot deduce group index. Passed label value does not exist. Value: "<< value;
  }
  return finding->second;
}


const mitk::Label* mitk::LabelSetImage::GetLabel(LabelValueType value) const
{
  auto finding = m_LabelMap.find(value);
  if (m_LabelMap.end() != finding)
  {
    return finding->second;
  }
  return nullptr;
};

mitk::Label* mitk::LabelSetImage::GetLabel(LabelValueType value)
{
  auto finding = m_LabelMap.find(value);
  if (m_LabelMap.end() != finding)
  {
    return finding->second;
  }
  return nullptr;
};

bool mitk::LabelSetImage::IsLabelLocked(LabelValueType value) const
{
  if (value == UnlabeledValue)
  {
    return m_UnlabeledLabelLock;
  }

  const auto label = this->GetLabel(value);
  return label->GetLocked();
}

const mitk::LabelSetImage::ConstLabelVectorType mitk::LabelSetImage::GetLabels() const
{
  ConstLabelVectorType result;
  for (auto [value, label] : m_LabelMap)
  {
    result.emplace_back(label);
  }
  return result;
}

const mitk::LabelSetImage::LabelVectorType mitk::LabelSetImage::GetLabels()
{
  LabelVectorType result;
  for (auto [value, label] : m_LabelMap)
  {
    result.emplace_back(label);
  }
  return result;
}

const mitk::LabelSetImage::ConstLabelVectorType mitk::LabelSetImage::GetLabelsInGroup(GroupIndexType index) const
{
  if (!this->ExistGroup(index))
    mitkThrow() << "Cannot get labels of an invalid group. Invalid group index: " << index;

  mitk::LabelSetImage::ConstLabelVectorType result;
  const auto labelValues = m_GroupToLabelMap.find(index)->second;

  for (const auto& labelValue : labelValues)
  {
    const auto* label = this->GetLabel(labelValue);

    if (label != nullptr)
      result.emplace_back(label);
  }

  return result;
}

const mitk::LabelSetImage::LabelVectorType mitk::LabelSetImage::GetLabelsInGroup(GroupIndexType index)
{
  if (!this->ExistGroup(index))
    mitkThrow() << "Cannot get labels of an invalid group. Invalid group index: " << index;

  mitk::LabelSetImage::LabelVectorType result;
  const auto labelValues = m_GroupToLabelMap[index];

  for (const auto& labelValue : labelValues)
  {
    auto* label = this->GetLabel(labelValue);

    if (label != nullptr)
      result.emplace_back(label);
  }

  return result;
}

void mitk::LabelSetImage::ReinitMaps()
{
  this->m_LabelMap.clear();
  this->m_LabelToGroupMap.clear();
  this->m_GroupToLabelMap.clear();

  for (GroupIndexType layerID = 0; layerID < this->GetNumberOfLayers(); ++layerID)
  {
    auto labelSet = this->GetLabelSet(layerID);

    if (labelSet->GetNumberOfLabels() != 0)
    {
      for (auto iter = labelSet->IteratorBegin(); iter != labelSet->IteratorEnd(); ++iter)
      {
        if (iter->first != UnlabeledValue)
          this->AddLabelToMap(iter->first, iter->second, layerID);
      }
    }
    else
    {
      m_GroupToLabelMap[layerID] = {};
    }
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

/** Functor class that implements the label transfer and is used in conjunction with the itk::BinaryFunctorImageFilter.
* For details regarding the usage of the filter and the functor patterns, please see info of itk::BinaryFunctorImageFilter.
*/
template <class TDestinationPixel, class TSourcePixel, class TOutputpixel>
class LabelTransferFunctor
{

public:
  LabelTransferFunctor() {};

  LabelTransferFunctor(const mitk::LabelSet* destinationLabelSet, mitk::Label::PixelType sourceBackground,
    mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked,
    mitk::Label::PixelType sourceLabel, mitk::Label::PixelType newDestinationLabel, mitk::MultiLabelSegmentation::MergeStyle mergeStyle,
    mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle) :
    m_DestinationLabelSet(destinationLabelSet), m_SourceBackground(sourceBackground),
    m_DestinationBackground(destinationBackground), m_DestinationBackgroundLocked(destinationBackgroundLocked),
    m_SourceLabel(sourceLabel), m_NewDestinationLabel(newDestinationLabel), m_MergeStyle(mergeStyle), m_OverwriteStyle(overwriteStyle)
  {
  };

  ~LabelTransferFunctor() {};

  bool operator!=(const LabelTransferFunctor& other)const
  {
    return !(*this == other);
  }
  bool operator==(const LabelTransferFunctor& other) const
  {
    return this->m_SourceBackground == other.m_SourceBackground &&
      this->m_DestinationBackground == other.m_DestinationBackground &&
      this->m_DestinationBackgroundLocked == other.m_DestinationBackgroundLocked &&
      this->m_SourceLabel == other.m_SourceLabel &&
      this->m_NewDestinationLabel == other.m_NewDestinationLabel &&
      this->m_MergeStyle == other.m_MergeStyle &&
      this->m_OverwriteStyle == other.m_OverwriteStyle &&
      this->m_DestinationLabelSet == other.m_DestinationLabelSet;
  }

  LabelTransferFunctor& operator=(const LabelTransferFunctor& other)
  {
    this->m_DestinationLabelSet = other.m_DestinationLabelSet;
    this->m_SourceBackground = other.m_SourceBackground;
    this->m_DestinationBackground = other.m_DestinationBackground;
    this->m_DestinationBackgroundLocked = other.m_DestinationBackgroundLocked;
    this->m_SourceLabel = other.m_SourceLabel;
    this->m_NewDestinationLabel = other.m_NewDestinationLabel;
    this->m_MergeStyle = other.m_MergeStyle;
    this->m_OverwriteStyle = other.m_OverwriteStyle;

    return *this;
  }

  inline TOutputpixel operator()(const TDestinationPixel& existingDestinationValue, const TSourcePixel& existingSourceValue)
  {
    if (existingSourceValue == this->m_SourceLabel)
    {
      if (mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks == this->m_OverwriteStyle)
      {
        return this->m_NewDestinationLabel;
      }
      else
      {
        if (existingDestinationValue == m_DestinationBackground)
        {
          if (!m_DestinationBackgroundLocked)
          {
            return this->m_NewDestinationLabel;
          }
        }
        else
        {
          auto label = this->m_DestinationLabelSet->GetLabel(existingDestinationValue);
          if (nullptr == label || !label->GetLocked())
          {
            return this->m_NewDestinationLabel;
          }
        }
      }
    }
    else if (mitk::MultiLabelSegmentation::MergeStyle::Replace == this->m_MergeStyle
      && existingSourceValue == this->m_SourceBackground
      && existingDestinationValue == this->m_NewDestinationLabel
      && (mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks == this->m_OverwriteStyle
          || !this->m_DestinationBackgroundLocked))
    {
      return this->m_DestinationBackground;
    }

    return existingDestinationValue;
  }

private:
  const mitk::LabelSet* m_DestinationLabelSet = nullptr;
  mitk::Label::PixelType m_SourceBackground = 0;
  mitk::Label::PixelType m_DestinationBackground = 0;
  bool m_DestinationBackgroundLocked = false;
  mitk::Label::PixelType m_SourceLabel = 1;
  mitk::Label::PixelType m_NewDestinationLabel = 1;
  mitk::MultiLabelSegmentation::MergeStyle m_MergeStyle = mitk::MultiLabelSegmentation::MergeStyle::Replace;
  mitk::MultiLabelSegmentation::OverwriteStyle m_OverwriteStyle = mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks;
};

/**Helper function used by TransferLabelContentAtTimeStep to allow the templating over different image dimensions in conjunction of AccessFixedPixelTypeByItk_n.*/
template<unsigned int VImageDimension>
void TransferLabelContentAtTimeStepHelper(const itk::Image<mitk::Label::PixelType, VImageDimension>* itkSourceImage, mitk::Image* destinationImage,
  const mitk::LabelSet* destinationLabelSet, mitk::Label::PixelType sourceBackground, mitk::Label::PixelType destinationBackground,
  bool destinationBackgroundLocked, mitk::Label::PixelType sourceLabel, mitk::Label::PixelType newDestinationLabel, mitk::MultiLabelSegmentation::MergeStyle mergeStyle, mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
{
  typedef itk::Image<mitk::Label::PixelType, VImageDimension> ContentImageType;
  typename ContentImageType::Pointer itkDestinationImage;
  mitk::CastToItkImage(destinationImage, itkDestinationImage);

  auto sourceRegion = itkSourceImage->GetLargestPossibleRegion();
  auto relevantRegion = itkDestinationImage->GetLargestPossibleRegion();
  bool overlapping = relevantRegion.Crop(sourceRegion);

  if (!overlapping)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage and destinationImage seem to have no overlapping image region.";
  }

  typedef LabelTransferFunctor <mitk::Label::PixelType, mitk::Label::PixelType, mitk::Label::PixelType> LabelTransferFunctorType;
  typedef itk::BinaryFunctorImageFilter<ContentImageType, ContentImageType, ContentImageType, LabelTransferFunctorType> FilterType;

  LabelTransferFunctorType transferFunctor(destinationLabelSet, sourceBackground, destinationBackground,
    destinationBackgroundLocked, sourceLabel, newDestinationLabel, mergeStyle, overwriteStyle);

  auto transferFilter = FilterType::New();

  transferFilter->SetFunctor(transferFunctor);
  transferFilter->InPlaceOn();
  transferFilter->SetInput1(itkDestinationImage);
  transferFilter->SetInput2(itkSourceImage);
  transferFilter->GetOutput()->SetRequestedRegion(relevantRegion);

  transferFilter->Update();
}

void mitk::TransferLabelContentAtTimeStep(
  const Image* sourceImage, Image* destinationImage, const mitk::LabelSet* destinationLabelSet, const TimeStepType timeStep, mitk::Label::PixelType sourceBackground,
  mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked, std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationImage must not be null.";
  }
  if (nullptr == destinationLabelSet)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationLabelSet must not be null";
  }

  if (sourceImage == destinationImage && labelMapping.size() > 1)
  {
    MITK_DEBUG << "Warning. Using TransferLabelContentAtTimeStep or TransferLabelContent with equal source and destination and more then on label to transfer, can lead to wrong results. Please see documentation and verify that the usage is OK.";
  }

  Image::ConstPointer sourceImageAtTimeStep = SelectImageByTimeStep(sourceImage, timeStep);
  Image::Pointer destinationImageAtTimeStep = SelectImageByTimeStep(destinationImage, timeStep);

  if (nullptr == sourceImageAtTimeStep)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage does not have the requested time step: " << timeStep;
  }
  if (nullptr == destinationImageAtTimeStep)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; destinationImage does not have the requested time step: " << timeStep;
  }

  for (const auto& [sourceLabel, newDestinationLabel] : labelMapping)
  {
    if (LabelSetImage::UnlabeledValue!=newDestinationLabel && nullptr == destinationLabelSet->GetLabel(newDestinationLabel))
    {
      mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep. Defined destination label does not exist in destinationImage. newDestinationLabel: " << newDestinationLabel;
    }

    AccessFixedPixelTypeByItk_n(sourceImageAtTimeStep, TransferLabelContentAtTimeStepHelper, (Label::PixelType), (destinationImageAtTimeStep, destinationLabelSet, sourceBackground, destinationBackground, destinationBackgroundLocked, sourceLabel, newDestinationLabel, mergeStyle, overwriteStlye));
    destinationLabelSet->ModifyLabelEvent.Send(newDestinationLabel);
  }
  destinationImage->Modified();
}

void mitk::TransferLabelContent(
  const Image* sourceImage, Image* destinationImage, const mitk::LabelSet* destinationLabelSet, mitk::Label::PixelType sourceBackground,
  mitk::Label::PixelType destinationBackground, bool destinationBackgroundLocked, std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; destinationImage must not be null.";
  }

  const auto sourceTimeStepCount = sourceImage->GetTimeGeometry()->CountTimeSteps();
  if (sourceTimeStepCount != destinationImage->GetTimeGeometry()->CountTimeSteps())
  {
    mitkThrow() << "Invalid call of TransferLabelContent; mismatch between images in number of time steps.";
  }

  for (mitk::TimeStepType i = 0; i < sourceTimeStepCount; ++i)
  {
    TransferLabelContentAtTimeStep(sourceImage, destinationImage, destinationLabelSet, i, sourceBackground,
      destinationBackground, destinationBackgroundLocked, labelMapping, mergeStyle, overwriteStlye);
  }
}

void mitk::TransferLabelContentAtTimeStep(
  const LabelSetImage* sourceImage, LabelSetImage* destinationImage, const TimeStepType timeStep,
  std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep; sourceImage must not be null.";
  }

  const auto destinationLabelSet = destinationImage->GetLabelSet(destinationImage->GetActiveLayer());

  for (const auto& mappingElement : labelMapping)
  {
    if (LabelSetImage::UnlabeledValue != mappingElement.first && !sourceImage->ExistLabel(mappingElement.first, sourceImage->GetActiveLayer()))
    {
      mitkThrow() << "Invalid call of TransferLabelContentAtTimeStep. Defined source label does not exist in sourceImage. SourceLabel: " << mappingElement.first;
    }
  }

  TransferLabelContentAtTimeStep(sourceImage, destinationImage, destinationLabelSet, timeStep, LabelSetImage::UnlabeledValue, LabelSetImage::UnlabeledValue, destinationImage->GetUnlabeledLabelLock(),
    labelMapping, mergeStyle, overwriteStlye);
}

void mitk::TransferLabelContent(
  const LabelSetImage* sourceImage, LabelSetImage* destinationImage,
  std::vector<std::pair<Label::PixelType, Label::PixelType> > labelMapping,
  MultiLabelSegmentation::MergeStyle mergeStyle, MultiLabelSegmentation::OverwriteStyle overwriteStlye)
{
  if (nullptr == sourceImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; sourceImage must not be null.";
  }
  if (nullptr == destinationImage)
  {
    mitkThrow() << "Invalid call of TransferLabelContent; destinationImage must not be null.";
  }

  const auto sourceTimeStepCount = sourceImage->GetTimeGeometry()->CountTimeSteps();
  if (sourceTimeStepCount != destinationImage->GetTimeGeometry()->CountTimeSteps())
  {
    mitkThrow() << "Invalid call of TransferLabelContent; images have no equal number of time steps.";
  }

  for (mitk::TimeStepType i = 0; i < sourceTimeStepCount; ++i)
  {
    TransferLabelContentAtTimeStep(sourceImage, destinationImage, i, labelMapping, mergeStyle, overwriteStlye);
  }
}

