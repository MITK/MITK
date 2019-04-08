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

#include "mitkDiffusionPropertyHelper.h"
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <itkImageRegionIterator.h>
#include <mitkProperties.h>
#include <vnl/algo/vnl_matrix_inverse.h>

#include <mitkCoreServices.h>
#include <mitkPropertyPersistenceInfo.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>

const std::string mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME = "DWMRI.GradientDirections";
const std::string mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME = "DWMRI.OriginalGradientDirections";
const std::string mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME = "DWMRI.MeasurementFrame";
const std::string mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME = "DWMRI.ReferenceBValue";
const std::string mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME = "DWMRI.BValueMap";
const std::string mitk::DiffusionPropertyHelper::MODALITY = "DWMRI.Modality";
const std::string mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS = "DWMRI.ApplyMatrixToGradients";
const std::string mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS = "DWMRI.ApplyMfToGradients";

mitk::DiffusionPropertyHelper::DiffusionPropertyHelper()
{
}

mitk::DiffusionPropertyHelper::~DiffusionPropertyHelper()
{
}


mitk::DiffusionPropertyHelper::ImageType::Pointer mitk::DiffusionPropertyHelper::GetItkVectorImage(mitk::Image* image)
{
  ImageType::Pointer vectorImage = ImageType::New();
  mitk::CastToItkImage(image, vectorImage);
  return vectorImage;
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer
mitk::DiffusionPropertyHelper::CalcAveragedDirectionSet(double precision, GradientDirectionsContainerType::Pointer directions)
{
  // save old and construct new direction container
  GradientDirectionsContainerType::Pointer newDirections = GradientDirectionsContainerType::New();

  // fill new direction container
  for(GradientDirectionsContainerType::ConstIterator gdcitOld = directions->Begin();
      gdcitOld != directions->End(); ++gdcitOld)
  {
    // already exists?
    bool found = false;
    for(GradientDirectionsContainerType::ConstIterator gdcitNew = newDirections->Begin();
        gdcitNew != newDirections->End(); ++gdcitNew)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        found = true;
        break;
      }
    }

    // if not found, add it to new container
    if(!found)
    {
      newDirections->push_back(gdcitOld.Value());
    }
  }

  return newDirections;
}

void mitk::DiffusionPropertyHelper::AverageRedundantGradients(mitk::Image* image, double precision)
{
  GradientDirectionsContainerType::Pointer oldDirs = GetOriginalGradientContainer(image);
  GradientDirectionsContainerType::Pointer newDirs = CalcAveragedDirectionSet(precision, oldDirs);

  // if sizes equal, we do not need to do anything in this function
  if(oldDirs->size() == newDirs->size())
    return;

  // new image
  ImageType::Pointer oldImage = ImageType::New();
  mitk::CastToItkImage( image, oldImage);
  ImageType::Pointer newITKImage = ImageType::New();
  newITKImage->SetSpacing( oldImage->GetSpacing() );   // Set the image spacing
  newITKImage->SetOrigin( oldImage->GetOrigin() );     // Set the image origin
  newITKImage->SetDirection( oldImage->GetDirection() );  // Set the image direction
  newITKImage->SetLargestPossibleRegion( oldImage->GetLargestPossibleRegion() );
  newITKImage->SetVectorLength( newDirs->size() );
  newITKImage->SetBufferedRegion( oldImage->GetLargestPossibleRegion() );
  newITKImage->Allocate();

  // average image data that corresponds to identical directions
  itk::ImageRegionIterator< ImageType > newIt(newITKImage, newITKImage->GetLargestPossibleRegion());
  newIt.GoToBegin();
  itk::ImageRegionIterator< ImageType > oldIt(oldImage, oldImage->GetLargestPossibleRegion());
  oldIt.GoToBegin();

  // initial new value of voxel
  ImageType::PixelType newVec;
  newVec.SetSize(newDirs->size());
  newVec.AllocateElements(newDirs->size());

  // find which gradients should be averaged
  GradientDirectionsContainerType::Pointer oldDirections = oldDirs;
  std::vector<std::vector<int> > dirIndices;
  for(GradientDirectionsContainerType::ConstIterator gdcitNew = newDirs->Begin();
      gdcitNew != newDirs->End(); ++gdcitNew)
  {
    dirIndices.push_back(std::vector<int>(0));
    for(GradientDirectionsContainerType::ConstIterator gdcitOld = oldDirs->Begin();
        gdcitOld != oldDirections->End(); ++gdcitOld)
    {
      if(AreAlike(gdcitNew.Value(), gdcitOld.Value(), precision))
      {
        //MITK_INFO << gdcitNew.Value() << "  " << gdcitOld.Value();
        dirIndices[gdcitNew.Index()].push_back(gdcitOld.Index());
      }
    }
  }

  //int ind1 = -1;
  while(!newIt.IsAtEnd())
  {
    // init new vector with zeros
    newVec.Fill(0.0);

    // the old voxel value with duplicates
    ImageType::PixelType oldVec = oldIt.Get();

    for(unsigned int i=0; i<dirIndices.size(); i++)
    {
      // do the averaging
      const unsigned int numavg = dirIndices[i].size();
      unsigned int sum = 0;
      for(unsigned int j=0; j<numavg; j++)
      {
        //MITK_INFO << newVec[i] << " << " << oldVec[dirIndices[i].at(j)];
        sum += oldVec[dirIndices[i].at(j)];
      }
      if(numavg == 0)
      {
        MITK_ERROR << "VectorImage: Error on averaging. Possibly due to corrupted data";
        return;
      }
      newVec[i] = sum / numavg;
    }

    newIt.Set(newVec);

    ++newIt;
    ++oldIt;
  }

  mitk::GrabItkImageMemory( newITKImage, image );

  SetOriginalGradientContainer(image, newDirs);
  InitializeImage(image);
  std::cout << std::endl;
}

