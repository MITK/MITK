/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkODFDetailsView.h"
#include <QmitkStdMultiWidget.h>
#include <QTableWidgetItem>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>

const std::string QmitkODFDetailsView::VIEW_ID = "org.mitk.views.odfdetails";

QmitkODFDetailsView::QmitkODFDetailsView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
{
  m_VtkActor = vtkActor::New();
  m_VtkMapper = vtkPolyDataMapper::New();
  m_Renderer = vtkRenderer::New();
  m_VtkRenderWindow = vtkRenderWindow::New();
  m_RenderWindowInteractor = vtkRenderWindowInteractor::New();
  m_Camera = vtkCamera::New();
  m_VtkRenderWindow->SetSize(300,300);
}

QmitkODFDetailsView::~QmitkODFDetailsView()
{
  mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
  slicer->RemoveObserver(this->m_SliceObserverTag1);

  slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
  slicer->RemoveObserver(this->m_SliceObserverTag2);

  slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
  slicer->RemoveObserver(this->m_SliceObserverTag3);
}

void QmitkODFDetailsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkODFDetailsViewControls;
    m_Controls->setupUi( parent );

    this->m_Controls->m_ODFValueTable->setColumnCount(QBALL_ODFSIZE);
    this->m_Controls->m_ODFValueTable->setRowCount(1);
  }
}

void QmitkODFDetailsView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
    command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
    m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
    command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
    m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
    command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
    m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

}

void QmitkODFDetailsView::StdMultiWidgetNotAvailable()
{

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag1 );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag2 );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag3 );
  }

  m_MultiWidget = NULL;
}

void QmitkODFDetailsView::OnSliceChanged(const itk::EventObject& /*e*/)
{
  m_Values.clear();
  std::vector<mitk::DataNode*> nodes = this->GetDataManagerSelection();
  if (nodes.empty()) return;
  if (!nodes.front()) return;
  if (nodes.size()>1) return;

  mitk::QBallImage::Pointer img =
    dynamic_cast<mitk::QBallImage*>(
    nodes.front()->GetData());

  if (!img) return;

  if (!m_MultiWidget) return;

  m_TemplateOdf = itk::OrientationDistributionFunction<float,QBALL_ODFSIZE>::GetBaseMesh();
  m_OdfTransform = vtkSmartPointer<vtkTransform>::New();
  m_OdfTransform->Identity();
  m_OdfVals = vtkSmartPointer<vtkDoubleArray>::New();
  m_OdfSource = vtkSmartPointer<vtkOdfSource>::New();
  itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf;

  mitk::Point3D world = m_MultiWidget->GetCrossPosition();
  mitk::Point3D index;
  img->GetTimeSlicedGeometry()->WorldToIndex(world, index);

  OdfVectorImgType::Pointer itkQBallImage = OdfVectorImgType::New();
  mitk::CastToItkImage<OdfVectorImgType>(img, itkQBallImage);

  float sum = 0;
  float max = itk::NumericTraits<float>::min();
  float min = itk::NumericTraits<float>::max();
  std::ostringstream text;
  if( text.good() )
  {
    m_Values.clear();
    OdfVectorImgType::IndexType ind;
    ind[0] = (int)(index[0]+0.5);
    ind[1] = (int)(index[1]+0.5);
    ind[2] = (int)(index[2]+0.5);

    OdfVectorImgType::PixelType pixel = itkQBallImage->GetPixel(ind);


    for (int i=0; i<QBALL_ODFSIZE; i++){
      odf.SetNthComponent(i, pixel[i]);
      text.clear();
      text.str("");
      text << pixel[i];
      sum += pixel[i];
      if (pixel[i]>max)
        max = pixel[i];
      if (pixel[i]<min)
        min = pixel[i];

      QTableWidgetItem* item  = new QTableWidgetItem(text.str().c_str());
      this->m_Controls->m_ODFValueTable->setItem(0,i,item);
      this->m_Controls->m_ODFValueTable->update();

      m_Values.push_back(pixel[i]);
    }
    float mean = sum/QBALL_ODFSIZE;

    text.clear();
    text.str("");
    text << sum;
    this->m_Controls->m_Sum->setText(text.str().c_str());
    text.clear();
    text.str("");
    text << mean ;
    this->m_Controls->m_Mean->setText(text.str().c_str());
    text.clear();
    text.str("");
    text << ind[0] << ", " << ind[1] << ", " << ind[2];
    this->m_Controls->m_Pos->setText(text.str().c_str());
    text.clear();
    text.str("");
    text << min;
    this->m_Controls->m_Min->setText(text.str().c_str());
    text.clear();
    text.str("");
    text << max;
    this->m_Controls->m_Max->setText(text.str().c_str());
    text.clear();
    text.str("");
    text << odf.GetGeneralizedFractionalAnisotropy();
    this->m_Controls->m_GFA->setText(text.str().c_str());

    m_Controls->m_ODFDetailsWidget->SetParameters(m_Values);
    m_Controls->m_ODFRenderWidget->GenerateODF(m_Values);
  }
}
