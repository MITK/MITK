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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchPart.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberProcessingView.h"

// Qt
#include <QMessageBox>

// MITK
#include <mitkNodePredicateProperty.h>
#include <mitkImageCast.h>
#include <mitkPointSet.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkImageAccessByItk.h>
#include <mitkDataNodeObject.h>
#include <mitkTensorImage.h>
#include "usModuleRegistry.h"
#include <itkFiberCurvatureFilter.h>

#include "mitkNodePredicateDataType.h"
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>

// ITK
#include <itkResampleImageFilter.h>
#include <itkGaussianInterpolateImageFunction.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkTractsToFiberEndingsImageFilter.h>
#include <itkTractDensityImageFilter.h>
#include <itkImageRegion.h>
#include <itkTractsToRgbaImageFilter.h>
#include <itkFiberExtractionFilter.h>

const std::string QmitkFiberProcessingView::VIEW_ID = "org.mitk.views.fiberprocessing";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace mitk;

QmitkFiberProcessingView::QmitkFiberProcessingView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_CircleCounter(0)
  , m_PolygonCounter(0)
  , m_UpsamplingFactor(1)
{

}

// Destructor
QmitkFiberProcessingView::~QmitkFiberProcessingView()
{
  RemoveObservers();
}

void QmitkFiberProcessingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberProcessingViewControls;
    m_Controls->setupUi( parent );

    connect( m_Controls->m_CircleButton, SIGNAL( clicked() ), this, SLOT( OnDrawCircle() ) );
    connect( m_Controls->m_PolygonButton, SIGNAL( clicked() ), this, SLOT( OnDrawPolygon() ) );
    connect(m_Controls->PFCompoANDButton, SIGNAL(clicked()), this, SLOT(GenerateAndComposite()) );
    connect(m_Controls->PFCompoORButton, SIGNAL(clicked()), this, SLOT(GenerateOrComposite()) );
    connect(m_Controls->PFCompoNOTButton, SIGNAL(clicked()), this, SLOT(GenerateNotComposite()) );
    connect(m_Controls->m_GenerateRoiImage, SIGNAL(clicked()), this, SLOT(GenerateRoiImage()) );

    connect(m_Controls->m_JoinBundles, SIGNAL(clicked()), this, SLOT(JoinBundles()) );
    connect(m_Controls->m_SubstractBundles, SIGNAL(clicked()), this, SLOT(SubstractBundles()) );
    connect(m_Controls->m_CopyBundle, SIGNAL(clicked()), this, SLOT(CopyBundles()) );

    connect(m_Controls->m_ExtractFibersButton, SIGNAL(clicked()), this, SLOT(Extract()));
    connect(m_Controls->m_RemoveButton, SIGNAL(clicked()), this, SLOT(Remove()));
    connect(m_Controls->m_ModifyButton, SIGNAL(clicked()), this, SLOT(Modify()));

    connect(m_Controls->m_ExtractionMethodBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
    connect(m_Controls->m_RemovalMethodBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
    connect(m_Controls->m_ModificationMethodBox, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateGui()));
    connect(m_Controls->m_ExtractionBoxMask, SIGNAL(currentIndexChanged(int)), this, SLOT(OnMaskExtractionChanged()));

    m_Controls->m_ColorMapBox->SetDataStorage(this->GetDataStorage());
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
    mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
    mitk::NodePredicateDataType::Pointer isOdf = mitk::NodePredicateDataType::New("OdfImage");
    mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
    isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isOdf);
    mitk::NodePredicateNot::Pointer noDiffusionImage = mitk::NodePredicateNot::New(isDiffusionImage);
    mitk::NodePredicateAnd::Pointer finalPredicate = mitk::NodePredicateAnd::New(isMitkImage, noDiffusionImage);
    m_Controls->m_ColorMapBox->SetPredicate(finalPredicate);

    m_Controls->label_17->setVisible(false);
    m_Controls->m_FiberExtractionFractionBox->setVisible(false);
  }

  UpdateGui();
}

void QmitkFiberProcessingView::OnMaskExtractionChanged()
{
  if (m_Controls->m_ExtractionBoxMask->currentIndex() == 2 || m_Controls->m_ExtractionBoxMask->currentIndex() == 3)
  {
    m_Controls->label_17->setVisible(true);
    m_Controls->m_FiberExtractionFractionBox->setVisible(true);
    m_Controls->m_BothEnds->setVisible(false);
  }
  else
  {
    m_Controls->label_17->setVisible(false);
    m_Controls->m_FiberExtractionFractionBox->setVisible(false);
    if (m_Controls->m_ExtractionBoxMask->currentIndex() != 3)
      m_Controls->m_BothEnds->setVisible(true);
  }
}

void QmitkFiberProcessingView::SetFocus()
{
  m_Controls->toolBoxx->setFocus();
}

void QmitkFiberProcessingView::Modify()
{
  switch (m_Controls->m_ModificationMethodBox->currentIndex())
  {
  case 0:
  {
    ResampleSelectedBundlesSpline();
    break;
  }
  case 1:
  {
    ResampleSelectedBundlesLinear();
    break;
  }
  case 2:
  {
    CompressSelectedBundles();
    break;
  }
  case 3:
  {
    DoImageColorCoding();
    break;
  }
  case 4:
  {
    MirrorFibers();
    break;
  }
  case 5:
  {
    WeightFibers();
    break;
  }
  case 6:
  {
    DoCurvatureColorCoding();
    break;
  }
  case 7:
  {
    DoWeightColorCoding();
    break;
  }
  case 8:
  {
    DoLengthColorCoding();
    break;
  }
  }
}

void QmitkFiberProcessingView::WeightFibers()
{
  float weight = this->m_Controls->m_BundleWeightBox->value();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->SetFiberWeights(weight);
  }

}

void QmitkFiberProcessingView::Remove()
{
  switch (m_Controls->m_RemovalMethodBox->currentIndex())
  {
  case 0:
  {
    RemoveDir();
    break;
  }
  case 1:
  {
    PruneBundle();
    break;
  }
  case 2:
  {
    ApplyCurvatureThreshold();
    break;
  }
  case 3:
  {
    RemoveWithMask(false);
    break;
  }
  case 4:
  {
    RemoveWithMask(true);
    break;
  }
  case 5:
  {
    ApplyWeightThreshold();
    break;
  }
  }
}

void QmitkFiberProcessingView::Extract()
{
  switch (m_Controls->m_ExtractionMethodBox->currentIndex())
  {
  case 0:
  {
    ExtractWithPlanarFigure();
    break;
  }
  case 1:
  {
    switch (m_Controls->m_ExtractionBoxMask->currentIndex())
    {
    {
    case 0:
        ExtractWithMask(true, false);
        break;
    }
    {
    case 1:
        ExtractWithMask(true, true);
        break;
    }
    {
    case 2:
        ExtractWithMask(false, false);
        break;
    }
    {
    case 3:
        ExtractWithMask(false, true);
        break;
    }
    }
    break;
  }
  }
}