void mitk::DiffusionPropertyHelper::ApplyMeasurementFrameAndRotationMatrix(mitk::Image* image)
{

  if(  image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer originalDirections = GetOriginalGradientContainer(image);

  MeasurementFrameType  measurementFrame = GetMeasurementFrame(image);

  mitk::Vector3D s = image->GetGeometry()->GetSpacing();
  mitk::VnlVector c0 = image->GetGeometry()->GetMatrixColumn(0)/s[0];
  mitk::VnlVector c1 = image->GetGeometry()->GetMatrixColumn(1)/s[1];
  mitk::VnlVector c2 = image->GetGeometry()->GetMatrixColumn(2)/s[2];
  MeasurementFrameType imageRotationMatrix;
  imageRotationMatrix[0][0] = c0[0];
  imageRotationMatrix[1][0] = c0[1];
  imageRotationMatrix[2][0] = c0[2];
  imageRotationMatrix[0][1] = c1[0];
  imageRotationMatrix[1][1] = c1[1];
  imageRotationMatrix[2][1] = c1[2];
  imageRotationMatrix[0][2] = c2[0];
  imageRotationMatrix[1][2] = c2[1];
  imageRotationMatrix[2][2] = c2[2];

  GradientDirectionsContainerType::Pointer directions = GradientDirectionsContainerType::New();

  if( originalDirections.IsNull() || ( originalDirections->size() == 0 ) )
  {
    // original direction container was not set
    return;
  }

  bool apply_matrix = true;
  image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS.c_str(), apply_matrix);
  bool apply_mf = true;
  image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS.c_str(), apply_mf);

  if (apply_matrix)
  {
    MITK_INFO << "Applying image rotation to diffusion-gradient directions:";
    std::cout << imageRotationMatrix << std::endl;
  }
  if (apply_mf)
  {
    MITK_INFO << "Applying measurement frame to diffusion-gradient directions:";
    std::cout << measurementFrame << std::endl;
  }

  int c = 0;
  for(GradientDirectionsContainerType::ConstIterator gdcit = originalDirections->Begin();
      gdcit != originalDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    if (apply_matrix)
      vec = vec.pre_multiply(measurementFrame);
    if (apply_mf)
      vec = vec.pre_multiply(imageRotationMatrix);
    directions->InsertElement(c, vec);
    c++;
  }

  SetGradientContainer(image, directions);
}

