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


#include <mitkDataCollection.h>
#include <mitkImageCast.h>

// ITK
#include <itkImage.h>
#include <itkComposeImageFilter.h>

mitk::DataCollection::DataCollection()
{
}


mitk::DataCollection::~DataCollection()
{
}

void
mitk::DataCollection::Init(std::string name)
{
  m_Name = name;
  m_Parent = nullptr;
}

size_t mitk::DataCollection::AddData(DataObject::Pointer data, std::string name, std::string filePath)
{
  mitk::DataCollection* dc = dynamic_cast<mitk::DataCollection*>(data.GetPointer());
  if (dc) dc->SetParent(this);

  m_DataVector.push_back(data.GetPointer());
  m_NameVector.push_back(name);
  m_FilePathVector.push_back(filePath);
  size_t lastIndex = m_DataVector.size()-1;
  m_DataNames[name] = lastIndex;
  return lastIndex;
}

void
mitk::DataCollection::SetName(std::string name)
{
  m_Name = name;
}

std::string
mitk::DataCollection::GetName() const
{
  return m_Name;
}

std::string mitk::DataCollection::GetDataFilePath(size_t index) const
{
  if (index >= m_FilePathVector.size())
    mitkThrow() << "No element with the given index";
  return m_FilePathVector[index];
}


size_t
mitk::DataCollection::NameToIndex(std::string name)
{
  size_t index;
  try
  {
    index = m_DataNames.at(name); // only brackets [] cause map to return std element, which in this case is 0
  }
  catch (...)
  {
    mitkThrow() << "No element with the given name: " << name;
  }
  return index;
}

std::string
mitk::DataCollection::IndexToName(size_t index) const
{
  if (index >= m_NameVector.size())
  {
    MITK_ERROR << "DataCollection Index does not exist";
    mitkThrow() << "No element with the given index";
  }
  return m_NameVector[index];
}

bool
mitk::DataCollection::HasElement(std::string name)
{
  bool isInside = (m_DataNames.find(name) != m_DataNames.end());
  return isInside;
}

bool
mitk::DataCollection::HasElement(size_t index)
{
  if (index >= m_NameVector.size())
    return false;
  return true;
}

size_t
mitk::DataCollection::Size() const
{
  return m_DataVector.size();
}

void
mitk::DataCollection::SetData(itk::DataObject::Pointer data, size_t index)
{
  if (!HasElement(index))
    mitkThrow() << "Unkown index. No Element with the given index";

  mitk::DataCollection::Pointer dc = dynamic_cast<mitk::DataCollection*>(data.GetPointer());
  if (dc)
    dc->SetParent(this);

  m_DataVector[index] = data;
}

void
mitk::DataCollection::SetData(itk::DataObject::Pointer data, std::string name)
{
  SetData(data, NameToIndex(name));
}

itk::DataObject::Pointer
mitk::DataCollection::GetData(size_t index)
{
  if (!HasElement(index))
    mitkThrow() << "Unkown index. No Element with the given index";

  return m_DataVector[index];
}

itk::DataObject::Pointer
mitk::DataCollection::GetData(std::string name)
{
  return GetData(NameToIndex(name));
}

template <typename TPixel>
mitk::Image::Pointer
ConvertItkObjectToMitkImage(itk::DataObject * itkData, mitk::Image::Pointer mitkImage)
{
  if (mitkImage.IsNull())
  {
    typename itk::Image<TPixel, 3>::Pointer itkImage = dynamic_cast<itk::Image<TPixel, 3> *>(itkData);
    if (itkImage.IsNotNull())
    {
      mitk::CastToMitkImage(itkImage,mitkImage);
    }
  }
  if (mitkImage.IsNull())
  {
    typename itk::Image<TPixel, 2>::Pointer itkImage = dynamic_cast<itk::Image<TPixel, 2>*>(itkData);
    if (itkImage.IsNotNull())
    {
      mitk::CastToMitkImage(itkImage,mitkImage);
    }
  }
  return mitkImage;
}


mitk::Image::Pointer
mitk::DataCollection::GetMitkImage(size_t index)
{
  itk::DataObject* itkData = GetData(index).GetPointer();
  mitk::Image::Pointer mitkImage =
    dynamic_cast<mitk::Image*>(itkData);

  mitkImage = ConvertItkObjectToMitkImage<double>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<float>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<unsigned char>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<signed char>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<unsigned short>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<signed short>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<unsigned int>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<signed int>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<unsigned long>(itkData, mitkImage);
  mitkImage = ConvertItkObjectToMitkImage<signed long>(itkData, mitkImage);

  return mitkImage;
}

mitk::Image::Pointer
mitk::DataCollection::GetMitkImage(std::string name)
{
  return GetMitkImage(NameToIndex(name));
}