void QmitkFiberProcessingView::PruneBundle()
{
  int minLength = this->m_Controls->m_PruneFibersMinBox->value();
  int maxLength = this->m_Controls->m_PruneFibersMaxBox->value();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    if (!fib->RemoveShortFibers(minLength))
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
    else if (!fib->RemoveLongFibers(maxLength))
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::ApplyWeightThreshold()
{
  float thr = this->m_Controls->m_WeightThresholdBox->value();
  std::vector< DataNode::Pointer > nodes = m_SelectedFB;
  for (auto node : nodes)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());

    mitk::FiberBundle::Pointer newFib = fib->FilterByWeights(thr);
    if (newFib->GetNumFibers()>0)
    {
      newFib->ColorFibersByFiberWeights(false, true);
      node->SetData(newFib);
    }
    else
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::ApplyCurvatureThreshold()
{
  int angle = this->m_Controls->m_CurvSpinBox->value();
  int dist = this->m_Controls->m_CurvDistanceSpinBox->value();
  std::vector< DataNode::Pointer > nodes = m_SelectedFB;
  for (auto node : nodes)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());

    itk::FiberCurvatureFilter::Pointer filter = itk::FiberCurvatureFilter::New();
    filter->SetInputFiberBundle(fib);
    filter->SetAngularDeviation(angle);
    filter->SetDistance(dist);
    filter->SetRemoveFibers(m_Controls->m_RemoveCurvedFibersBox->isChecked());
    filter->Update();
    mitk::FiberBundle::Pointer newFib = filter->GetOutputFiberBundle();
    if (newFib->GetNumFibers()>0)
    {
      newFib->ColorFibersByOrientation();
      node->SetData(newFib);
    }
    else
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::RemoveDir()
{
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    vnl_vector_fixed<double,3> dir;
    dir[0] = m_Controls->m_ExtractDirX->value();
    dir[1] = m_Controls->m_ExtractDirY->value();
    dir[2] = m_Controls->m_ExtractDirZ->value();
    fib->RemoveDir(dir,cos((float)m_Controls->m_ExtractAngle->value()*itk::Math::pi/180));
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::RemoveWithMask(bool removeInside)
{
  if (m_RoiImageNode.IsNull())
    return;

  mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_RoiImageNode->GetData());
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());

    ItkUCharImageType::Pointer mask = ItkUCharImageType::New();
    mitk::CastToItkImage(mitkMask, mask);
    mitk::FiberBundle::Pointer newFib = fib->RemoveFibersOutside(mask, removeInside);
    if (newFib->GetNumFibers()<=0)
    {
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
      continue;
    }
    node->SetData(newFib);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::ExtractWithMask(bool onlyEnds, bool invert)
{
  if (m_RoiImageNode.IsNull())
    return;

  mitk::Image::Pointer mitkMask = dynamic_cast<mitk::Image*>(m_RoiImageNode->GetData());
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    QString name(node->GetName().c_str());

    ItkFloatImageType::Pointer mask = ItkFloatImageType::New();
    mitk::CastToItkImage(mitkMask, mask);

    itk::FiberExtractionFilter<float>::Pointer extractor = itk::FiberExtractionFilter<float>::New();
    extractor->SetInputFiberBundle(fib);
    extractor->SetRoiImages({mask});
    extractor->SetThreshold(m_Controls->m_FiberExtractionThresholdBox->value());
    extractor->SetOverlapFraction(m_Controls->m_FiberExtractionFractionBox->value());
    extractor->SetBothEnds(m_Controls->m_BothEnds->isChecked());
    extractor->SetInterpolate(m_Controls->m_InterpolateRoiBox->isChecked());
    if (invert)
      extractor->SetNoPositives(true);
    else
      extractor->SetNoNegatives(true);
    if (onlyEnds)
      extractor->SetMode(itk::FiberExtractionFilter<float>::MODE::ENDPOINTS);
    extractor->Update();

    mitk::FiberBundle::Pointer newFib;
    if (invert)
      newFib = extractor->GetNegatives().at(0);
    else
      newFib = extractor->GetPositives().at(0);

    if (newFib.IsNull() || newFib->GetNumFibers()<=0)
    {
      QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
      continue;
    }

    DataNode::Pointer newNode = DataNode::New();
    newNode->SetData(newFib);

    if (invert)
    {
      name += "_not";

      if (onlyEnds)
        name += "-ending-in-mask";
      else
        name += "-passing-mask";
    }
    else
    {
      if (onlyEnds)
        name += "_ending-in-mask";
      else
        name += "_passing-mask";
    }

    newNode->SetName(name.toStdString());
    GetDataStorage()->Add(newNode);
    node->SetVisibility(false);
  }
}

void QmitkFiberProcessingView::GenerateRoiImage()
{
  if (m_SelectedPF.empty())
    return;

  mitk::BaseGeometry::Pointer geometry;
  if (!m_SelectedFB.empty())
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.front()->GetData());
    geometry = fib->GetGeometry();
  }
  else if (m_SelectedImage)
    geometry = m_SelectedImage->GetGeometry();
  else
    return;

  itk::Vector<double,3> spacing = geometry->GetSpacing();
  spacing /= m_UpsamplingFactor;

  mitk::Point3D newOrigin = geometry->GetOrigin();
  mitk::Geometry3D::BoundsArrayType bounds = geometry->GetBounds();
  newOrigin[0] += bounds.GetElement(0);
  newOrigin[1] += bounds.GetElement(2);
  newOrigin[2] += bounds.GetElement(4);

  itk::Matrix<double, 3, 3> direction;
  itk::ImageRegion<3> imageRegion;
  for (int i=0; i<3; i++)
    for (int j=0; j<3; j++)
      direction[j][i] = geometry->GetMatrixColumn(i)[j]/spacing[j];
  imageRegion.SetSize(0, geometry->GetExtent(0)*m_UpsamplingFactor);
  imageRegion.SetSize(1, geometry->GetExtent(1)*m_UpsamplingFactor);
  imageRegion.SetSize(2, geometry->GetExtent(2)*m_UpsamplingFactor);

  m_PlanarFigureImage = ItkUCharImageType::New();
  m_PlanarFigureImage->SetSpacing( spacing );   // Set the image spacing
  m_PlanarFigureImage->SetOrigin( newOrigin );     // Set the image origin
  m_PlanarFigureImage->SetDirection( direction );  // Set the image direction
  m_PlanarFigureImage->SetRegions( imageRegion );
  m_PlanarFigureImage->Allocate();
  m_PlanarFigureImage->FillBuffer( 0 );

  Image::Pointer tmpImage = Image::New();
  tmpImage->InitializeByItk(m_PlanarFigureImage.GetPointer());
  tmpImage->SetVolume(m_PlanarFigureImage->GetBufferPointer());

  std::string name = m_SelectedPF.at(0)->GetName();
  WritePfToImage(m_SelectedPF.at(0), tmpImage);
  for (unsigned int i=1; i<m_SelectedPF.size(); i++)
  {
    name += "+";
    name += m_SelectedPF.at(i)->GetName();
    WritePfToImage(m_SelectedPF.at(i), tmpImage);
  }

  DataNode::Pointer node = DataNode::New();
  tmpImage = Image::New();
  tmpImage->InitializeByItk(m_PlanarFigureImage.GetPointer());
  tmpImage->SetVolume(m_PlanarFigureImage->GetBufferPointer());
  node->SetData(tmpImage);
  node->SetName(name);
  this->GetDataStorage()->Add(node);
}