void mitk::DiffusionPropertyHelper::UnApplyMeasurementFrameAndRotationMatrix(mitk::Image* image)
{

  if(  image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer modifiedDirections = GetGradientContainer(image);

  MeasurementFrameType measurementFrame = GetMeasurementFrame(image);
  measurementFrame = vnl_matrix_inverse<double>(measurementFrame).pinverse();

  mitk::Vector3D s = image->GetGeometry()->GetSpacing();
  mitk::VnlVector c0 = image->GetGeometry()->GetMatrixColumn(0)/s[0];
  mitk::VnlVector c1 = image->GetGeometry()->GetMatrixColumn(1)/s[1];
  mitk::VnlVector c2 = image->GetGeometry()->GetMatrixColumn(2)/s[2];
  MeasurementFrameType imageRotationMatrix;
  imageRotationMatrix[0][0] = c0[0];
  imageRotationMatrix[1][0] = c0[1];
  imageRotationMatrix[2][0] = c0[2];
  imageRotationMatrix[0][1] = c1[0];
  imageRotationMatrix[1][1] = c1[1];
  imageRotationMatrix[2][1] = c1[2];
  imageRotationMatrix[0][2] = c2[0];
  imageRotationMatrix[1][2] = c2[1];
  imageRotationMatrix[2][2] = c2[2];
  imageRotationMatrix = vnl_matrix_inverse<double>(imageRotationMatrix).pinverse();

  GradientDirectionsContainerType::Pointer directions = GradientDirectionsContainerType::New();

  if( modifiedDirections.IsNull() || ( modifiedDirections->size() == 0 ) )
  {
    // original direction container was not set
    return;
  }

  bool apply_matrix = true;
  image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS.c_str(), apply_matrix);
  bool apply_mf = true;
  image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS.c_str(), apply_mf);

  if (apply_matrix)
  {
    MITK_INFO << "Reverting image rotation to diffusion-gradient directions:";
    std::cout << imageRotationMatrix << std::endl;
  }
  if (apply_mf)
  {
    MITK_INFO << "Reverting measurement frame to diffusion-gradient directions:";
    std::cout << measurementFrame << std::endl;
  }

  int c = 0;
  for(GradientDirectionsContainerType::ConstIterator gdcit = modifiedDirections->Begin();
      gdcit != modifiedDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    if (apply_matrix)
      vec = vec.pre_multiply(imageRotationMatrix);
    if (apply_mf)
      vec = vec.pre_multiply(measurementFrame);
    directions->InsertElement(c, vec);
    c++;
  }

  SetOriginalGradientContainer(image, directions);
}

void mitk::DiffusionPropertyHelper::SetApplyMatrixToGradients(mitk::Image* image, bool apply)
{
  image->GetPropertyList()->SetProperty( mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS.c_str(), mitk::BoolProperty::New(apply) );
}

void mitk::DiffusionPropertyHelper::SetApplyMfToGradients(mitk::Image* image, bool apply)
{
  image->GetPropertyList()->SetProperty( mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS.c_str(), mitk::BoolProperty::New(apply) );
}

void mitk::DiffusionPropertyHelper::UpdateBValueMap(mitk::Image* image)
{
  BValueMapType b_ValueMap;

  if(image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).IsNotNull())
    b_ValueMap = GetBValueMap(image);

  if(!b_ValueMap.empty())
    b_ValueMap.clear();

  if(GetGradientContainer(image).IsNotNull())
  {
    GradientDirectionsContainerType::Pointer directions = GetGradientContainer(image);

    for(auto gdcit = directions->Begin(); gdcit!=directions->End(); ++gdcit)
    {
      b_ValueMap[GetB_Value(image, gdcit.Index())].push_back(gdcit.Index());
    }
  }

  SetBValueMap(image, b_ValueMap);
}

bool mitk::DiffusionPropertyHelper::AreAlike(GradientDirectionType g1, GradientDirectionType g2, double precision)
{
  GradientDirectionType diff = g1 - g2;
  GradientDirectionType diff2 = g1 + g2;
  return diff.two_norm() < precision || diff2.two_norm() < precision;
}

float mitk::DiffusionPropertyHelper::GetB_Value(const mitk::Image* image, unsigned int i)
{
  GradientDirectionsContainerType::Pointer directions = GetGradientContainer(image);
  float b_value = GetReferenceBValue(image);

  if(i > directions->Size()-1)
    return -1;

  if(directions->ElementAt(i).one_norm() <= 0.0)
  {
    return 0;
  }
  else
  {
    double twonorm = directions->ElementAt(i).two_norm();
    double bval = b_value*twonorm*twonorm;

    if (bval<0)
      bval = ceil(bval - 0.5);
    else
      bval = floor(bval + 0.5);

    return bval;
  }
}

void mitk::DiffusionPropertyHelper::CopyProperties(mitk::Image* source, mitk::Image* target, bool ignore_original_gradients)
{
  mitk::PropertyList::Pointer props = source->GetPropertyList()->Clone();
  if (ignore_original_gradients)
    props->RemoveProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME);
  target->SetPropertyList(props);
}

