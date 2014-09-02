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

// Qmitk
#include "QmitkTbssSkeletonizationView.h"


#include <itkSkeletonizationFilter.h>
#include <itkProjectionFilter.h>
#include <itkDistanceMapFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include <itkImageFileReader.h>

// mitk
#include <mitkImagePixelReadAccessor.h>
#include <mitkImage.h>

// Qt
#include <QInputDialog>
#include <QMessageBox>

//vtk
#include <vtkLinearTransform.h>
#include <vtkMatrix4x4.h>

// Boost
#include <boost/lexical_cast.hpp>



const std::string QmitkTbssSkeletonizationView::VIEW_ID = "org.mitk.views.tbssskeletonization";

using namespace berry;


QmitkTbssSkeletonizationView::QmitkTbssSkeletonizationView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
{

}

QmitkTbssSkeletonizationView::~QmitkTbssSkeletonizationView()
{
}

void QmitkTbssSkeletonizationView::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{


  //datamanager selection changed
  if (!this->IsActivated())
    return;

  bool found3dImage = false;
  bool found4dImage = false;

  this->m_Controls->m_TubularName->setText(QString("Tubular Structure Mask: "));
  this->m_Controls->m_TubularName->setEnabled(false);
  this->m_Controls->m_MeanLabel->setText(QString("Mean: "));
  this->m_Controls->m_MeanLabel->setEnabled(false);
  this->m_Controls->m_PatientDataLabel->setText(QString("Patient Data: "));
  this->m_Controls->m_PatientDataLabel->setEnabled(false);


  // iterate selection
  for ( int i=0; i<nodes.size(); i++ )
  {


    // only look at interesting types from valid nodes
    mitk::BaseData* nodeData = nodes[i]->GetData();
    std::string name = "";
    nodes[i]->GetStringProperty("name", name);


    if(nodeData)
    {
      if(QString("Image").compare(nodeData->GetNameOfClass())==0)
      {
        mitk::Image* img = static_cast<mitk::Image*>(nodeData);

        if(img->GetDimension() == 3)
        {
          bool isBinary(false);
          nodes[i]->GetBoolProperty("binary", isBinary);


          if(isBinary)
          {
            QString label("Tubular Structure Mask: ");
            label.append(QString(name.c_str()));
            this->m_Controls->m_TubularName->setText(label);
            this->m_Controls->m_TubularName->setEnabled(true);
          }
          else
          {
            found3dImage = true;
            QString label("Mean: ");
            label.append(QString(name.c_str()));
            this->m_Controls->m_MeanLabel->setText(label);
            this->m_Controls->m_MeanLabel->setEnabled(true);
          }
        }
        else if(img->GetDimension() == 4)
        {
          found4dImage = true;
          QString label("Patient Data: ");
          label.append(QString(name.c_str()));
          this->m_Controls->m_PatientDataLabel->setText(label);
          this->m_Controls->m_PatientDataLabel->setEnabled(true);
        }
      }
    }

  }

  this->m_Controls->m_Skeletonize->setEnabled(found3dImage);
  this->m_Controls->m_Project->setEnabled(found3dImage && found4dImage);
  this->m_Controls->m_OutputMask->setEnabled(found3dImage && found4dImage);
  this->m_Controls->m_OutputDistanceMap->setEnabled(found3dImage && found4dImage);

}



void QmitkTbssSkeletonizationView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkTbssSkeletonizationViewControls;
    m_Controls->setupUi( parent );
    this->CreateConnections();
  }

}

void QmitkTbssSkeletonizationView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkTbssSkeletonizationView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}

void QmitkTbssSkeletonizationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_Skeletonize), SIGNAL(clicked()), this, SLOT(Skeletonize() ));
    connect( (QObject*)(m_Controls->m_Project), SIGNAL(clicked()), this, SLOT(Project() ));
  }
}



void QmitkTbssSkeletonizationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkTbssSkeletonizationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkTbssSkeletonizationView::Skeletonize()
{




  typedef itk::SkeletonizationFilter<FloatImageType, FloatImageType> SkeletonisationFilterType;
  SkeletonisationFilterType::Pointer skeletonizer = SkeletonisationFilterType::New();


  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();


  mitk::Image::Pointer meanImage = mitk::Image::New();
  std::string name = "";

  for ( int i=0; i<nodes.size(); i++ )
  {
    // process only on valid nodes
    mitk::BaseData* nodeData = nodes[i]->GetData();

    if(nodeData)
    {
      if(QString("Image").compare(nodeData->GetNameOfClass())==0)
      {

        bool isBinary(false);
        nodes[i]->GetBoolProperty("binary", isBinary);

        mitk::Image* img = static_cast<mitk::Image*>(nodeData);
        if(img->GetDimension() == 3 && !isBinary)
        {
          meanImage = img;
          name = nodes[i]->GetName();
        }
      }
    }
  }


  // Calculate skeleton
  FloatImageType::Pointer itkImg = FloatImageType::New();
  mitk::CastToItkImage(meanImage, itkImg);
  skeletonizer->SetInput(itkImg);
  skeletonizer->Update();


  FloatImageType::Pointer output = skeletonizer->GetOutput();
  mitk::Image::Pointer mitkOutput = mitk::Image::New();
  mitk::CastToMitkImage(output, mitkOutput);
  name += "_skeleton";
  AddToDataStorage(mitkOutput, name);



}

void QmitkTbssSkeletonizationView::Project()
{

  typedef itk::SkeletonizationFilter<FloatImageType, FloatImageType> SkeletonisationFilterType;
  typedef itk::ProjectionFilter ProjectionFilterType;
  typedef itk::DistanceMapFilter<FloatImageType, FloatImageType> DistanceMapFilterType;

  SkeletonisationFilterType::Pointer skeletonizer = SkeletonisationFilterType::New();



  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();


  mitk::Image::Pointer meanImage = mitk::Image::New();
  mitk::Image::Pointer subjects = mitk::Image::New();
  mitk::Image::Pointer tubular = mitk::Image::New();

  for ( int i=0; i<nodes.size(); i++ )
  {
    // process only on valid nodes
    mitk::BaseData* nodeData = nodes[i]->GetData();

    if(nodeData)
    {
      if(QString("Image").compare(nodeData->GetNameOfClass())==0)
      {
        mitk::Image* img = static_cast<mitk::Image*>(nodeData);
        if(img->GetDimension() == 3)
        {

          bool isBinary(false);
          nodes[i]->GetBoolProperty("binary", isBinary);


          if(isBinary)
          {
            tubular = img;
          }
          else
          {
            meanImage = img;
          }
        }

        else if(img->GetDimension() == 4)
        {
          subjects = img;
        }
      }
    }

  }

  Float4DImageType::Pointer allFA = ConvertToItk(subjects);

    // Calculate skeleton
  FloatImageType::Pointer itkImg = FloatImageType::New();
  mitk::CastToItkImage(meanImage, itkImg);
  skeletonizer->SetInput(itkImg);
  skeletonizer->Update();


  FloatImageType::Pointer output = skeletonizer->GetOutput();
  mitk::Image::Pointer mitkOutput = mitk::Image::New();
  mitk::CastToMitkImage(output, mitkOutput);
  AddToDataStorage(mitkOutput, "mean_FA_skeletonised");



  // Retrieve direction image needed later by the projection filter
  DirectionImageType::Pointer directionImg = skeletonizer->GetVectorImage();


  // Calculate distance image
  DistanceMapFilterType::Pointer distanceMapFilter = DistanceMapFilterType::New();
  distanceMapFilter->SetInput(output);
  distanceMapFilter->Update();
  FloatImageType::Pointer distanceMap = distanceMapFilter->GetOutput();

  if(m_Controls->m_OutputDistanceMap->isChecked())
  {
    mitk::Image::Pointer mitkDistance = mitk::Image::New();
    mitk::CastToMitkImage(distanceMap, mitkDistance);
    AddToDataStorage(mitkDistance, "distance map");
  }

  // Do projection

  // Ask a threshold to create a skeleton mask
  double threshold = -1.0;
  while(threshold == -1.0)
  {
    threshold = QInputDialog::getDouble(m_Controls->m_Skeletonize, tr("Specify the FA threshold"),
                                        tr("Threshold:"), QLineEdit::Normal,
                                        0.2);

    if(threshold < 0.0 || threshold > 1.0)
    {
      QMessageBox msgBox;
      msgBox.setText("Please choose a value between 0 and 1");
      msgBox.exec();
      threshold = -1.0;
    }
  }

  typedef itk::BinaryThresholdImageFilter<FloatImageType, CharImageType> ThresholdFilterType;
  ThresholdFilterType::Pointer thresholder = ThresholdFilterType::New();
  thresholder->SetInput(output);
  thresholder->SetLowerThreshold(threshold);
  thresholder->SetUpperThreshold(std::numeric_limits<float>::max());
  thresholder->SetOutsideValue(0);
  thresholder->SetInsideValue(1);
  thresholder->Update();


  CharImageType::Pointer thresholdedImg = thresholder->GetOutput();


  if(m_Controls->m_OutputMask->isChecked())
  {
    mitk::Image::Pointer mitkThresholded = mitk::Image::New();
    mitk::CastToMitkImage(thresholdedImg, mitkThresholded);
    std::string maskName = "skeleton_mask_at_" + boost::lexical_cast<std::string>(threshold);
    AddToDataStorage(mitkThresholded, maskName);
  }




  CharImageType::Pointer itkTubular = CharImageType::New();
  mitk::CastToItkImage(tubular, itkTubular);


  ProjectionFilterType::Pointer projectionFilter = ProjectionFilterType::New();
  projectionFilter->SetDistanceMap(distanceMap);
  projectionFilter->SetDirections(directionImg);
  projectionFilter->SetAllFA(allFA);
  projectionFilter->SetTube(itkTubular);
  projectionFilter->SetSkeleton(thresholdedImg);
  projectionFilter->Project();

  Float4DImageType::Pointer projected = projectionFilter->GetProjections();

  mitk::Image::Pointer mitkProjections = mitk::Image::New();
  mitk::CastToMitkImage(projected, mitkProjections);

  AddToDataStorage(mitkProjections, "all_FA_projected");



}