void QmitkFiberProcessingView::WritePfToImage(mitk::DataNode::Pointer node, mitk::Image* image)
{
  if (dynamic_cast<mitk::PlanarFigure*>(node->GetData()))
  {
    m_PlanarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    AccessFixedDimensionByItk_2(
          image,
          InternalReorientImagePlane, 3,
          m_PlanarFigure->GetGeometry(), -1);

    AccessFixedDimensionByItk_2(
          m_InternalImage,
          InternalCalculateMaskFromPlanarFigure,
          3, 2, node->GetName() );
  }
  else if (dynamic_cast<mitk::PlanarFigureComposite*>(node->GetData()))
  {
    DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(node);
    for (unsigned int i=0; i<children->Size(); i++)
    {
      WritePfToImage(children->at(i), image);
    }
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkFiberProcessingView::InternalReorientImagePlane( const itk::Image< TPixel, VImageDimension > *image, mitk::BaseGeometry* planegeo3D, int additionalIndex )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< float, VImageDimension > FloatImageType;

  typedef itk::ResampleImageFilter<ImageType, FloatImageType, double> ResamplerType;
  typename ResamplerType::Pointer resampler = ResamplerType::New();

  mitk::PlaneGeometry* planegeo = dynamic_cast<mitk::PlaneGeometry*>(planegeo3D);

  float upsamp = m_UpsamplingFactor;
  float gausssigma = 0.5;

  // Spacing
  typename ResamplerType::SpacingType spacing = planegeo->GetSpacing();
  spacing[0] = image->GetSpacing()[0] / upsamp;
  spacing[1] = image->GetSpacing()[1] / upsamp;
  spacing[2] = image->GetSpacing()[2];
  resampler->SetOutputSpacing( spacing );

  // Size
  typename ResamplerType::SizeType size;
  size[0] = planegeo->GetExtentInMM(0) / spacing[0];
  size[1] = planegeo->GetExtentInMM(1) / spacing[1];
  size[2] = 1;
  resampler->SetSize( size );

  // Origin
  typename mitk::Point3D orig = planegeo->GetOrigin();
  typename mitk::Point3D corrorig;
  planegeo3D->WorldToIndex(orig,corrorig);
  corrorig[0] += 0.5/upsamp;
  corrorig[1] += 0.5/upsamp;
  corrorig[2] += 0;
  planegeo3D->IndexToWorld(corrorig,corrorig);
  resampler->SetOutputOrigin(corrorig );

  // Direction
  typename ResamplerType::DirectionType direction;
  typename mitk::AffineTransform3D::MatrixType matrix = planegeo->GetIndexToWorldTransform()->GetMatrix();
  for(unsigned int c=0; c<matrix.ColumnDimensions; c++)
  {
    double sum = 0;
    for(unsigned int r=0; r<matrix.RowDimensions; r++)
      sum += matrix(r,c)*matrix(r,c);
    for(unsigned int r=0; r<matrix.RowDimensions; r++)
      direction(r,c) = matrix(r,c)/sqrt(sum);
  }
  resampler->SetOutputDirection( direction );

  // Gaussian interpolation
  if(gausssigma != 0)
  {
    double sigma[3];
    for( unsigned int d = 0; d < 3; d++ )
      sigma[d] = gausssigma * image->GetSpacing()[d];
    double alpha = 2.0;
    typedef itk::GaussianInterpolateImageFunction<ImageType, double> GaussianInterpolatorType;
    typename GaussianInterpolatorType::Pointer interpolator = GaussianInterpolatorType::New();
    interpolator->SetInputImage( image );
    interpolator->SetParameters( sigma, alpha );
    resampler->SetInterpolator( interpolator );
  }
  else
  {
    typedef typename itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;
    typename InterpolatorType::Pointer interpolator = InterpolatorType::New();
    interpolator->SetInputImage( image );
    resampler->SetInterpolator( interpolator );
  }

  resampler->SetInput( image );
  resampler->SetDefaultPixelValue(0);
  resampler->Update();

  if(additionalIndex < 0)
  {
    this->m_InternalImage = mitk::Image::New();
    this->m_InternalImage->InitializeByItk( resampler->GetOutput() );
    this->m_InternalImage->SetVolume( resampler->GetOutput()->GetBufferPointer() );
  }
}

template < typename TPixel, unsigned int VImageDimension >
void QmitkFiberProcessingView::InternalCalculateMaskFromPlanarFigure( itk::Image< TPixel, VImageDimension > *image, unsigned int axis, std::string )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  // Generate mask image as new image with same header as input image and
  // initialize with "1".
  ItkUCharImageType::Pointer newMaskImage = ItkUCharImageType::New();
  newMaskImage->SetSpacing( image->GetSpacing() );   // Set the image spacing
  newMaskImage->SetOrigin( image->GetOrigin() );     // Set the image origin
  newMaskImage->SetDirection( image->GetDirection() );  // Set the image direction
  newMaskImage->SetRegions( image->GetLargestPossibleRegion() );
  newMaskImage->Allocate();
  newMaskImage->FillBuffer( 1 );

  // Generate VTK polygon from (closed) PlanarFigure polyline
  // (The polyline points are shifted by -0.5 in z-direction to make sure
  // that the extrusion filter, which afterwards elevates all points by +0.5
  // in z-direction, creates a 3D object which is cut by the the plane z=0)
  const PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
  const PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const BaseGeometry *imageGeometry3D = m_InternalImage->GetGeometry( 0 );

  vtkPolyData *polyline = vtkPolyData::New();
  polyline->Allocate( 1, 1 );

  // Determine x- and y-dimensions depending on principal axis
  int i0, i1;
  switch ( axis )
  {
  case 0:
    i0 = 1;
    i1 = 2;
    break;
  case 1:
    i0 = 0;
    i1 = 2;
    break;
  case 2:
  default:
    i0 = 0;
    i1 = 1;
    break;
  }

  // Create VTK polydata object of polyline contour
  vtkPoints *points = vtkPoints::New();
  PlanarFigure::PolyLineType::const_iterator it;
  unsigned int numberOfPoints = 0;

  for ( it = planarFigurePolyline.begin(); it != planarFigurePolyline.end(); ++it )
  {
    Point3D point3D;

    // Convert 2D point back to the local index coordinates of the selected image
    Point2D point2D = *it;
    planarFigurePlaneGeometry->WorldToIndex(point2D, point2D);
    point2D[0] -= 0.5/m_UpsamplingFactor;
    point2D[1] -= 0.5/m_UpsamplingFactor;
    planarFigurePlaneGeometry->IndexToWorld(point2D, point2D);
    planarFigurePlaneGeometry->Map( point2D, point3D );

    // Polygons (partially) outside of the image bounds can not be processed further due to a bug in vtkPolyDataToImageStencil
    if ( !imageGeometry3D->IsInside( point3D ) )
    {
      float bounds[2] = {0,0};
      bounds[0] =
          this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i0);
      bounds[1] =
          this->m_InternalImage->GetLargestPossibleRegion().GetSize().GetElement(i1);

      imageGeometry3D->WorldToIndex( point3D, point3D );

      if (point3D[i0]<0)
        point3D[i0] = 0.0;
      else if (point3D[i0]>bounds[0])
        point3D[i0] = bounds[0]-0.001;

      if (point3D[i1]<0)
        point3D[i1] = 0.0;
      else if (point3D[i1]>bounds[1])
        point3D[i1] = bounds[1]-0.001;

      points->InsertNextPoint( point3D[i0], point3D[i1], -0.5 );
      numberOfPoints++;
    }
    else
    {
      imageGeometry3D->WorldToIndex( point3D, point3D );
      // Add point to polyline array
      points->InsertNextPoint( point3D[i0], point3D[i1], -0.5 );
      numberOfPoints++;
    }
  }
  polyline->SetPoints( points );
  points->Delete();

  vtkIdType *ptIds = new vtkIdType[numberOfPoints];
  for ( vtkIdType i = 0; i < numberOfPoints; ++i )
    ptIds[i] = i;
  polyline->InsertNextCell( VTK_POLY_LINE, numberOfPoints, ptIds );

  // Extrude the generated contour polygon
  vtkLinearExtrusionFilter *extrudeFilter = vtkLinearExtrusionFilter::New();
  extrudeFilter->SetInputData( polyline );
  extrudeFilter->SetScaleFactor( 1 );
  extrudeFilter->SetExtrusionTypeToNormalExtrusion();
  extrudeFilter->SetVector( 0.0, 0.0, 1.0 );

  // Make a stencil from the extruded polygon
  vtkPolyDataToImageStencil *polyDataToImageStencil = vtkPolyDataToImageStencil::New();
  polyDataToImageStencil->SetInputConnection( extrudeFilter->GetOutputPort() );

  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< ItkUCharImageType > ImageImportType;
  typedef itk::VTKImageExport< ItkUCharImageType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( newMaskImage );

  vtkImageImport *vtkImporter = vtkImageImport::New();
  this->ConnectPipelines( itkExporter, vtkImporter );
  vtkImporter->Update();

  // Apply the generated image stencil to the input image
  vtkImageStencil *imageStencilFilter = vtkImageStencil::New();
  imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
  imageStencilFilter->SetStencilConnection(polyDataToImageStencil->GetOutputPort() );
  imageStencilFilter->ReverseStencilOff();
  imageStencilFilter->SetBackgroundValue( 0 );
  imageStencilFilter->Update();

  // Export from VTK back to ITK
  vtkImageExport *vtkExporter = vtkImageExport::New();
  vtkExporter->SetInputConnection( imageStencilFilter->GetOutputPort() );
  vtkExporter->Update();

  typename ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();

  // calculate cropping bounding box
  m_InternalImageMask3D = itkImporter->GetOutput();
  m_InternalImageMask3D->SetDirection(image->GetDirection());

  itk::ImageRegionConstIterator<ItkUCharImageType>
      itmask(m_InternalImageMask3D, m_InternalImageMask3D->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType>
      itimage(image, image->GetLargestPossibleRegion());

  itmask.GoToBegin();
  itimage.GoToBegin();

  typename ImageType::SizeType lowersize = {{itk::NumericTraits<unsigned long>::max(),itk::NumericTraits<unsigned long>::max(),itk::NumericTraits<unsigned long>::max()}};
  typename ImageType::SizeType uppersize = {{0,0,0}};
  while( !itmask.IsAtEnd() )
  {
    if(itmask.Get() == 0)
      itimage.Set(0);
    else
    {
      typename ImageType::IndexType index = itimage.GetIndex();
      typename ImageType::SizeType signedindex;
      signedindex[0] = index[0];
      signedindex[1] = index[1];
      signedindex[2] = index[2];

      lowersize[0] = signedindex[0] < lowersize[0] ? signedindex[0] : lowersize[0];
      lowersize[1] = signedindex[1] < lowersize[1] ? signedindex[1] : lowersize[1];
      lowersize[2] = signedindex[2] < lowersize[2] ? signedindex[2] : lowersize[2];

      uppersize[0] = signedindex[0] > uppersize[0] ? signedindex[0] : uppersize[0];
      uppersize[1] = signedindex[1] > uppersize[1] ? signedindex[1] : uppersize[1];
      uppersize[2] = signedindex[2] > uppersize[2] ? signedindex[2] : uppersize[2];
    }

    ++itmask;
    ++itimage;
  }

  typename ImageType::IndexType index;
  index[0] = lowersize[0];
  index[1] = lowersize[1];
  index[2] = lowersize[2];

  typename ImageType::SizeType size;
  size[0] = uppersize[0] - lowersize[0] + 1;
  size[1] = uppersize[1] - lowersize[1] + 1;
  size[2] = uppersize[2] - lowersize[2] + 1;

  itk::ImageRegion<3> cropRegion = itk::ImageRegion<3>(index, size);

  // crop internal mask
  typedef itk::RegionOfInterestImageFilter< ItkUCharImageType, ItkUCharImageType > ROIMaskFilterType;
  typename ROIMaskFilterType::Pointer roi2 = ROIMaskFilterType::New();
  roi2->SetRegionOfInterest(cropRegion);
  roi2->SetInput(m_InternalImageMask3D);
  roi2->Update();
  m_InternalImageMask3D = roi2->GetOutput();

  Image::Pointer tmpImage = Image::New();
  tmpImage->InitializeByItk(m_InternalImageMask3D.GetPointer());
  tmpImage->SetVolume(m_InternalImageMask3D->GetBufferPointer());

  Image::Pointer tmpImage2 = Image::New();
  tmpImage2->InitializeByItk(m_PlanarFigureImage.GetPointer());
  const BaseGeometry *pfImageGeometry3D = tmpImage2->GetGeometry( 0 );

  const BaseGeometry *intImageGeometry3D = tmpImage->GetGeometry( 0 );

  typedef itk::ImageRegionIteratorWithIndex<ItkUCharImageType> IteratorType;
  IteratorType imageIterator (m_InternalImageMask3D, m_InternalImageMask3D->GetRequestedRegion());
  imageIterator.GoToBegin();
  while ( !imageIterator.IsAtEnd() )
  {
    unsigned char val = imageIterator.Value();
    if (val>0)
    {
      itk::Index<3> index = imageIterator.GetIndex();
      Point3D point;
      point[0] = index[0];
      point[1] = index[1];
      point[2] = index[2];

      intImageGeometry3D->IndexToWorld(point, point);
      pfImageGeometry3D->WorldToIndex(point, point);

      point[i0] += 0.5;
      point[i1] += 0.5;

      index[0] = point[0];
      index[1] = point[1];
      index[2] = point[2];

      if (pfImageGeometry3D->IsIndexInside(index))
        m_PlanarFigureImage->SetPixel(index, 1);
    }
    ++imageIterator;
  }

  // Clean up VTK objects
  polyline->Delete();
  extrudeFilter->Delete();
  polyDataToImageStencil->Delete();
  vtkImporter->Delete();
  imageStencilFilter->Delete();
  //vtkExporter->Delete(); // TODO: crashes when outcommented; memory leak??
  delete[] ptIds;
}

