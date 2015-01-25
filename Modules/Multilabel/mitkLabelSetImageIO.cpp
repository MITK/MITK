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

#ifndef __mitkLabelSetImageWriter__cpp
#define __mitkLabelSetImageWriter__cpp

#include "mitkBasePropertySerializer.h"
#include "mitkIOMimeTypes.h"
#include "mitkLabelSetImageIO.h"

// itk
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

#include "tinyxml.h"

namespace mitk {

LabelSetImageIO::LabelSetImageIO()
  : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(), IOMimeTypes::NRRD_MIMETYPE(), "MITK Multilabel Image")
{
  AbstractFileWriter::SetRanking(10);
  AbstractFileReader::SetRanking(10);
  this->RegisterService();
}

IFileIO::ConfidenceLevel LabelSetImageIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported) return Unsupported;
  const LabelSetImage* input = static_cast<const LabelSetImage*>(this->GetInput());
  if (input)
    return Supported;
  else
    return Unsupported;
}

void LabelSetImageIO::Write()
{
  ValidateOutputLocation();

  const LabelSetImage* input = static_cast<const LabelSetImage*>(this->GetInput());

  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      mitkThrow() << "Could not set locale " << currLocale;
    }
  }

  typedef itk::ImageFileWriter<VectorImageType> WriterType;

  VectorImageType::Pointer vectorImage = input->GetVectorImage(true); // force update

  char keybuffer[512];
  char valbuffer[512];

  sprintf( keybuffer, "modality");
  sprintf( valbuffer, "org.mitk.image.multilabel");
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

  sprintf( keybuffer, "layers");
  sprintf( valbuffer, "%1d", input->GetNumberOfLayers());
  itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

  for (unsigned int layerIdx=0; layerIdx<input->GetNumberOfLayers(); layerIdx++)
  {
    sprintf( keybuffer, "layer_%03d", layerIdx ); // layer idx
    sprintf( valbuffer, "%1d", input->GetNumberOfLabels(layerIdx)); // number of labels for the layer
    itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

    mitk::LabelSet::LabelContainerConstIteratorType iter = input->GetLabelSet(layerIdx)->IteratorConstBegin();
    unsigned int count(0);
    while (iter != input->GetLabelSet(layerIdx)->IteratorConstEnd())
    {
      std::auto_ptr<TiXmlDocument> document;
      document.reset(new TiXmlDocument());

      TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
      document->LinkEndChild( decl );
      TiXmlElement * labelElem = GetLabelAsTiXmlElement(iter->second);
      document->LinkEndChild( labelElem );
      TiXmlPrinter printer;
      printer.SetIndent("");
      printer.SetLineBreak("");

      document->Accept(&printer);

      sprintf( keybuffer, "org.mitk.label_%03u_%05u", layerIdx, count );
      itk::EncapsulateMetaData<std::string>(vectorImage->GetMetaDataDictionary(),std::string(keybuffer), printer.Str());
      ++iter;
      ++count;
    }
  }

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();

  WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput(vectorImage);

  LocalFile localFile(this);
  nrrdWriter->SetFileName(localFile.GetFileName().c_str());
  nrrdWriter->UseCompressionOn();
  nrrdWriter->SetImageIO(io);

  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    MITK_ERROR << "Could not write segmentation session. See error log for details.";
    mitkThrow() << e.GetDescription();
  }

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    mitkThrow() << "Could not reset locale " << currLocale;
  }
}

IFileIO::ConfidenceLevel LabelSetImageIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  const std::string fileName = this->GetLocalFileName();
  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileName(fileName);
  io->ReadImageInformation();

  itk::MetaDataDictionary imgMetaDataDictionary = io->GetMetaDataDictionary();
  std::string value ("");
  itk::ExposeMetaData<std::string> (imgMetaDataDictionary, "modality", value);
  if (value.compare("org.mitk.image.multilabel") == 0)
  {
    return Supported;
  }
  else
    return Unsupported;
}

