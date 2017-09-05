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

const std::string mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME = "DWMRI.GradientDirections";
const std::string mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME = "DWMRI.OriginalGradientDirections";
const std::string mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME = "DWMRI.MeasurementFrame";
const std::string mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME = "DWMRI.ReferenceBValue";
const std::string mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME = "DWMRI.BValueMap";
const std::string mitk::DiffusionPropertyHelper::MODALITY = "DWMRI.Modality";
const std::string mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS = "DWMRI.KeepOriginalDirections";

mitk::DiffusionPropertyHelper::DiffusionPropertyHelper()
{
}

mitk::DiffusionPropertyHelper::DiffusionPropertyHelper( mitk::Image* inputImage)
  : m_Image( inputImage )
{
  // Update props
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

void mitk::DiffusionPropertyHelper::AverageRedundantGradients(double precision)
{

  mitk::GradientDirectionsProperty* DirectionsProperty = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() );
  GradientDirectionsContainerType::Pointer oldDirs = DirectionsProperty->GetGradientDirectionsContainer();

  GradientDirectionsContainerType::Pointer newDirs =
      CalcAveragedDirectionSet(precision, oldDirs);

  // if sizes equal, we do not need to do anything in this function
  if(oldDirs->size() == newDirs->size())
    return;

  // new image
  ImageType::Pointer oldImage = ImageType::New();
  mitk::CastToItkImage( m_Image, oldImage);
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

    // progress
    //typename ImageType::IndexType ind = newIt.GetIndex();
    //ind1 = ind.m_Index[2];

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

  mitk::GrabItkImageMemory( newITKImage, m_Image );

  m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newDirs ) );
  m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newDirs ) );
  ApplyMeasurementFrameAndRotationMatrix();
  UpdateBValueMap();
  std::cout << std::endl;
}

void mitk::DiffusionPropertyHelper::ApplyMeasurementFrameAndRotationMatrix()
{

  if(  m_Image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer  originalDirections = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();

  MeasurementFrameType  measurementFrame = GetMeasurementFrame(m_Image);

  mitk::Vector3D s = m_Image->GetGeometry()->GetSpacing();
  mitk::VnlVector c0 = m_Image->GetGeometry()->GetMatrixColumn(0)/s[0];
  mitk::VnlVector c1 = m_Image->GetGeometry()->GetMatrixColumn(1)/s[1];
  mitk::VnlVector c2 = m_Image->GetGeometry()->GetMatrixColumn(2)/s[2];
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

  bool keep_originals = false;
  m_Image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS.c_str(), keep_originals);

  if (!keep_originals)
  {
    MITK_INFO << "Applying measurement frame to diffusion-gradient directions:";
    std::cout << measurementFrame << std::endl;
    MITK_INFO << "Applying image rotation to diffusion-gradient directions:";
    std::cout << imageRotationMatrix << std::endl;
  }

  int c = 0;
  for(GradientDirectionsContainerType::ConstIterator gdcit = originalDirections->Begin();
      gdcit != originalDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    if (!keep_originals)
    {
      vec = vec.pre_multiply(measurementFrame);
      vec = vec.pre_multiply(imageRotationMatrix);
    }
    directions->InsertElement(c, vec);
    c++;
  }

  m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( directions ) );
}

void mitk::DiffusionPropertyHelper::UnApplyMeasurementFrameAndRotationMatrix()
{

  if(  m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer  modifiedDirections = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();

  MeasurementFrameType  measurementFrame = GetMeasurementFrame(m_Image);
  measurementFrame = vnl_matrix_inverse<double>(measurementFrame).pinverse();

  mitk::Vector3D s = m_Image->GetGeometry()->GetSpacing();
  mitk::VnlVector c0 = m_Image->GetGeometry()->GetMatrixColumn(0)/s[0];
  mitk::VnlVector c1 = m_Image->GetGeometry()->GetMatrixColumn(1)/s[1];
  mitk::VnlVector c2 = m_Image->GetGeometry()->GetMatrixColumn(2)/s[2];
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

  bool keep_originals = false;
  m_Image->GetPropertyList()->GetBoolProperty(mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS.c_str(), keep_originals);

  if (!keep_originals)
  {
    MITK_INFO << "Reverting image rotation to diffusion-gradient directions:";
    std::cout << imageRotationMatrix << std::endl;
    MITK_INFO << "Reverting measurement frame to diffusion-gradient directions:";
    std::cout << measurementFrame << std::endl;
  }

  int c = 0;
  for(GradientDirectionsContainerType::ConstIterator gdcit = modifiedDirections->Begin();
      gdcit != modifiedDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    if (!keep_originals)
    {
      vec = vec.pre_multiply(imageRotationMatrix);
      vec = vec.pre_multiply(measurementFrame);
    }
    directions->InsertElement(c, vec);
    c++;
  }

  m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( directions ) );
}