void QmitkFiberProcessingView::UpdateGui()
{
  m_Controls->m_FibLabel->setText("<font color='red'>mandatory</font>");
  m_Controls->m_PfLabel->setText("<font color='grey'>needed for extraction</font>");
  m_Controls->m_InputData->setTitle("Please Select Input Data");

  m_Controls->m_RemoveButton->setEnabled(false);

  m_Controls->m_PlanarFigureButtonsFrame->setEnabled(false);
  m_Controls->PFCompoANDButton->setEnabled(false);
  m_Controls->PFCompoORButton->setEnabled(false);
  m_Controls->PFCompoNOTButton->setEnabled(false);

  m_Controls->m_GenerateRoiImage->setEnabled(false);
  m_Controls->m_ExtractFibersButton->setEnabled(false);
  m_Controls->m_ModifyButton->setEnabled(false);

  m_Controls->m_CopyBundle->setEnabled(false);
  m_Controls->m_JoinBundles->setEnabled(false);
  m_Controls->m_SubstractBundles->setEnabled(false);

  // disable alle frames
  m_Controls->m_BundleWeightFrame->setVisible(false);
  m_Controls->m_ExtactionFramePF->setVisible(false);
  m_Controls->m_RemoveDirectionFrame->setVisible(false);
  m_Controls->m_RemoveLengthFrame->setVisible(false);
  m_Controls->m_RemoveCurvatureFrame->setVisible(false);
  m_Controls->m_RemoveByWeightFrame->setVisible(false);
  m_Controls->m_SmoothFibersFrame->setVisible(false);
  m_Controls->m_CompressFibersFrame->setVisible(false);
  m_Controls->m_ColorFibersFrame->setVisible(false);
  m_Controls->m_MirrorFibersFrame->setVisible(false);
  m_Controls->m_MaskExtractionFrame->setVisible(false);
  m_Controls->m_ColorMapBox->setVisible(false);

  bool pfSelected = !m_SelectedPF.empty();
  bool fibSelected = !m_SelectedFB.empty();
  bool multipleFibsSelected = (m_SelectedFB.size()>1);
  bool maskSelected = m_RoiImageNode.IsNotNull();
  bool imageSelected = m_SelectedImage.IsNotNull();

  // toggle visibility of elements according to selected method
  switch ( m_Controls->m_ExtractionMethodBox->currentIndex() )
  {
  case 0:
    m_Controls->m_ExtactionFramePF->setVisible(true);
    break;
  case 1:
    m_Controls->m_MaskExtractionFrame->setVisible(true);
    break;
  }

  switch ( m_Controls->m_RemovalMethodBox->currentIndex() )
  {
  case 0:
    m_Controls->m_RemoveDirectionFrame->setVisible(true);
    if ( fibSelected )
      m_Controls->m_RemoveButton->setEnabled(true);
    break;
  case 1:
    m_Controls->m_RemoveLengthFrame->setVisible(true);
    if ( fibSelected )
      m_Controls->m_RemoveButton->setEnabled(true);
    break;
  case 2:
    m_Controls->m_RemoveCurvatureFrame->setVisible(true);
    if ( fibSelected )
      m_Controls->m_RemoveButton->setEnabled(true);
    break;
  case 3:
    break;
  case 4:
    break;
  case 5:
    m_Controls->m_RemoveByWeightFrame->setVisible(true);
    if ( fibSelected )
      m_Controls->m_RemoveButton->setEnabled(true);
    break;
  }

  switch ( m_Controls->m_ModificationMethodBox->currentIndex() )
  {
  case 0:
    m_Controls->m_SmoothFibersFrame->setVisible(true);
    break;
  case 1:
    m_Controls->m_SmoothFibersFrame->setVisible(true);
    break;
  case 2:
    m_Controls->m_CompressFibersFrame->setVisible(true);
    break;
  case 3:
    m_Controls->m_ColorFibersFrame->setVisible(true);
    m_Controls->m_ColorMapBox->setVisible(true);
    break;
  case 4:
    m_Controls->m_MirrorFibersFrame->setVisible(true);
    if (m_SelectedSurfaces.size()>0)
      m_Controls->m_ModifyButton->setEnabled(true);
    break;
  case 5:
    m_Controls->m_BundleWeightFrame->setVisible(true);
    break;
  case 6:
    m_Controls->m_ColorFibersFrame->setVisible(true);
    break;
  case 7:
    m_Controls->m_ColorFibersFrame->setVisible(true);
    break;
  case 8:
    m_Controls->m_ColorFibersFrame->setVisible(true);
    break;
  }

  // are fiber bundles selected?
  if ( fibSelected )
  {
    m_Controls->m_CopyBundle->setEnabled(true);
    m_Controls->m_ModifyButton->setEnabled(true);
    m_Controls->m_PlanarFigureButtonsFrame->setEnabled(true);
    m_Controls->m_FibLabel->setText(QString(m_SelectedFB.at(0)->GetName().c_str()));

    // one bundle and one planar figure needed to extract fibers
    if (pfSelected && m_Controls->m_ExtractionMethodBox->currentIndex()==0)
    {
      m_Controls->m_InputData->setTitle("Input Data");
      m_Controls->m_PfLabel->setText(QString(m_SelectedPF.at(0)->GetName().c_str()));
      m_Controls->m_ExtractFibersButton->setEnabled(true);
    }

    // more than two bundles needed to join/subtract
    if (multipleFibsSelected)
    {
      m_Controls->m_FibLabel->setText("multiple bundles selected");

      m_Controls->m_JoinBundles->setEnabled(true);
      m_Controls->m_SubstractBundles->setEnabled(true);
    }

    if (maskSelected && m_Controls->m_ExtractionMethodBox->currentIndex()==1)
    {
      m_Controls->m_InputData->setTitle("Input Data");
      m_Controls->m_PfLabel->setText(QString(m_RoiImageNode->GetName().c_str()));
      m_Controls->m_ExtractFibersButton->setEnabled(true);
    }

    if (maskSelected && (m_Controls->m_RemovalMethodBox->currentIndex()==3 || m_Controls->m_RemovalMethodBox->currentIndex()==4) )
    {
      m_Controls->m_InputData->setTitle("Input Data");
      m_Controls->m_PfLabel->setText(QString(m_RoiImageNode->GetName().c_str()));
      m_Controls->m_RemoveButton->setEnabled(true);
    }
  }

  // are planar figures selected?
  if (pfSelected)
  {
    if ( fibSelected || m_SelectedImage.IsNotNull())
      m_Controls->m_GenerateRoiImage->setEnabled(true);

    if (m_SelectedPF.size() > 1)
    {
      m_Controls->PFCompoANDButton->setEnabled(true);
      m_Controls->PFCompoORButton->setEnabled(true);
    }
    else
      m_Controls->PFCompoNOTButton->setEnabled(true);
  }

  // is image selected
  if (imageSelected || maskSelected)
  {
    m_Controls->m_PlanarFigureButtonsFrame->setEnabled(true);
  }
}