std::vector<BaseData::Pointer> LabelSetImageIO::Read()
{
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );

  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      mitkThrow() << "Could not set locale.";
    }
  }

  LabelSetImage::VectorImageType::Pointer vectorImage;

  typedef itk::ImageFileReader<VectorImageType> FileReaderType;
  FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName(this->GetLocalFileName());
  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  reader->SetImageIO(io);
  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  catch(...)
  {
    mitkThrow() << "Unknown exception caught!";
  }

  vectorImage = reader->GetOutput();

  if (vectorImage.IsNull())
    mitkThrow() << "Could not retrieve the vector image.";

  LabelSetImage::Pointer output = LabelSetImage::New();

  ImageType::Pointer auxImg = ImageType::New();
  auxImg->SetSpacing( vectorImage->GetSpacing() );
  auxImg->SetOrigin( vectorImage->GetOrigin() );
  auxImg->SetDirection( vectorImage->GetDirection() );
  auxImg->SetRegions( vectorImage->GetLargestPossibleRegion() );
  auxImg->Allocate();

  // initialize output image based on vector image meta information
  output->InitializeByItk<ImageType>( auxImg.GetPointer() );

  itk::MetaDataDictionary imgMetaDictionary = vectorImage->GetMetaDataDictionary();

  char keybuffer[256];

  int numberOfLayers = GetIntByKey(imgMetaDictionary,"layers");
  std::string _xmlStr;
  mitk::Label::Pointer label;

  for( int layerIdx=0; layerIdx < numberOfLayers; layerIdx++)
  {
    sprintf( keybuffer, "layer_%03d", layerIdx );
    int numberOfLabels = GetIntByKey(imgMetaDictionary,keybuffer);

    mitk::LabelSet::Pointer labelSet = mitk::LabelSet::New();

    for(int labelIdx=0; labelIdx < numberOfLabels; labelIdx++)
    {
      TiXmlDocument doc;
      sprintf( keybuffer, "label_%03d_%05d", layerIdx, labelIdx );
      _xmlStr = GetStringByKey(imgMetaDictionary,keybuffer);
      doc.Parse(_xmlStr.c_str());

      TiXmlElement * labelElem = doc.FirstChildElement("Label");
      if (labelElem == 0)
        mitkThrow() << "Error parsing NRRD header for mitk::LabelSetImage IO";

      label = LoadLabelFromTiXmlDocument(labelElem);

      if(label->GetValue() == 0) // set exterior label is needed to hold exterior information
        output->SetExteriorLabel(label);
      labelSet->AddLabel(label);
    }
    output->AddLayer(labelSet);
  }

  // set vector image
  output->SetVectorImage(vectorImage);

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    mitkThrow() << "Could not reset locale!";
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(output.GetPointer());
  return result;
}

int LabelSetImageIO::GetIntByKey(const itk::MetaDataDictionary & dic,const std::string & str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    itk::ExposeMetaData<std::string> (dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return atoi(metaString.c_str());
    }
  }
  return 0;
}

std::string LabelSetImageIO::GetStringByKey(const itk::MetaDataDictionary & dic,const std::string & str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    itk::ExposeMetaData<std::string> (dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return metaString;
    }
  }
  return metaString;
}

Label::Pointer LabelSetImageIO::LoadLabelFromTiXmlDocument(TiXmlElement * labelElem)
{
  // reread
  TiXmlElement * propElem = labelElem->FirstChildElement("property");

  std::string name;
  mitk::BaseProperty::Pointer prop;

  mitk::Label::Pointer label = mitk::Label::New();
  while(propElem)
  {
    LabelSetImageIO::PropertyFromXmlElem( name, prop, propElem );
    label->SetProperty( name, prop );
    propElem = propElem->NextSiblingElement( "property" );
  }

  return label.GetPointer();
}