void mitk::DiffusionPropertyHelper::ClearMeasurementFrameAndRotationMatrixFromGradients(mitk::Image* image)
{

  if(  image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer  originalDirections = static_cast<mitk::GradientDirectionsProperty*>( image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();

  GradientDirectionsContainerType::Pointer directions = GradientDirectionsContainerType::New();

  if( originalDirections.IsNull() || ( originalDirections->size() == 0 ) )
  {
    // original direction container was not set
    return;
  }

  int c = 0;
  for(GradientDirectionsContainerType::ConstIterator gdcit = originalDirections->Begin();
      gdcit != originalDirections->End(); ++gdcit)
  {
    vnl_vector<double> vec = gdcit.Value();
    directions->InsertElement(c, vec);
    c++;
  }

  image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS.c_str(), mitk::BoolProperty::New(true) );
  image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( directions ) );
}


void mitk::DiffusionPropertyHelper::UpdateBValueMap()
{
  BValueMapType b_ValueMap;

  if(m_Image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).IsNull())
  {
  }
  else
  {
    b_ValueMap = static_cast<mitk::BValueMapProperty*>(m_Image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )->GetBValueMap();
  }

  if(!b_ValueMap.empty())
  {
    b_ValueMap.clear();
  }

  if( m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).IsNotNull() )
  {
    GradientDirectionsContainerType::Pointer directions = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

    GradientDirectionsContainerType::ConstIterator gdcit;
    for( gdcit = directions->Begin(); gdcit != directions->End(); ++gdcit)
    {
      b_ValueMap[GetB_Value(gdcit.Index())].push_back(gdcit.Index());
    }
  }

  m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New( b_ValueMap ) );
}

bool mitk::DiffusionPropertyHelper::AreAlike(GradientDirectionType g1,
                                             GradientDirectionType g2,
                                             double precision)
{
  GradientDirectionType diff = g1 - g2;
  GradientDirectionType diff2 = g1 + g2;
  return diff.two_norm() < precision || diff2.two_norm() < precision;
}

float mitk::DiffusionPropertyHelper::GetB_Value(unsigned int i)
{
  GradientDirectionsContainerType::Pointer directions = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )->GetGradientDirectionsContainer();

  float b_value = static_cast<mitk::FloatProperty*>(m_Image->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )->GetValue();

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

void mitk::DiffusionPropertyHelper::InitializeImage()
{
  if ( m_Image->GetProperty(mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS.c_str()).IsNull() )
    m_Image->SetProperty( mitk::DiffusionPropertyHelper::KEEP_ORIGINAL_DIRECTIONS.c_str(), mitk::BoolProperty::New(false) );

  if(  m_Image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    // we don't have the original gradient directions. Therefore use the modified directions and roatate them back.
    this->UnApplyMeasurementFrameAndRotationMatrix();
  }
  else
    this->ApplyMeasurementFrameAndRotationMatrix();
  this->UpdateBValueMap();

  // initialize missing properties
  mitk::MeasurementFrameProperty::Pointer mf = dynamic_cast<mitk::MeasurementFrameProperty *>( m_Image->GetProperty(MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer());
  if( mf.IsNull() )
  {
    //no measurement frame present, identity is assumed
    MeasurementFrameType identity;
    identity.set_identity();
    m_Image->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( identity ));
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

bool mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage() const
{
  return IsDiffusionWeightedImage(m_Image);
}

const mitk::DiffusionPropertyHelper::BValueMapType &mitk::DiffusionPropertyHelper::GetBValueMap() const
{
  return GetBValueMap(m_Image);
}

float mitk::DiffusionPropertyHelper::GetReferenceBValue() const
{
  return GetReferenceBValue(m_Image);
}

const mitk::DiffusionPropertyHelper::MeasurementFrameType & mitk::DiffusionPropertyHelper::GetMeasurementFrame() const
{
  return GetMeasurementFrame(m_Image);
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer mitk::DiffusionPropertyHelper::GetOriginalGradientContainer() const
{
  return GetOriginalGradientContainer(m_Image);
}

mitk::DiffusionPropertyHelper::GradientDirectionsContainerType::Pointer mitk::DiffusionPropertyHelper::GetGradientContainer() const
{
  return GetGradientContainer(m_Image);
}
