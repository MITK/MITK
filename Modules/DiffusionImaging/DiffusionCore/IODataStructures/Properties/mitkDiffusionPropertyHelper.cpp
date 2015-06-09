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

const std::string mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME = "meta.GradientDirections";
const std::string mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME = "meta.OriginalGradientDirections";
const std::string mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME = "meta.MeasurementFrame";
const std::string mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME = "meta.ReferenceBValue";
const std::string mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME = "BValueMap";

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

  m_Image->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newDirs ) );
  m_Image->SetProperty( mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( newDirs ) );
  ApplyMeasurementFrame();
  UpdateBValueMap();
  std::cout << std::endl;
}

void mitk::DiffusionPropertyHelper::ApplyMeasurementFrame()
{

  if(  m_Image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).IsNull() )
  {
    return;
  }

  GradientDirectionsContainerType::Pointer  originalDirections = static_cast<mitk::GradientDirectionsProperty*>( m_Image->GetProperty(mitk::DiffusionPropertyHelper::ORIGINALGRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer())->GetGradientDirectionsContainer();

  MeasurementFrameType  measurementFrame = GetMeasurementFrame(m_Image);

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
    vec = vec.pre_multiply(measurementFrame);
    MITK_INFO << gdcit.Value();
    directions->InsertElement(c, vec);
    c++;
  }

  m_Image->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( directions ) );
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
        MITK_INFO << gdcit.Value();
      b_ValueMap[GetB_Value(gdcit.Index())].push_back(gdcit.Index());
    }
  }

  m_Image->SetProperty( mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New( b_ValueMap ) );
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

void mitk::DiffusionPropertyHelper::InitializeImage()
{
  this->ApplyMeasurementFrame();
  this->UpdateBValueMap();

  // initialize missing properties
  mitk::MeasurementFrameProperty::Pointer mf = dynamic_cast<mitk::MeasurementFrameProperty *>( m_Image->GetProperty(MEASUREMENTFRAMEPROPERTYNAME.c_str()).GetPointer());
  if( mf.IsNull() )
  {
    //no measurement frame present, identity is assumed
    MeasurementFrameType identity;
    identity.set_identity();
    m_Image->SetProperty( mitk::DiffusionPropertyHelper::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( identity ));
  }
}

bool mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(const mitk::DataNode* node)
{
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