bool LabelSetImageIO::PropertyFromXmlElem(std::string& key, mitk::BaseProperty::Pointer& prop, TiXmlElement* elem)
{
  std::string type;
  elem->QueryStringAttribute("type", &type);
  elem->QueryStringAttribute("key", &key);

  // construct name of serializer class
  std::string serializername(type);
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << type << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple deserializers found for " << type << "Using arbitrarily the first one.";

  for ( std::list<itk::LightObject::Pointer>::iterator iter = allSerializers.begin();
        iter != allSerializers.end();
        ++iter )
  {
    if (BasePropertySerializer* serializer = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
    {
      try
      {
        prop = serializer->Deserialize(elem->FirstChildElement());
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Deserializer " << serializer->GetNameOfClass() << " failed: " << e.what();
        return false;
      }
      break;
    }
  }
  if(prop.IsNull())
    return false;
  return true;
}

void LabelSetImageIO::LoadLabelSetImagePreset(const std::string & presetFilename, mitk::LabelSetImage::Pointer& inputImage )
{
  std::unique_ptr<TiXmlDocument> presetXmlDoc(new TiXmlDocument());

  bool ok = presetXmlDoc->LoadFile(presetFilename);
  if ( !ok )
    return;

  TiXmlElement * presetElem = presetXmlDoc->FirstChildElement("LabelSetImagePreset");
  if(!presetElem)
  {
    MITK_INFO << "No valid preset XML";
    return;
  }

  int numberOfLayers;
  presetElem->QueryIntAttribute("layers", &numberOfLayers);

  for(int i = 0 ; i < numberOfLayers; i++)
  {
    TiXmlElement * layerElem = presetElem->FirstChildElement("Layer");
    int numberOfLabels;
    layerElem->QueryIntAttribute("labels", &numberOfLabels);

    if(inputImage->GetLabelSet(i) == NULL) inputImage->AddLayer();

    TiXmlElement * labelElement = layerElem->FirstChildElement("Label");
    if(labelElement == NULL) break;
    for(int j = 0 ; j < numberOfLabels; j++)
    {

      mitk::Label::Pointer label = mitk::LabelSetImageIO::LoadLabelFromTiXmlDocument(labelElement);
      inputImage->GetLabelSet()->AddLabel(label);

      labelElement = labelElement->NextSiblingElement("Label");
      if(labelElement == NULL) break;
    }
  }
}

LabelSetImageIO* LabelSetImageIO::IOClone() const
{
  return new LabelSetImageIO(*this);
}

TiXmlElement * LabelSetImageIO::GetLabelAsTiXmlElement(Label * label)
{

  TiXmlElement* labelElem = new TiXmlElement("Label");

  // add XML contents
  const PropertyList::PropertyMap* propmap = label->GetMap();
  for ( PropertyList::PropertyMap::const_iterator iter = propmap->begin();
        iter != propmap->end();
        ++iter )
  {
    std::string key = iter->first;
    const BaseProperty* property = iter->second;
    TiXmlElement* element = PropertyToXmlElem( key, property );
    if (element)
     labelElem->LinkEndChild( element );

  }
  return labelElem;
}

TiXmlElement* LabelSetImageIO::PropertyToXmlElem( const std::string& key, const BaseProperty* property )
{
  TiXmlElement* keyelement = new TiXmlElement("property");
  keyelement->SetAttribute("key", key);
  keyelement->SetAttribute("type", property->GetNameOfClass());

  // construct name of serializer class
  std::string serializername(property->GetNameOfClass());
  serializername += "Serializer";

  std::list<itk::LightObject::Pointer> allSerializers = itk::ObjectFactoryBase::CreateAllInstance(serializername.c_str());
  if (allSerializers.size() < 1)
    MITK_ERROR << "No serializer found for " << property->GetNameOfClass() << ". Skipping object";

  if (allSerializers.size() > 1)
    MITK_WARN << "Multiple serializers found for " << property->GetNameOfClass() << "Using arbitrarily the first one.";

  for ( std::list<itk::LightObject::Pointer>::iterator iter = allSerializers.begin();
        iter != allSerializers.end();
        ++iter )
  {
    if (BasePropertySerializer* serializer = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
    {
      serializer->SetProperty(property);
      try
      {
        TiXmlElement* valueelement = serializer->Serialize();
        if (valueelement)
          keyelement->LinkEndChild( valueelement );
      }
      catch (std::exception& e)
      {
        MITK_ERROR << "Serializer " << serializer->GetNameOfClass() << " failed: " << e.what();
      }
      break;
    }
  }
  return keyelement;
}

bool LabelSetImageIO::SaveLabelSetImagePreset(const std::string & presetFilename, LabelSetImage::Pointer & inputImage)
{
  TiXmlDocument * presetXmlDoc =  new TiXmlDocument();

  TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
  presetXmlDoc->LinkEndChild( decl );

  TiXmlElement * presetElement = new TiXmlElement("LabelSetImagePreset");
  presetElement->SetAttribute("layers", inputImage->GetNumberOfLayers());

  presetXmlDoc->LinkEndChild(presetElement);

  for (unsigned int layerIdx=0; layerIdx<inputImage->GetNumberOfLayers(); layerIdx++)
  {
    TiXmlElement * layerElement = new TiXmlElement("Layer");
    layerElement->SetAttribute("index", layerIdx);
    layerElement->SetAttribute("labels", inputImage->GetNumberOfLabels(layerIdx));

    presetElement->LinkEndChild(layerElement);

    for (unsigned int labelIdx=0; labelIdx<inputImage->GetNumberOfLabels(layerIdx); labelIdx++)
    {
      TiXmlElement * labelAsXml = LabelSetImageIO::GetLabelAsTiXmlElement(inputImage->GetLabel(labelIdx,layerIdx));
      layerElement->LinkEndChild(labelAsXml);
    }
  }

  bool wasSaved = presetXmlDoc->SaveFile(presetFilename);
  delete presetXmlDoc;

  return wasSaved;
}

} //namespace




#endif //__mitkLabelSetImageWriter__cpp