void mitk::DiffusionPropertyHelper::InitializeImage(mitk::Image* image)
{
  if ( image->GetProperty(mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS.c_str()).IsNull() )
    image->SetProperty( mitk::DiffusionPropertyHelper::APPLY_MATRIX_TO_GRADIENTS.c_str(), mitk::BoolProperty::New(true) );

  if ( image->GetProperty(mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS.c_str()).IsNull() )
    image->SetProperty( mitk::DiffusionPropertyHelper::APPLY_MF_TO_GRADIENTS.c_str(), mitk::BoolProperty::New(true) );

  if(  image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    // we don't have the original gradient directions. Therefore use the modified directions and rotate them back.
    UnApplyMeasurementFrameAndRotationMatrix(image);
  }
  else
    ApplyMeasurementFrameAndRotationMatrix(image);
  UpdateBValueMap(image);

  // initialize missing properties
  mitk::MeasurementFrameProperty::Pointer mf = dynamic_cast<mitk::MeasurementFrameProperty *>( image->GetProperty(MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer());
  if( mf.IsNull() )
  {
    //no measurement frame present, identity is assumed
    MeasurementFrameType identity;
    identity.set_identity();
    image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( identity ));
  }
}

bool mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(const mitk::DataNode* node)
{
  if ( node==nullptr )
    return false;
  if ( node->GetData()==nullptr )
    return false;
  return IsDiffusionWeightedImage(dynamic_cast<mitk::Image *>(node->GetData()));
}