void QmitkFiberProcessingView::NodeRemoved(const mitk::DataNode* node )
{
  for (auto fnode: m_SelectedFB)
    if (node == fnode)
    {
      m_SelectedFB.clear();
      break;
    }

  berry::IWorkbenchPart::Pointer nullPart;
  QList<mitk::DataNode::Pointer> nodes;
  OnSelectionChanged(nullPart, nodes);
}

void QmitkFiberProcessingView::NodeAdded(const mitk::DataNode* )
{
  if (!m_Controls->m_InteractiveBox->isChecked())
  {
    berry::IWorkbenchPart::Pointer nullPart;
    QList<mitk::DataNode::Pointer> nodes;
    OnSelectionChanged(nullPart, nodes);
  }
}

void QmitkFiberProcessingView::OnEndInteraction()
{
  if (m_Controls->m_InteractiveBox->isChecked())
    ExtractWithPlanarFigure(true);
}

void QmitkFiberProcessingView::AddObservers()
{
  typedef itk::SimpleMemberCommand< QmitkFiberProcessingView > SimpleCommandType;
  for (auto node : m_SelectedPF)
  {
    mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    if (figure!=nullptr)
    {
      figure->RemoveAllObservers();

      // add observer for event when interaction with figure starts
      SimpleCommandType::Pointer endInteractionCommand = SimpleCommandType::New();
      endInteractionCommand->SetCallbackFunction( this, &QmitkFiberProcessingView::OnEndInteraction);
      m_EndInteractionObserverTag = figure->AddObserver( mitk::EndInteractionPlanarFigureEvent(), endInteractionCommand );
    }
  }
}