template <class ImageType>
ImageType mitk::DataCollection::GetItkImage(size_t index, ImageType *itkImage)
{
  Image* image = dynamic_cast<Image*> (GetData(index).GetPointer());
    if (image != nullptr)
  {
    CastToItkImage(image, itkImage );
  }
  else if (nullptr != dynamic_cast<ImageType*>(GetData(index)))
    itkImage = dynamic_cast<ImageType*>(GetData(index));
}

template <class ImageType>
ImageType mitk::DataCollection::GetItkImage(std::string name, ImageType* itkImage)
{
  Image* image = dynamic_cast<Image*> (GetData(NameToIndex(name)).GetPointer());
    if (image != nullptr)
  {
    CastToItkImage(image, itkImage );
  }
  else if (nullptr != dynamic_cast<ImageType*> (GetData(NameToIndex(name))))
    itkImage = dynamic_cast<ImageType*> (GetData(NameToIndex(name)));
}

itk::DataObject::Pointer&
mitk::DataCollection::operator[](size_t index)
{
  return m_DataVector[index];
}

itk::DataObject::Pointer&
mitk::DataCollection::operator[](std::string &name)
{
  return operator[](NameToIndex(name));
}

void mitk::DataCollection::SetXMLFile(std::string absoluteXMlFile)
{
  m_XMLFile = absoluteXMlFile;
}

std::string mitk::DataCollection::GetXMLFile()
{
  return m_XMLFile;
}

void mitk::DataCollection::SetParent(mitk::DataCollection *parent)
{
  m_Parent = parent;
}

mitk::DataCollection*
mitk::DataCollection::GetParent()
{
  return m_Parent;
}

void mitk::DataCollection::SetNameForIndex(size_t index, std::string &name)
{
 std::map<std::string, size_t>::iterator it = m_DataNames.find(m_NameVector[index]);
  m_DataNames.erase(it);
  m_DataNames[name] = index;
  m_NameVector[index] = name;

}

bool mitk::DataCollection::RemoveIndex(size_t index)
{
  if (!HasElement(index))
    return false;

  m_DataVector.erase(m_DataVector.begin() + index);
  m_NameVector.erase(m_NameVector.begin() + index);
  m_FilePathVector.erase(m_FilePathVector.begin() + index);

  m_DataNames.clear();
  for (size_t i = 0; i < m_NameVector.size(); ++i)
  {
    m_DataNames[m_NameVector[i]] = i;
  }

  return true;
}

bool mitk::DataCollection::RemoveElement(std::string &name)
{
  if (!HasElement(name))
    return false;

  return RemoveIndex(NameToIndex(name));
}

void mitk::DataCollection::Clear()
{
  for (std::vector<itk::DataObject::Pointer>::iterator it = m_DataVector.begin(); it != m_DataVector.end(); ++it)
  {
    DataCollection* col = dynamic_cast<DataCollection*>((*it).GetPointer()) ;
    if (col != nullptr)
      col->Clear();
    else
      *it = nullptr;

  }
  m_Parent = nullptr;
}

/* Superclass methods, that need to be implemented */
void mitk::DataCollection::UpdateOutputInformation()
{

}

void mitk::DataCollection::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::DataCollection::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::DataCollection::VerifyRequestedRegion()
{
  return false;
}

void mitk::DataCollection::SetRequestedRegion(const itk::DataObject *)
{

}

mitk::DataNode::Pointer mitk::DataCollection::GetDataNode(size_t index)
{
  mitk::BaseData::Pointer data = dynamic_cast<mitk::BaseData*>(GetMitkImage(index).GetPointer());

  if (data.IsNull())
  {
    data = dynamic_cast<mitk::BaseData*>(GetData(index).GetPointer());
  }
  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetName(IndexToName(index));
  node->SetData(data);

  return node;
}

mitk::DataNode::Pointer mitk::DataCollection::GetDataNode(std::string name)
{
  return GetDataNode(NameToIndex(name));
}

mitk::Image::Pointer mitk::DataCollection::GetProbabilityMap(std::vector<std::string> probabilityNamesVector)
{
  typedef itk::Image<double, 3> ImageType;
  typedef itk::VectorImage<double, 3> VectorImageType;
  typedef itk::ComposeImageFilter<ImageType, VectorImageType> ComposeFilterType;

  ComposeFilterType::Pointer composer = ComposeFilterType::New();
  int i = 0;
  for (std::vector<std::string>::iterator it = probabilityNamesVector.begin(); it != probabilityNamesVector.end(); ++it)
  {
    ImageType* img = dynamic_cast<ImageType*>(this->GetData((*it)).GetPointer());
    if (img != nullptr)
    {
      composer->SetInput(i, img);
      ++i;
    }
  }
  composer->Update();
  VectorImageType::Pointer vecImage = composer->GetOutput();

  mitk::Image::Pointer probImage = mitk::Image::New();
  probImage->InitializeByItk(vecImage.GetPointer(), vecImage->GetNumberOfComponentsPerPixel());
  probImage->SetVolume(vecImage->GetBufferPointer());

  return probImage;
}
