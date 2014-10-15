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


#include "mitkRTDoseReader.h"

#include <mitkIOUtil.h>
#include <mitkDataNodeFactory.h>
#include <mitkImagePixelWriteAccessor.h>
#include <mitkImageAccessByItk.h>
#include <itkImageIterator.h>
#include <itkImageRegionIterator.h>

#include <DataStructures/mitkRTConstants.h>

namespace mitk
{

  RTDoseReader::RTDoseReader(){}

  RTDoseReader::~RTDoseReader(){}

  mitk::DataNode::Pointer RTDoseReader::
      LoadRTDose(const char* filename)
  {
    DcmFileFormat fileformat;
    OFCondition outp = fileformat.loadFile(filename, EXS_Unknown);
    if(outp.bad())
    {
      MITK_ERROR << "Cant read the file" << std::endl;
    }
    DcmDataset *dataset = fileformat.getDataset();

    std::string name = filename;
    itk::FilenamesContainer file;
    file.push_back(name);

    mitk::DicomSeriesReader* reader = new mitk::DicomSeriesReader;

    mitk::DataNode::Pointer originalNode = reader->LoadDicomSeries(file,false);

    if(originalNode.IsNull())
    {
      MITK_ERROR << "Error reading the dcm file" << std::endl;
      return 0;
    }

    mitk::Image::Pointer originalImage
        = dynamic_cast<mitk::Image*>(originalNode->GetData());

    DRTDoseIOD doseObject;
    OFCondition result = doseObject.read(*dataset);

    if(result.bad())
    {
      MITK_ERROR << "Error reading the Dataset" << std::endl;
      return 0;
    }

    OFString gridScaling;
    Float32 gridscale;

    doseObject.getDoseGridScaling(gridScaling);
    gridscale = OFStandard::atof(gridScaling.c_str());

    AccessByItk_1(originalImage, MultiplayGridScaling, gridscale);

    double prescripeDose = this->GetMaxDoseValue(dataset);

    originalNode->SetName("RT Dose");
    originalNode->SetFloatProperty(mitk::Constants::PRESCRIBED_DOSE_PROPERTY_NAME.c_str(),prescripeDose);
    originalNode->SetFloatProperty(mitk::Constants::REFERENCE_DOSE_PROPERTY_NAME.c_str(), 40);
    originalNode->SetBoolProperty(mitk::Constants::DOSE_PROPERTY_NAME.c_str(),true);
    return originalNode;
  }

  template<typename TPixel, unsigned int VImageDimension>
  void RTDoseReader::MultiplayGridScaling(itk::Image<TPixel,
                                           VImageDimension>* image,
                                           Float32 gridscale)
  {
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    itk::ImageRegionIterator<InputImageType> it(image,
                                                image->GetRequestedRegion());
    for(it=it.Begin(); !it.IsAtEnd(); ++it)
    {
      it.Set(it.Get()*gridscale);
    }
  }

  double RTDoseReader::GetMaxDoseValue(DcmDataset* dataSet)
  {
    DRTDoseIOD doseObject;
    OFCondition result = doseObject.read(*dataSet);
    if(result.bad())
    {
      MITK_ERROR << "Error reading the RT Dose dataset" << std::endl;
      return 0;
    }

    Uint16 rows, columns, frames;
    OFString nrframes, gridScaling;
    const Uint16 *pixelData = NULL;
    Float32 gridscale;

    Uint16 &rows_ref = rows;
    Uint16 &columns_ref = columns;

    doseObject.getRows(rows_ref);
    doseObject.getColumns(columns_ref);
    doseObject.getNumberOfFrames(nrframes);
    doseObject.getDoseGridScaling(gridScaling);

    frames = atoi(nrframes.c_str());
    gridscale = OFStandard::atof(gridScaling.c_str());
    dataSet->findAndGetUint16Array(DCM_PixelData, pixelData, 0);

    int size = columns*rows*frames;
    double highest = 0;

    for(int i=0; i<size; ++i)
    {
      if((pixelData[i]*gridscale)>highest)
      {
        highest = pixelData[i] * gridscale;
      }
    }

    return highest;
  }

}