void QmitkFiberProcessingView::RemoveObservers()
{
  for (auto node : m_SelectedPF)
  {
    mitk::PlanarFigure* figure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
    if (figure!=nullptr)
      figure->RemoveAllObservers();
  }
}

void QmitkFiberProcessingView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  RemoveObservers();

  //reset existing Vectors containing FiberBundles and PlanarFigures from a previous selection
  std::vector<mitk::DataNode::Pointer>  lastSelectedFB = m_SelectedFB;
  m_SelectedFB.clear();
  m_SelectedPF.clear();
  m_SelectedSurfaces.clear();
  m_SelectedImage = nullptr;
  m_RoiImageNode = nullptr;

  for (auto node: nodes)
  {
    if ( dynamic_cast<mitk::FiberBundle*>(node->GetData()) )
      m_SelectedFB.push_back(node);
    else if (dynamic_cast<mitk::PlanarPolygon*>(node->GetData()) || dynamic_cast<mitk::PlanarFigureComposite*>(node->GetData()) || dynamic_cast<mitk::PlanarCircle*>(node->GetData()))
      m_SelectedPF.push_back(node);
    else if (dynamic_cast<mitk::Image*>(node->GetData()))
    {
      m_SelectedImage = dynamic_cast<mitk::Image*>(node->GetData());
      if (m_SelectedImage->GetDimension()==3)
        m_RoiImageNode = node;
    }
    else if (dynamic_cast<mitk::Surface*>(node->GetData()))
      m_SelectedSurfaces.push_back(dynamic_cast<mitk::Surface*>(node->GetData()));
  }

  // if we perform interactive fiber extraction, we want to avoid auto-selection of the extracted bundle
  if (m_SelectedFB.empty() && m_Controls->m_InteractiveBox->isChecked())
    m_SelectedFB = lastSelectedFB;

  // if no fibers or surfaces are selected, select topmost
  if (m_SelectedFB.empty() && m_SelectedSurfaces.empty())
  {
    int maxLayer = 0;
    itk::VectorContainer<unsigned int, mitk::DataNode::Pointer>::ConstPointer nodes = this->GetDataStorage()->GetAll();
    for (unsigned int i=0; i<nodes->Size(); i++)
      if (dynamic_cast<mitk::FiberBundle*>(nodes->at(i)->GetData()))
      {
        mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(nodes->at(i));
        if (sources->Size()>0)
          continue;

        int layer = 0;
        nodes->at(i)->GetPropertyValue("layer", layer);
        if (layer>=maxLayer)
        {
          maxLayer = layer;
          m_SelectedFB.clear();
          m_SelectedFB.push_back(nodes->at(i));
        }
      }
  }

  // if no plar figure is selected, select topmost
  if (m_SelectedPF.empty())
  {
    int maxLayer = 0;
    itk::VectorContainer<unsigned int, mitk::DataNode::Pointer>::ConstPointer nodes = this->GetDataStorage()->GetAll();
    for (unsigned int i=0; i<nodes->Size(); i++)
      if (dynamic_cast<mitk::PlanarPolygon*>(nodes->at(i)->GetData()) || dynamic_cast<mitk::PlanarFigureComposite*>(nodes->at(i)->GetData()) || dynamic_cast<mitk::PlanarCircle*>(nodes->at(i)->GetData()))
      {
        mitk::DataStorage::SetOfObjects::ConstPointer sources = GetDataStorage()->GetSources(nodes->at(i));
        if (sources->Size()>0)
          continue;

        int layer = 0;
        nodes->at(i)->GetPropertyValue("layer", layer);
        if (layer>=maxLayer)
        {
          maxLayer = layer;
          m_SelectedPF.clear();
          m_SelectedPF.push_back(nodes->at(i));
        }
      }
  }

  AddObservers();
  UpdateGui();
}