void QmitkTbssSkeletonizationView::AddToDataStorage(mitk::Image* img, std::string name)
{
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty( "name", mitk::StringProperty::New(name) );
  result->SetData( img );

  // add new image to data storage and set as active to ease further processing
  GetDefaultDataStorage()->Add( result );
}


Float4DImageType::Pointer QmitkTbssSkeletonizationView::ConvertToItk(itk::SmartPointer<mitk::Image> image)
{

  Float4DImageType::Pointer output = Float4DImageType::New();

  mitk::BaseGeometry* geo = image->GetGeometry();
  mitk::Vector3D mitkSpacing = geo->GetSpacing();
  mitk::Point3D mitkOrigin = geo->GetOrigin();

  Float4DImageType::SpacingType spacing;
  spacing[0] = mitkSpacing[0];
  spacing[1] = mitkSpacing[1];
  spacing[2] = mitkSpacing[2];
  spacing[3] = 1.0; // todo: check if spacing has length 4

  Float4DImageType::PointType origin;
  origin[0] = mitkOrigin[0];
  origin[1] = mitkOrigin[1];
  origin[2] = mitkOrigin[2];
  origin[3] = 0;

  Float4DImageType::SizeType size;
  size[0] = image->GetDimension(0);
  size[1] = image->GetDimension(1);
  size[2] = image->GetDimension(2);
  size[3] = image->GetDimension(3);

  Float4DImageType::DirectionType dir;
  vtkLinearTransform* lin = geo->GetVtkTransform();
  vtkMatrix4x4 *m = lin->GetMatrix();

  dir.Fill(0.0);
  for(int x=0; x<3; x++)
  {
    for(int y=0; y<3; y++)
    {
      dir[x][y] = m->GetElement(x,y);
    }
  }
  dir[3][3] = 1;



  output->SetSpacing(spacing);
  output->SetOrigin(origin);
  output->SetRegions(size);
  output->SetDirection(dir);
  output->Allocate();


  if(image->GetDimension() == 4)
  {
    int timesteps = image->GetDimension(3);

    try{
      // REPLACE THIS METHODE()ConvertToItk) WITH mitk::CastToItk

      // iterate through the subjects and copy data to output
      for(int t=0; t<timesteps; t++)
      {
        for(int x=0; x<image->GetDimension(0); x++)
        {
          for(int y=0; y<image->GetDimension(1); y++)
          {
            for(int z=0; z<image->GetDimension(2); z++)
            {
              itk::Index<3> ix = {x, y, z};
              itk::Index<4> ix4 = {x, y, z, t};

              output->SetPixel(ix4, image->GetPixelValueByIndex(ix, t));

            }
          }
        }
      }
    }
    catch(std::exception & e)
    {
      MITK_INFO << e.what();
    }

  }


  return output;
}
