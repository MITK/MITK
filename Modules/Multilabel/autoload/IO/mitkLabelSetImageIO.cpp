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

#include "mitkLabelSetImageIO.h"
#include "mitkBasePropertySerializer.h"
#include "mitkIOMimeTypes.h"
#include "mitkImageAccessByItk.h"
#include "mitkLabelSetIOHelper.h"
#include "mitkLabelSetImageConverter.h"
#include <mitkLocaleSwitch.h>

// itk
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

namespace mitk
{
  LabelSetImageIO::LabelSetImageIO()
    : AbstractFileIO(LabelSetImage::GetStaticNameOfClass(), IOMimeTypes::NRRD_MIMETYPE(), "MITK Multilabel Image")
  {
    AbstractFileWriter::SetRanking(10);
    AbstractFileReader::SetRanking(10);
    this->RegisterService();
  }

  IFileIO::ConfidenceLevel LabelSetImageIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;
    const auto *input = static_cast<const LabelSetImage *>(this->GetInput());
    if (input)
      return Supported;
    else
      return Unsupported;
  }

  void LabelSetImageIO::Write()
  {
    ValidateOutputLocation();

    auto input = dynamic_cast<const LabelSetImage *>(this->GetInput());

    mitk::LocaleSwitch localeSwitch("C");

    mitk::Image::Pointer inputVector = mitk::ConvertLabelSetImageToImage(input);

    // image write
    if (inputVector.IsNull())
    {
      mitkThrow() << "Cannot write non-image data";
    }

    itk::NrrdImageIO::Pointer nrrdImageIo = itk::NrrdImageIO::New();

    // Clone the image geometry, because we might have to change it
    // for writing purposes
    BaseGeometry::Pointer geometry = inputVector->GetGeometry()->Clone();

    // Check if geometry information will be lost
    if (inputVector->GetDimension() == 2 && !geometry->Is2DConvertable())
    {
      MITK_WARN << "Saving a 2D image with 3D geometry information. Geometry information will be lost! You might "
                   "consider using Convert2Dto3DImageFilter before saving.";

      // set matrix to identity
      mitk::AffineTransform3D::Pointer affTrans = mitk::AffineTransform3D::New();
      affTrans->SetIdentity();
      mitk::Vector3D spacing = geometry->GetSpacing();
      mitk::Point3D origin = geometry->GetOrigin();
      geometry->SetIndexToWorldTransform(affTrans);
      geometry->SetSpacing(spacing);
      geometry->SetOrigin(origin);
    }

    LocalFile localFile(this);
    const std::string path = localFile.GetFileName();

    MITK_INFO << "Writing image: " << path << std::endl;

    try
    {
      // Implementation of writer using itkImageIO directly. This skips the use
      // of templated itkImageFileWriter, which saves the multiplexing on MITK side.

      const unsigned int dimension = inputVector->GetDimension();
      const unsigned int *const dimensions = inputVector->GetDimensions();
      const mitk::PixelType pixelType = inputVector->GetPixelType();
      const mitk::Vector3D mitkSpacing = geometry->GetSpacing();
      const mitk::Point3D mitkOrigin = geometry->GetOrigin();

      // Due to templating in itk, we are forced to save a 4D spacing and 4D Origin,
      // though they are not supported in MITK
      itk::Vector<double, 4u> spacing4D;
      spacing4D[0] = mitkSpacing[0];
      spacing4D[1] = mitkSpacing[1];
      spacing4D[2] = mitkSpacing[2];
      spacing4D[3] = 1; // There is no support for a 4D spacing. However, we should have a valid value here

      itk::Vector<double, 4u> origin4D;
      origin4D[0] = mitkOrigin[0];
      origin4D[1] = mitkOrigin[1];
      origin4D[2] = mitkOrigin[2];
      origin4D[3] = 0; // There is no support for a 4D origin. However, we should have a valid value here

      // Set the necessary information for imageIO
      nrrdImageIo->SetNumberOfDimensions(dimension);
      nrrdImageIo->SetPixelType(pixelType.GetPixelType());
      nrrdImageIo->SetComponentType(pixelType.GetComponentType() < PixelComponentUserType ?
                                      static_cast<itk::ImageIOBase::IOComponentType>(pixelType.GetComponentType()) :
                                      itk::ImageIOBase::UNKNOWNCOMPONENTTYPE);
      nrrdImageIo->SetNumberOfComponents(pixelType.GetNumberOfComponents());

      itk::ImageIORegion ioRegion(dimension);

      for (unsigned int i = 0; i < dimension; i++)
      {
        nrrdImageIo->SetDimensions(i, dimensions[i]);
        nrrdImageIo->SetSpacing(i, spacing4D[i]);
        nrrdImageIo->SetOrigin(i, origin4D[i]);

        mitk::Vector3D mitkDirection;
        mitkDirection.SetVnlVector(geometry->GetIndexToWorldTransform()->GetMatrix().GetVnlMatrix().get_column(i));
        itk::Vector<double, 4u> direction4D;
        direction4D[0] = mitkDirection[0];
        direction4D[1] = mitkDirection[1];
        direction4D[2] = mitkDirection[2];

        // MITK only supports a 3x3 direction matrix. Due to templating in itk, however, we must
        // save a 4x4 matrix for 4D images. in this case, add an homogneous component to the matrix.
        if (i == 3)
        {
          direction4D[3] = 1; // homogenous component
        }
        else
        {
          direction4D[3] = 0;
        }
        vnl_vector<double> axisDirection(dimension);
        for (unsigned int j = 0; j < dimension; j++)
        {
          axisDirection[j] = direction4D[j] / spacing4D[i];
        }
        nrrdImageIo->SetDirection(i, axisDirection);

        ioRegion.SetSize(i, inputVector->GetLargestPossibleRegion().GetSize(i));
        ioRegion.SetIndex(i, inputVector->GetLargestPossibleRegion().GetIndex(i));
      }

      // use compression if available
      nrrdImageIo->UseCompressionOn();

      nrrdImageIo->SetIORegion(ioRegion);
      nrrdImageIo->SetFileName(path);

      // label set specific meta data
      char keybuffer[512];
      char valbuffer[512];

      sprintf(keybuffer, "modality");
      sprintf(valbuffer, "org.mitk.image.multilabel");
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(keybuffer), std::string(valbuffer));

      sprintf(keybuffer, "layers");
      sprintf(valbuffer, "%1d", input->GetNumberOfLayers());
      itk::EncapsulateMetaData<std::string>(
        nrrdImageIo->GetMetaDataDictionary(), std::string(keybuffer), std::string(valbuffer));

      for (unsigned int layerIdx = 0; layerIdx < input->GetNumberOfLayers(); layerIdx++)
      {
        sprintf(keybuffer, "layer_%03u", layerIdx);                    // layer idx
        sprintf(valbuffer, "%1u", input->GetNumberOfLabels(layerIdx)); // number of labels for the layer
        itk::EncapsulateMetaData<std::string>(
          nrrdImageIo->GetMetaDataDictionary(), std::string(keybuffer), std::string(valbuffer));

        auto iter = input->GetLabelSet(layerIdx)->IteratorConstBegin();
        unsigned int count(0);
        while (iter != input->GetLabelSet(layerIdx)->IteratorConstEnd())
        {
          std::unique_ptr<TiXmlDocument> document;
          document.reset(new TiXmlDocument());

          auto *decl = new TiXmlDeclaration("1.0", "", ""); // TODO what to write here? encoding? etc....
          document->LinkEndChild(decl);
          TiXmlElement *labelElem = mitk::LabelSetIOHelper::GetLabelAsTiXmlElement(iter->second);
          document->LinkEndChild(labelElem);
          TiXmlPrinter printer;
          printer.SetIndent("");
          printer.SetLineBreak("");

          document->Accept(&printer);

          sprintf(keybuffer, "org.mitk.label_%03u_%05u", layerIdx, count);
          itk::EncapsulateMetaData<std::string>(
            nrrdImageIo->GetMetaDataDictionary(), std::string(keybuffer), printer.Str());
          ++iter;
          ++count;
        }
      }
      // end label set specific meta data

      ImageReadAccessor imageAccess(inputVector);
      nrrdImageIo->Write(imageAccess.GetData());
    }
    catch (const std::exception &e)
    {
      mitkThrow() << e.what();
    }
    // end image write
  }

  IFileIO::ConfidenceLevel LabelSetImageIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;
    const std::string fileName = this->GetLocalFileName();
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(fileName);
    io->ReadImageInformation();

    itk::MetaDataDictionary imgMetaDataDictionary = io->GetMetaDataDictionary();
    std::string value("");
    itk::ExposeMetaData<std::string>(imgMetaDataDictionary, "modality", value);
    if (value.compare("org.mitk.image.multilabel") == 0)
    {
      return Supported;
    }
    else
      return Unsupported;
  }

  std::vector<BaseData::Pointer> LabelSetImageIO::Read()
  {
    mitk::LocaleSwitch localeSwitch("C");

    // begin regular image loading, adapted from mitkItkImageIO
    itk::NrrdImageIO::Pointer nrrdImageIO = itk::NrrdImageIO::New();
    Image::Pointer image = Image::New();

    const unsigned int MINDIM = 2;
    const unsigned int MAXDIM = 4;

    const std::string path = this->GetLocalFileName();

    MITK_INFO << "loading " << path << " via itk::ImageIOFactory... " << std::endl;

    // Check to see if we can read the file given the name or prefix
    if (path.empty())
    {
      mitkThrow() << "Empty filename in mitk::ItkImageIO ";
    }

    // Got to allocate space for the image. Determine the characteristics of
    // the image.
    nrrdImageIO->SetFileName(path);
    nrrdImageIO->ReadImageInformation();

    unsigned int ndim = nrrdImageIO->GetNumberOfDimensions();
    if (ndim < MINDIM || ndim > MAXDIM)
    {
      MITK_WARN << "Sorry, only dimensions 2, 3 and 4 are supported. The given file has " << ndim
                << " dimensions! Reading as 4D.";
      ndim = MAXDIM;
    }

    itk::ImageIORegion ioRegion(ndim);
    itk::ImageIORegion::SizeType ioSize = ioRegion.GetSize();
    itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();

    unsigned int dimensions[MAXDIM];
    dimensions[0] = 0;
    dimensions[1] = 0;
    dimensions[2] = 0;
    dimensions[3] = 0;

    ScalarType spacing[MAXDIM];
    spacing[0] = 1.0f;
    spacing[1] = 1.0f;
    spacing[2] = 1.0f;
    spacing[3] = 1.0f;

    Point3D origin;
    origin.Fill(0);

    unsigned int i;
    for (i = 0; i < ndim; ++i)
    {
      ioStart[i] = 0;
      ioSize[i] = nrrdImageIO->GetDimensions(i);
      if (i < MAXDIM)
      {
        dimensions[i] = nrrdImageIO->GetDimensions(i);
        spacing[i] = nrrdImageIO->GetSpacing(i);
        if (spacing[i] <= 0)
          spacing[i] = 1.0f;
      }
      if (i < 3)
      {
        origin[i] = nrrdImageIO->GetOrigin(i);
      }
    }

    ioRegion.SetSize(ioSize);
    ioRegion.SetIndex(ioStart);

    MITK_INFO << "ioRegion: " << ioRegion << std::endl;
    nrrdImageIO->SetIORegion(ioRegion);
    void *buffer = new unsigned char[nrrdImageIO->GetImageSizeInBytes()];
    nrrdImageIO->Read(buffer);

    image->Initialize(MakePixelType(nrrdImageIO), ndim, dimensions);
    image->SetImportChannel(buffer, 0, Image::ManageMemory);

    // access direction of itk::Image and include spacing
    mitk::Matrix3D matrix;
    matrix.SetIdentity();
    unsigned int j, itkDimMax3 = (ndim >= 3 ? 3 : ndim);
    for (i = 0; i < itkDimMax3; ++i)
      for (j = 0; j < itkDimMax3; ++j)
        matrix[i][j] = nrrdImageIO->GetDirection(j)[i];

    // re-initialize PlaneGeometry with origin and direction
    PlaneGeometry *planeGeometry = image->GetSlicedGeometry(0)->GetPlaneGeometry(0);
    planeGeometry->SetOrigin(origin);
    planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

    // re-initialize SlicedGeometry3D
    SlicedGeometry3D *slicedGeometry = image->GetSlicedGeometry(0);
    slicedGeometry->InitializeEvenlySpaced(planeGeometry, image->GetDimension(2));
    slicedGeometry->SetSpacing(spacing);

    MITK_INFO << slicedGeometry->GetCornerPoint(false, false, false);
    MITK_INFO << slicedGeometry->GetCornerPoint(true, true, true);

    // re-initialize TimeGeometry
    ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
    timeGeometry->Initialize(slicedGeometry, image->GetDimension(3));
    image->SetTimeGeometry(timeGeometry);

    buffer = nullptr;
    MITK_INFO << "number of image components: " << image->GetPixelType().GetNumberOfComponents() << std::endl;

    const itk::MetaDataDictionary &dictionary = nrrdImageIO->GetMetaDataDictionary();
    for (auto iter = dictionary.Begin(), iterEnd = dictionary.End(); iter != iterEnd;
         ++iter)
    {
      std::string key = std::string("meta.") + iter->first;
      if (iter->second->GetMetaDataObjectTypeInfo() == typeid(std::string))
      {
        std::string value =
          dynamic_cast<itk::MetaDataObject<std::string> *>(iter->second.GetPointer())->GetMetaDataObjectValue();
        image->SetProperty(key.c_str(), mitk::StringProperty::New(value));
      }
    }

    // end regular image loading

    LabelSetImage::Pointer output = ConvertImageToLabelSetImage(image);

    // get labels and add them as properties to the image
    char keybuffer[256];

    unsigned int numberOfLayers = GetIntByKey(dictionary, "layers");
    std::string _xmlStr;
    mitk::Label::Pointer label;

    for (unsigned int layerIdx = 0; layerIdx < numberOfLayers; layerIdx++)
    {
      sprintf(keybuffer, "layer_%03u", layerIdx);
      int numberOfLabels = GetIntByKey(dictionary, keybuffer);

      mitk::LabelSet::Pointer labelSet = mitk::LabelSet::New();

      for (int labelIdx = 0; labelIdx < numberOfLabels; labelIdx++)
      {
        TiXmlDocument doc;
        sprintf(keybuffer, "label_%03u_%05d", layerIdx, labelIdx);
        _xmlStr = GetStringByKey(dictionary, keybuffer);
        doc.Parse(_xmlStr.c_str());

        TiXmlElement *labelElem = doc.FirstChildElement("Label");
        if (labelElem == nullptr)
          mitkThrow() << "Error parsing NRRD header for mitk::LabelSetImage IO";

        label = mitk::LabelSetIOHelper::LoadLabelFromTiXmlDocument(labelElem);

        if (label->GetValue() == 0) // set exterior label is needed to hold exterior information
          output->SetExteriorLabel(label);
        labelSet->AddLabel(label);
        labelSet->SetLayer(layerIdx);
      }
      output->AddLabelSetToLayer(layerIdx, labelSet);
    }

    MITK_INFO << "...finished!" << std::endl;

    std::vector<BaseData::Pointer> result;
    result.push_back(output.GetPointer());
    return result;
  }

  int LabelSetImageIO::GetIntByKey(const itk::MetaDataDictionary &dic, const std::string &str)
  {
    std::vector<std::string> imgMetaKeys = dic.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString("");
    for (; itKey != imgMetaKeys.end(); itKey++)
    {
      itk::ExposeMetaData<std::string>(dic, *itKey, metaString);
      if (itKey->find(str.c_str()) != std::string::npos)
      {
        return atoi(metaString.c_str());
      }
    }
    return 0;
  }

  std::string LabelSetImageIO::GetStringByKey(const itk::MetaDataDictionary &dic, const std::string &str)
  {
    std::vector<std::string> imgMetaKeys = dic.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString("");
    for (; itKey != imgMetaKeys.end(); itKey++)
    {
      itk::ExposeMetaData<std::string>(dic, *itKey, metaString);
      if (itKey->find(str.c_str()) != std::string::npos)
      {
        return metaString;
      }
    }
    return metaString;
  }

  LabelSetImageIO *LabelSetImageIO::IOClone() const { return new LabelSetImageIO(*this); }
} // namespace

#endif //__mitkLabelSetImageWriter__cpp