void QmitkFiberProcessingView::OnDrawPolygon()
{
  mitk::PlanarPolygon::Pointer figure = mitk::PlanarPolygon::New();
  figure->ClosedOn();
  this->AddFigureToDataStorage(figure, QString("Polygon%1").arg(++m_PolygonCounter));
}

void QmitkFiberProcessingView::OnDrawCircle()
{
  mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
  this->AddFigureToDataStorage(figure, QString("Circle%1").arg(++m_CircleCounter));
}

void QmitkFiberProcessingView::AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name, const char *, mitk::BaseProperty* )
{
  // initialize figure's geometry with empty geometry
  mitk::PlaneGeometry::Pointer emptygeometry = mitk::PlaneGeometry::New();
  figure->SetPlaneGeometry( emptygeometry );

  //set desired data to DataNode where Planarfigure is stored
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(name.toStdString());
  newNode->SetData(figure);
  newNode->SetBoolProperty("planarfigure.3drendering", true);
  newNode->SetBoolProperty("planarfigure.3drendering.fill", true);

  mitk::PlanarFigureInteractor::Pointer figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(newNode->GetDataInteractor().GetPointer());
  if(figureInteractor.IsNull())
  {
    figureInteractor = mitk::PlanarFigureInteractor::New();
    us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
    figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
    figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
    figureInteractor->SetDataNode(newNode);
  }

  // figure drawn on the topmost layer / image
  GetDataStorage()->Add(newNode );

  RemoveObservers();
  for(unsigned int i = 0; i < m_SelectedPF.size(); i++)
    m_SelectedPF[i]->SetSelected(false);

  newNode->SetSelected(true);
  m_SelectedPF.clear();
  m_SelectedPF.push_back(newNode);
  AddObservers();
  UpdateGui();
}

void QmitkFiberProcessingView::ExtractWithPlanarFigure(bool interactive)
{
  if ( m_SelectedFB.empty() || m_SelectedPF.empty() ){
    QMessageBox::information( nullptr, "Warning", "No fibe bundle selected!");
    return;
  }

  try
  {
    std::vector<mitk::DataNode::Pointer> fiberBundles = m_SelectedFB;
    mitk::DataNode::Pointer planarFigure = m_SelectedPF.at(0);
    for (unsigned int i=0; i<fiberBundles.size(); i++)
    {
      mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(fiberBundles.at(i)->GetData());
      mitk::FiberBundle::Pointer extFB = fib->ExtractFiberSubset(planarFigure, GetDataStorage());

      if (interactive && m_Controls->m_InteractiveBox->isChecked())
      {
        if (m_InteractiveNode.IsNull())
        {
          m_InteractiveNode = mitk::DataNode::New();
          QString name("Interactive");
          m_InteractiveNode->SetName(name.toStdString());
          GetDataStorage()->Add(m_InteractiveNode);
        }
        float op = 5.0/sqrt(fib->GetNumFibers());
        float currentOp = 0;
        fiberBundles.at(i)->GetFloatProperty("opacity", currentOp);

        if (currentOp!=op)
        {
          fib->SetFiberColors(255, 255, 255);
          fiberBundles.at(i)->SetFloatProperty("opacity", op);
          fiberBundles.at(i)->SetBoolProperty("Fiber2DfadeEFX", false);
        }
        m_InteractiveNode->SetData(extFB);
      }
      else
      {
        if (extFB->GetNumFibers()<=0)
        {
          QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers.");
          continue;
        }

        mitk::DataNode::Pointer node;
        node = mitk::DataNode::New();
        node->SetData(extFB);
        QString name(fiberBundles.at(i)->GetName().c_str());
        name += "*";
        node->SetName(name.toStdString());
        fiberBundles.at(i)->SetVisibility(false);
        GetDataStorage()->Add(node);
      }
    }
  }
  catch(const std::out_of_range& )
  {
    QMessageBox::warning( nullptr, "Fiber extraction failed", "Did you only create the planar figure, using the circle or polygon button, but forgot to actually place it in the image afterwards? \nAfter creating a planar figure, simply left-click at the desired position in the image or on the tractogram to place it.");
  }
}

void QmitkFiberProcessingView::GenerateAndComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCAnd = mitk::PlanarFigureComposite::New();
  PFCAnd->setOperationType(mitk::PlanarFigureComposite::AND);

  mitk::DataNode::Pointer newPFCNode;
  newPFCNode = mitk::DataNode::New();
  newPFCNode->SetName("AND");
  newPFCNode->SetData(PFCAnd);

  AddCompositeToDatastorage(newPFCNode, m_SelectedPF);
  RemoveObservers();
  m_SelectedPF.clear();
  m_SelectedPF.push_back(newPFCNode);
  AddObservers();
  UpdateGui();
}

void QmitkFiberProcessingView::GenerateOrComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCOr = mitk::PlanarFigureComposite::New();
  PFCOr->setOperationType(mitk::PlanarFigureComposite::OR);

  mitk::DataNode::Pointer newPFCNode;
  newPFCNode = mitk::DataNode::New();
  newPFCNode->SetName("OR");
  newPFCNode->SetData(PFCOr);

  RemoveObservers();
  AddCompositeToDatastorage(newPFCNode, m_SelectedPF);
  m_SelectedPF.clear();
  m_SelectedPF.push_back(newPFCNode);
  UpdateGui();
}

void QmitkFiberProcessingView::GenerateNotComposite()
{
  mitk::PlanarFigureComposite::Pointer PFCNot = mitk::PlanarFigureComposite::New();
  PFCNot->setOperationType(mitk::PlanarFigureComposite::NOT);

  mitk::DataNode::Pointer newPFCNode;
  newPFCNode = mitk::DataNode::New();
  newPFCNode->SetName("NOT");
  newPFCNode->SetData(PFCNot);

  RemoveObservers();
  AddCompositeToDatastorage(newPFCNode, m_SelectedPF);
  m_SelectedPF.clear();
  m_SelectedPF.push_back(newPFCNode);
  AddObservers();
  UpdateGui();
}