bool mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(const mitk::Image * image)
{
  bool isDiffusionWeightedImage( true );

  if( image == nullptr )
  {
    isDiffusionWeightedImage = false;
  }

  if( isDiffusionWeightedImage )
  {
    mitk::FloatProperty::Pointer referenceBValue = dynamic_cast<mitk::FloatProperty *>(image->GetProperty(REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer());

    if( referenceBValue.IsNull() )
    {
      isDiffusionWeightedImage = false;
    }

  }

  unsigned int gradientDirections( 0 );
  if( isDiffusionWeightedImage )
  {
    mitk::GradientDirectionsProperty::Pointer gradientDirectionsProperty = dynamic_cast<mitk::GradientDirectionsProperty *>(image->GetProperty(GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer());

    if( gradientDirectionsProperty.IsNull() )
    {
      isDiffusionWeightedImage = false;
    }
    else
    {
      gradientDirections = gradientDirectionsProperty->GetGradientDirectionsContainer()->size();
    }
  }

  if( isDiffusionWeightedImage )
  {
    unsigned int components = image->GetPixelType().GetNumberOfComponents();

    if( components != gradientDirections )
    {
      isDiffusionWeightedImage = false;
    }
  }

  return isDiffusionWeightedImage;
}

const mitk::DiffusionPropertyHelper::BValueMapType & mitk::DiffusionPropertyHelper::GetBValueMap(const mitk::Image *image)
{
  return dynamic_cast<mitk::BValueMapProperty *>(image->GetProperty(BVALUEMAPPROPERTYNAME.c_str()).GetPointer())->GetBValueMap();
}



std::vector< int > mitk::DiffusionPropertyHelper::GetBValueVector(const mitk::Image* image)
{
  auto gcon = mitk::DiffusionPropertyHelper::GetGradientContainer(image);
  float b_value = mitk::DiffusionPropertyHelper::GetReferenceBValue(image);
  std::vector< int > bvalues;

  for (unsigned int i=0; i<gcon->Size(); ++i)
  {
    double twonorm = gcon->ElementAt(i).two_norm();
    double bval = b_value*twonorm*twonorm;

    if (bval<0)
      bval = ceil(bval - 0.5);
    else
      bval = floor(bval + 0.5);

    bvalues.push_back(bval);
  }
  return bvalues;
}

float mitk::DiffusionPropertyHelper::GetReferenceBValue(const mitk::Image *image)
{
  return dynamic_cast<mitk::FloatProperty *>(image->GetProperty(REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer())->GetValue();
}

const mitk::DiffusionPropertyHelper::MeasurementFrameType & mitk::DiffusionPropertyHelper::GetMeasurementFrame(const mitk::Image *image)
{
  mitk::MeasurementFrameProperty::Pointer mf = dynamic_cast<mitk::MeasurementFrameProperty *>( image->GetProperty(MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer());
  if( mf.IsNull() )
  {
    //no measurement frame present, identity is assumed
    MeasurementFrameType identity;
    identity.set_identity();
    mf = mitk::MeasurementFrameProperty::New( identity );
  }

  return mf->GetMeasurementFrame();
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(const mitk::Image *image)
{
  return dynamic_cast<mitk::GradientDirectionsProperty *>(image->GetProperty(ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer mitk::DiffusionPropertyHelper::GetGradientContainer(const mitk::Image *image)
{
  return dynamic_cast<mitk::GradientDirectionsProperty *>(image->GetProperty(GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();
}

void mitk::DiffusionPropertyHelper::SetReferenceBValue(mitk::Image* image, float b_value)
{
  image->GetPropertyList()->ReplaceProperty(REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New(b_value));
}

void mitk::DiffusionPropertyHelper::SetBValueMap(mitk::Image* image, BValueMapType map)
{
  image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New(map));
}

void mitk::DiffusionPropertyHelper::SetOriginalGradientContainer(mitk::Image* image, GradientDirectionsContainerType::Pointer g_cont)
{
  image->GetPropertyList()->ReplaceProperty(ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New(g_cont));
}

void mitk::DiffusionPropertyHelper::SetGradientContainer(mitk::Image* image, GradientDirectionsContainerType::Pointer g_cont)
{
  image->GetPropertyList()->ReplaceProperty(GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New(g_cont));
}

void mitk::DiffusionPropertyHelper::SetMeasurementFrame(mitk::Image* image, MeasurementFrameType mf)
{
  image->GetPropertyList()->ReplaceProperty( MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( mf ) );
}

void mitk::DiffusionPropertyHelper::RotateGradients(mitk::Image* image, vnl_matrix_fixed<double, 3, 3> rotation_matrix, bool normalize_columns)
{
  if (normalize_columns)
    rotation_matrix = rotation_matrix.normalize_columns();

  int c = 0;
  auto new_gradients = GradientDirectionsContainerType::New();
  auto old_gradients = GetGradientContainer(image);
  for(auto gdcit = old_gradients->Begin(); gdcit != old_gradients->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    vec = vec.pre_multiply(rotation_matrix);
    new_gradients->InsertElement(c, vec);
    c++;
  }
  SetGradientContainer(image, new_gradients);
}

void mitk::DiffusionPropertyHelper::RotateOriginalGradients(mitk::Image* image, vnl_matrix_fixed<double, 3, 3> rotation_matrix, bool normalize_columns)
{
  if (normalize_columns)
    rotation_matrix = rotation_matrix.normalize_columns();

  int c = 0;
  auto new_gradients = GradientDirectionsContainerType::New();
  auto old_gradients = GetOriginalGradientContainer(image);
  for(auto gdcit = old_gradients->Begin(); gdcit != old_gradients->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    vec = vec.pre_multiply(rotation_matrix);
    new_gradients->InsertElement(c, vec);
    c++;
  }
  SetOriginalGradientContainer(image, new_gradients);
}

void mitk::DiffusionPropertyHelper::SetupProperties()
{
  //register relevant properties
  //non-persistent properties
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME, "This map stores which b values belong to which gradients.");
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME, "The original gradients used during acquisition. This property may be empty.");
  //persistent properties
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, "The reference b value the gradients are normalized to.");
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, "The measurment frame used during acquisition.");
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, "The gradients after applying measurement frame and image matrix.");
  mitk::CoreServices::GetPropertyDescriptions()->AddDescription(mitk::DiffusionPropertyHelper::MODALITY, "Defines the modality used for acquisition. DWMRI signifies diffusion weighted images.");

  mitk::PropertyPersistenceInfo::Pointer PPI_referenceBValue = mitk::PropertyPersistenceInfo::New();
  PPI_referenceBValue->SetNameAndKey(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME, "DWMRI_b-value");
  mitk::PropertyPersistenceInfo::Pointer PPI_measurementFrame = mitk::PropertyPersistenceInfo::New();
  PPI_measurementFrame->SetNameAndKey(mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME, "measurement frame");
  mitk::PropertyPersistenceInfo::Pointer PPI_gradientContainer = mitk::PropertyPersistenceInfo::New();
  PPI_gradientContainer->SetNameAndKey(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME, "DWMRI_gradient");
  mitk::PropertyPersistenceInfo::Pointer PPI_modality = mitk::PropertyPersistenceInfo::New();
  PPI_modality->SetNameAndKey(mitk::DiffusionPropertyHelper::MODALITY, "modality");

  mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_referenceBValue.GetPointer() , true);
  mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_measurementFrame.GetPointer(), true);
  mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_gradientContainer.GetPointer(), true);
  mitk::CoreServices::GetPropertyPersistence()->AddInfo(PPI_modality.GetPointer(), true);
}
