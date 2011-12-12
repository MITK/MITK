/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryISelectionService.h>
#include <berryConstants.h>
#include <berryPlatformUI.h>

// Qmitk
#include "QmitkTbssSkeletonizationView.h"
#include <QmitkStdMultiWidget.h>

#include <mitkDataNodeObject.h>
#include <itkCastImageFilter.h>

#include <itkSkeletonizationFilter.h>
#include <itkProjectionFilter.h>
#include <itkDistanceMapFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include <itkImageFileReader.h>

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


struct TbssSkeletonizationSelListener : ISelectionListener
{

  berryObjectMacro(TbssSkeletonizationSelListener)

  TbssSkeletonizationSelListener(QmitkTbssSkeletonizationView* view)
  {
    m_View = view;
  }


  void DoSelectionChanged(ISelection::ConstPointer selection)
  {
    // save current selection in member variable
    m_View->m_CurrentSelection = selection.Cast<const IStructuredSelection>();

    // do something with the selected items
    if(m_View->m_CurrentSelection)
    {

      bool found3dImage = false;
      bool found4dImage = false;

      // iterate selection
      for (IStructuredSelection::iterator i = m_View->m_CurrentSelection->Begin();
        i != m_View->m_CurrentSelection->End(); ++i)
      {

        // extract datatree node
        if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
        {
          mitk::DataNode::Pointer node = nodeObj->GetDataNode();

          // only look at interesting types

          if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
          {
            mitk::Image* img = static_cast<mitk::Image*>(node->GetData());
            if(img->GetDimension() == 3)
            {
              found3dImage = true;
            }
            else if(img->GetDimension() == 4)
            {
              found4dImage = true;
            }
          }
        }

      }

      m_View->m_Controls->m_Skeletonize->setEnabled(found3dImage);
      m_View->m_Controls->m_Project->setEnabled(found3dImage && found4dImage);
      m_View->m_Controls->m_OutputMask->setEnabled(found3dImage && found4dImage);
      m_View->m_Controls->m_OutputDistanceMap->setEnabled(found3dImage && found4dImage);
    }
  }


  void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection)
  {
    // check, if selection comes from datamanager
    if (part)
    {
      QString partname(part->GetPartName().c_str());
      if(partname.compare("Datamanager")==0)
      {
        // apply selection
        DoSelectionChanged(selection);
      }
    }
  }

  QmitkTbssSkeletonizationView* m_View;
};


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

  m_SelListener = berry::ISelectionListener::Pointer(new TbssSkeletonizationSelListener(this));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_SelListener);
  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TbssSkeletonizationSelListener>()->DoSelectionChanged(sel);

  m_IsInitialized = false;



}

void QmitkTbssSkeletonizationView::Activated()
{
  QmitkFunctionality::Activated();

  berry::ISelection::ConstPointer sel(
    this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  m_CurrentSelection = sel.Cast<const IStructuredSelection>();
  m_SelListener.Cast<TbssSkeletonizationSelListener>()->DoSelectionChanged(sel);
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

  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();


    mitk::Image::Pointer meanImage = mitk::Image::New();

    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin();
      i != m_CurrentSelection->End();
      ++i)
    {
      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();

        if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
        {
          mitk::Image* img = static_cast<mitk::Image*>(node->GetData());
          if(img->GetDimension() == 3)
          {
            meanImage = img;
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
    AddToDataStorage(mitkOutput, "all_FA_skeletonised");


  }
}

void QmitkTbssSkeletonizationView::Project()
{
  typedef itk::SkeletonizationFilter<FloatImageType, FloatImageType> SkeletonisationFilterType;
  typedef itk::ProjectionFilter ProjectionFilterType;
  typedef itk::DistanceMapFilter<FloatImageType, FloatImageType> DistanceMapFilterType;

  SkeletonisationFilterType::Pointer skeletonizer = SkeletonisationFilterType::New();

  if (m_CurrentSelection)
  {
    mitk::DataStorage::SetOfObjects::Pointer set =
      mitk::DataStorage::SetOfObjects::New();


    mitk::Image::Pointer meanImage = mitk::Image::New();
    mitk::Image::Pointer subjects = mitk::Image::New();

    for (IStructuredSelection::iterator i = m_CurrentSelection->Begin();
      i != m_CurrentSelection->End();
      ++i)
    {
      if (mitk::DataNodeObject::Pointer nodeObj = i->Cast<mitk::DataNodeObject>())
      {
        mitk::DataNode::Pointer node = nodeObj->GetDataNode();

        if(QString("Image").compare(node->GetData()->GetNameOfClass())==0)
        {
          mitk::Image* img = static_cast<mitk::Image*>(node->GetData());
          if(img->GetDimension() == 3)
          {
            meanImage = img;
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



    typedef itk::ImageFileReader< CharImageType > CharReaderType;
    CharReaderType::Pointer reader = CharReaderType::New();
    reader->SetFileName("/local/testing/LowerCingulum_1mm.nii.gz");
    reader->Update();
    CharImageType::Pointer cingulum = reader->GetOutput();



    ProjectionFilterType::Pointer projectionFilter = ProjectionFilterType::New();
    projectionFilter->SetDistanceMap(distanceMap);
    projectionFilter->SetDirections(directionImg);
    projectionFilter->SetAllFA(allFA);
    projectionFilter->SetTube(cingulum);
    projectionFilter->SetSkeleton(thresholdedImg);
    projectionFilter->Project();

    Float4DImageType::Pointer projected = projectionFilter->GetProjections();

    mitk::Image::Pointer mitkProjections = mitk::Image::New();
    mitk::CastToMitkImage(projected, mitkProjections);

    AddToDataStorage(mitkProjections, "all_FA_projected");

  }
}

void QmitkTbssSkeletonizationView::AddToDataStorage(mitk::Image* img, std::string name)
{
  mitk::DataNode::Pointer result = mitk::DataNode::New();
  result->SetProperty( "name", mitk::StringProperty::New(name) );
  result->SetData( img );

  // add new image to data storage and set as active to ease further processing
  GetDefaultDataStorage()->Add( result );
}


Float4DImageType::Pointer QmitkTbssSkeletonizationView::ConvertToItk(mitk::Image::Pointer image)
{
  Float4DImageType::Pointer output = Float4DImageType::New();

  mitk::Geometry3D* geo = image->GetGeometry();
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

    // iterate through the subjects and copy data to output
    for(int t=0; t<timesteps; t++)
    {
      for(int x=0; x<image->GetDimension(0); x++)
      {
        for(int y=0; y<image->GetDimension(1); y++)
        {
          for(int z=0; z<image->GetDimension(2); z++)
          {
            itk::Index<4> ix4;
            ix4[0] = x;
            ix4[1] = y;
            ix4[2] = z;
            ix4[3] = t;

            mitk::Index3D ix;
            ix[0] = x;
            ix[1] = y;
            ix[2] = z;

            output->SetPixel(ix4, image->GetPixelValueByIndex(ix, t));

          }
        }
      }
    }

  }


  return output;
}