void QmitkFiberProcessingView::AddCompositeToDatastorage(mitk::DataNode::Pointer pfc, std::vector<mitk::DataNode::Pointer> children, mitk::DataNode::Pointer parentNode )
{
  pfc->SetSelected(true);
  if (parentNode.IsNotNull())
    GetDataStorage()->Add(pfc, parentNode);
  else
    GetDataStorage()->Add(pfc);

  for (auto child : children)
  {
    if (dynamic_cast<PlanarFigure*>(child->GetData()))
    {
      mitk::DataNode::Pointer newChild;
      newChild = mitk::DataNode::New();
      newChild->SetData(dynamic_cast<PlanarFigure*>(child->GetData()));
      newChild->SetName( child->GetName() );
      newChild->SetBoolProperty("planarfigure.3drendering", true);
      newChild->SetBoolProperty("planarfigure.3drendering.fill", true);

      GetDataStorage()->Add(newChild, pfc);
      GetDataStorage()->Remove(child);
    }
    else if (dynamic_cast<PlanarFigureComposite*>(child->GetData()))
    {
      mitk::DataNode::Pointer newChild;
      newChild = mitk::DataNode::New();
      newChild->SetData(dynamic_cast<PlanarFigureComposite*>(child->GetData()));
      newChild->SetName( child->GetName() );

      std::vector< mitk::DataNode::Pointer > grandChildVector;
      mitk::DataStorage::SetOfObjects::ConstPointer grandchildren = GetDataStorage()->GetDerivations(child);
      for( mitk::DataStorage::SetOfObjects::const_iterator it = grandchildren->begin(); it != grandchildren->end(); ++it )
        grandChildVector.push_back(*it);

      AddCompositeToDatastorage(newChild, grandChildVector, pfc);
      GetDataStorage()->Remove(child);
    }
  }
  UpdateGui();
}

void QmitkFiberProcessingView::CopyBundles()
{
  if ( m_SelectedFB.empty() ){
    QMessageBox::information( nullptr, "Warning", "Select at least one fiber bundle!");
    MITK_WARN("QmitkFiberProcessingView") << "Select at least one fiber bundle!";
    return;
  }

  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    mitk::FiberBundle::Pointer newFib = fib->GetDeepCopy();

    node->SetVisibility(false);
    QString name("");
    name += QString(m_SelectedFB.at(0)->GetName().c_str());
    name += "_copy";

    mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
    fbNode->SetData(newFib);
    fbNode->SetName(name.toStdString());
    fbNode->SetVisibility(true);
    GetDataStorage()->Add(fbNode);
  }
  UpdateGui();
}

void QmitkFiberProcessingView::JoinBundles()
{
  if ( m_SelectedFB.size()<2 ){
    QMessageBox::information( nullptr, "Warning", "Select at least two fiber bundles!");
    MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
    return;
  }

  m_SelectedFB.at(0)->SetVisibility(false);
  mitk::FiberBundle::Pointer newBundle = dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.at(0)->GetData());

  std::vector< mitk::FiberBundle::Pointer > tractograms;
  for (unsigned int i=1; i<m_SelectedFB.size(); i++)
  {
    m_SelectedFB.at(i)->SetVisibility(false);
    tractograms.push_back(dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.at(i)->GetData()));
  }
  newBundle = newBundle->AddBundles(tractograms);

  mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
  fbNode->SetData(newBundle);
  fbNode->SetName("Joined_Tractograms");
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);
  UpdateGui();
}

void QmitkFiberProcessingView::SubstractBundles()
{
  if ( m_SelectedFB.size()<2 ){
    QMessageBox::information( nullptr, "Warning", "Select at least two fiber bundles!");
    MITK_WARN("QmitkFiberProcessingView") << "Select at least two fiber bundles!";
    return;
  }

  mitk::FiberBundle::Pointer newBundle = dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.at(0)->GetData());
  m_SelectedFB.at(0)->SetVisibility(false);
  QString name("");
  name += QString(m_SelectedFB.at(0)->GetName().c_str());
  for (unsigned int i=1; i<m_SelectedFB.size(); i++)
  {
    newBundle = newBundle->SubtractBundle(dynamic_cast<mitk::FiberBundle*>(m_SelectedFB.at(i)->GetData()));
    if (newBundle.IsNull())
      break;
    name += "-"+QString(m_SelectedFB.at(i)->GetName().c_str());
    m_SelectedFB.at(i)->SetVisibility(false);
  }
  if (newBundle.IsNull())
  {
    QMessageBox::information(nullptr, "No output generated:", "The resulting fiber bundle contains no fibers. Did you select the fiber bundles in the correct order? X-Y is not equal to Y-X!");
    return;
  }

  mitk::DataNode::Pointer fbNode = mitk::DataNode::New();
  fbNode->SetData(newBundle);
  fbNode->SetName(name.toStdString());
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);
  UpdateGui();
}

void QmitkFiberProcessingView::ResampleSelectedBundlesSpline()
{
  double factor = this->m_Controls->m_SmoothFibersBox->value();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ResampleSpline(factor);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::ResampleSelectedBundlesLinear()
{
  double factor = this->m_Controls->m_SmoothFibersBox->value();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ResampleLinear(factor);
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::CompressSelectedBundles()
{
  double factor = this->m_Controls->m_ErrorThresholdBox->value();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->Compress(factor);
    fib->ColorFibersByOrientation();
  }
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkFiberProcessingView::DoImageColorCoding()
{
  if (m_Controls->m_ColorMapBox->GetSelectedNode().IsNull())
  {
    QMessageBox::information(nullptr, "Bundle coloring aborted:", "No image providing the scalar values for coloring the selected bundle available.");
    return;
  }

  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ColorFibersByScalarMap(dynamic_cast<mitk::Image*>(m_Controls->m_ColorMapBox->GetSelectedNode()->GetData()), m_Controls->m_FiberOpacityBox->isChecked(), m_Controls->m_NormalizeColorValues->isChecked());
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}


void QmitkFiberProcessingView::DoCurvatureColorCoding()
{
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ColorFibersByCurvature(m_Controls->m_FiberOpacityBox->isChecked(), m_Controls->m_NormalizeColorValues->isChecked());
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}

void QmitkFiberProcessingView::DoLengthColorCoding()
{
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ColorFibersByLength(m_Controls->m_FiberOpacityBox->isChecked(), m_Controls->m_NormalizeColorValues->isChecked());
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}

void QmitkFiberProcessingView::DoWeightColorCoding()
{
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    fib->ColorFibersByFiberWeights(m_Controls->m_FiberOpacityBox->isChecked(), m_Controls->m_NormalizeColorValues->isChecked());
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}

void QmitkFiberProcessingView::MirrorFibers()
{
  unsigned int axis = this->m_Controls->m_MirrorFibersBox->currentIndex();
  for (auto node : m_SelectedFB)
  {
    mitk::FiberBundle::Pointer fib = dynamic_cast<mitk::FiberBundle*>(node->GetData());
    if (m_SelectedImage.IsNotNull())
      fib->SetReferenceGeometry(m_SelectedImage->GetGeometry());
    fib->MirrorFibers(axis);
  }


  for (auto surf : m_SelectedSurfaces)
  {
    vtkSmartPointer<vtkPolyData> poly = surf->GetVtkPolyData();
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();

    for (int i=0; i<poly->GetNumberOfPoints(); i++)
    {
      double* point = poly->GetPoint(i);
      point[axis] *= -1;
      vtkNewPoints->InsertNextPoint(point);
    }
    poly->SetPoints(vtkNewPoints);
    surf->CalculateBoundingBox();
  }

  if (auto renderWindowPart = this->GetRenderWindowPart())
  {
    renderWindowPart->RequestUpdate();
  }
}
