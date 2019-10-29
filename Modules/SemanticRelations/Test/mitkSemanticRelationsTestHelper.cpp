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

#include "mitkSemanticRelationsTestHelper.h"

// mitk semantic relations
#include "mitkDICOMHelper.h"
#include "mitkRelationStorage.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkImage.h>
#include <mitkIPersistenceService.h>
#include <mitkTemporoSpatialStringProperty.h>

// mitk multilabel
#include <mitkLabelSetImage.h>

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientOneCTImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient1"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20190101"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientOneMRImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient1"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20190101"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("MR"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientOneOtherCTImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient1"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20190131"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientTwoPETImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient2"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20180101"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("PT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientTwoSegmentation()
{
  // create new empty segmentation
  LabelSetImage::Pointer segmentation = LabelSetImage::New();

  // set properties of segmentation (base data)
  segmentation->SetProperty(GetCaseIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("Patient2"));
  segmentation->SetProperty(GetNodeIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New(UIDGeneratorBoost::GenerateUID()));
  segmentation->SetProperty(GetDateDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("20180101"));
  // segmentation modality is 'SEG'

  // create new data node and add segmentation as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(segmentation);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientThreeCTImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient3"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20190201"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientThreeCTSegmentation()
{
  // create new empty segmentation
  LabelSetImage::Pointer segmentation = LabelSetImage::New();

  // set properties of segmentation (base data)
  segmentation->SetProperty(GetCaseIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("Patient3"));
  segmentation->SetProperty(GetNodeIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New(UIDGeneratorBoost::GenerateUID()));
  segmentation->SetProperty(GetDateDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("20190201"));
  // segmentation modality is 'SEG'

  // create new data node and add segmentation as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(segmentation);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientThreeMRImage()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient3"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20190215"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("MR"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetPatientThreeMRSegmentation()
{
  // create new empty segmentation
  LabelSetImage::Pointer segmentation = LabelSetImage::New();

  // set properties of segmentation (base data)
  segmentation->SetProperty(GetCaseIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("Patient3"));
  segmentation->SetProperty(GetNodeIDDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New(UIDGeneratorBoost::GenerateUID()));
  segmentation->SetProperty(GetDateDICOMProperty().c_str(),
    TemporoSpatialStringProperty::New("20190215"));
  // segmentation modality is 'SEG'

  // create new data node and add segmentation as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(segmentation);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetInvalidDate()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient4"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));

  // no DICOM information for Date

  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetInvalidModality()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient5"));
  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20180101"));

  // no DICOM information for Modality

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetInvalidID()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  image->SetProperty(GetCaseIDDICOMProperty().c_str(),
    StringProperty::New("Patient6"));

  // no DICOM information for NodeID

  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20180101"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

mitk::DataNode::Pointer mitk::SemanticRelationsTestHelper::GetInvalidCaseID()
{
  // create new empty image
  Image::Pointer image = Image::New();

  // set properties of image (base data)
  // no DICOM information for CaseID

  image->SetProperty(GetNodeIDDICOMProperty().c_str(),
    StringProperty::New(UIDGeneratorBoost::GenerateUID()));
  image->SetProperty(GetDateDICOMProperty().c_str(),
    StringProperty::New("20180101"));
  image->SetProperty(GetModalityDICOMProperty().c_str(),
    StringProperty::New("CT"));

  // create new data node and add image as base data
  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetData(image);

  return dataNode;
}

void mitk::SemanticRelationsTestHelper::ClearRelationStorage()
{
  // access the storage
  PERSISTENCE_GET_SERVICE_MACRO
    if (nullptr == persistenceService)
    {
      MITK_DEBUG << "Persistence service could not be loaded";
      return;
    }

  auto allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
  for (auto& caseID : allCaseIDs)
  {
    persistenceService->RemovePropertyList(caseID);
  }

  std::string listIdentifier("caseIDs");
  persistenceService->RemovePropertyList(listIdentifier);
}
