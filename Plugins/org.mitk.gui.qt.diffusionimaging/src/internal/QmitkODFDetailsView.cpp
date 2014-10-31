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
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkODFDetailsView.h"
#include <QTableWidgetItem>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>
#include <mitkOdfNormalizationMethodProperty.h>
#include <QTextEdit>
#include <mitkTensorImage.h>
#include <QMessageBox>
#include <QmitkRenderingManager.h>

#include <mitkImageReadAccessor.h>

const std::string QmitkODFDetailsView::VIEW_ID = "org.mitk.views.odfdetails";

QmitkODFDetailsView::QmitkODFDetailsView()
  : QmitkAbstractView()
  , m_Controls( 0 )
  , m_OdfNormalization(0)
  , m_ImageNode(NULL)
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
  if (m_ImageNode.IsNotNull())
    m_ImageNode->RemoveObserver( m_PropertyObserverTag );
}

void QmitkODFDetailsView::Visible()
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if (renderWindow)
  {
    {
      mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("axial"))->GetSliceNavigationController();
      itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
      command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
      m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
      mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("sagittal"))->GetSliceNavigationController();
      itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
      command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
      m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
      mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("coronal"))->GetSliceNavigationController();
      itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
      command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
      m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }
  }
}

void QmitkODFDetailsView::Hidden()
{
  mitk::IRenderWindowPart* renderWindow = this->GetRenderWindowPart();

  if (renderWindow)
  {
    mitk::SliceNavigationController* slicer = renderWindow->GetQmitkRenderWindow(QString("axial"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag1);
    slicer = renderWindow->GetQmitkRenderWindow(QString("sagittal"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag2);
    slicer = renderWindow->GetQmitkRenderWindow(QString("coronal"))->GetSliceNavigationController();
    slicer->RemoveObserver(m_SliceObserverTag3);
  }
}

void QmitkODFDetailsView::Activated()
{
}

void QmitkODFDetailsView::Deactivated()
{
}

void QmitkODFDetailsView::SetFocus()
{
  this->m_Controls->m_ODFRenderWidget->setFocus();
}

void QmitkODFDetailsView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkODFDetailsViewControls;
    m_Controls->setupUi( parent );
    m_Controls->m_OdfBox->setVisible(false);
    m_Controls->m_ODFRenderWidget->setVisible(false);
  }
}

void QmitkODFDetailsView::OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes )
{
  if (m_ImageNode.IsNotNull())
    m_ImageNode->RemoveObserver( m_PropertyObserverTag );

  m_Controls->m_InputData->setTitle("Please Select Input Data");
  m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");

  m_ImageNode = NULL;

  // iterate selection
  foreach( mitk::DataNode::Pointer node, nodes )
  {

    if( node.IsNotNull() && (dynamic_cast<mitk::QBallImage*>(node->GetData()) || dynamic_cast<mitk::TensorImage*>(node->GetData())) )
    {
      m_Controls->m_InputImageLabel->setText(node->GetName().c_str());
      m_ImageNode = node;
    }
  }

  UpdateOdf();
  if (m_ImageNode.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkODFDetailsView>::Pointer command = itk::ReceptorMemberCommand<QmitkODFDetailsView>::New();
    command->SetCallbackFunction( this, &QmitkODFDetailsView::OnSliceChanged );
    m_PropertyObserverTag = m_ImageNode->AddObserver( itk::ModifiedEvent(), command );

    m_Controls->m_InputData->setTitle("Input Data");
  }
}

void QmitkODFDetailsView::UpdateOdf()
{
  try
  {
    m_Controls->m_OverviewBox->setVisible(true);
    if (m_ImageNode.IsNull() || !this->GetRenderWindowPart())
    {
      m_Controls->m_ODFRenderWidget->setVisible(false);
      m_Controls->m_OdfBox->setVisible(false);
      m_Controls->m_OverviewBox->setVisible(false);
      return;
    }

    // restore the input image label ( needed in case the last run resulted into an exception )
    m_Controls->m_InputImageLabel->setText(m_ImageNode->GetName().c_str());

    // ODF Normalization Property
    mitk::OdfNormalizationMethodProperty* nmp = dynamic_cast<mitk::OdfNormalizationMethodProperty*>(m_ImageNode->GetProperty( "Normalization" ));
    if(nmp)
      m_OdfNormalization = nmp->GetNormalization();

    m_TemplateOdf = itk::OrientationDistributionFunction<float,QBALL_ODFSIZE>::GetBaseMesh();
    m_OdfTransform = vtkSmartPointer<vtkTransform>::New();
    m_OdfTransform->Identity();
    m_OdfVals = vtkSmartPointer<vtkDoubleArray>::New();
    m_OdfSource = vtkSmartPointer<vtkOdfSource>::New();
    itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf;

    mitk::Point3D world = this->GetRenderWindowPart()->GetSelectedPosition();
    mitk::Point3D index;
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
    unsigned int *img_dimension = img->GetDimensions();
    img->GetGeometry()->WorldToIndex(world, index);

    float sum = 0;
    float max = itk::NumericTraits<float>::NonpositiveMin();
    float min = itk::NumericTraits<float>::max();
    QString values;
    QString overviewText;

    // check if dynamic_cast successfull and if the crosshair position is inside of the geometry of the ODF data
    // otherwise possible crash for a scenario with multiple nodes
    if (dynamic_cast<mitk::QBallImage*>(m_ImageNode->GetData()) && ( m_ImageNode->GetData()->GetGeometry()->IsInside(world) ) )
    {
      m_Controls->m_ODFRenderWidget->setVisible(true);
      m_Controls->m_OdfBox->setVisible(true);

      try
      {
        const mitk::QBallImage* qball_image = dynamic_cast< mitk::QBallImage* >( m_ImageNode->GetData() );

        // get access to the qball image data with explicitely allowing exceptions if memory locked
        mitk::ImageReadAccessor readAccess( qball_image, qball_image->GetVolumeData(0), mitk::ImageAccessorBase::ExceptionIfLocked );
        const float* qball_cPtr = static_cast< const float*>(readAccess.GetData());

        OdfVectorImgType::IndexType ind;
        ind[0] = (int)(index[0]+0.5);
        ind[1] = (int)(index[1]+0.5);
        ind[2] = (int)(index[2]+0.5);

        // pixel size = QBALL_ODFSIZE
        // position offset = standard offset
        unsigned int offset_to_data = QBALL_ODFSIZE * (ind[2] * img_dimension[1] * img_dimension[0] + ind[1] * img_dimension[0] + ind[0]);
        const float *pixel_data = qball_cPtr + offset_to_data;

        for (int i=0; i<QBALL_ODFSIZE; i++)
        {
          float val = pixel_data[i];
          odf.SetNthComponent(i, val);
          values += QString::number(i)+": "+QString::number(val)+"\n";
          sum += val;
          if (val>max)
            max = val;
          if (val<min)
            min = val;
        }
        float mean = sum/QBALL_ODFSIZE;

        QString pos = QString::number(ind[0])+", "+QString::number(ind[1])+", "+QString::number(ind[2]);
        overviewText += "Coordinates: "+pos+"\n";
        overviewText += "GFA: "+QString::number(odf.GetGeneralizedFractionalAnisotropy())+"\n";
        overviewText += "Sum: "+QString::number(sum)+"\n";
        overviewText += "Mean: "+QString::number(mean)+"\n";
        overviewText += "Min: "+QString::number(min)+"\n";
        overviewText += "Max: "+QString::number(max)+"\n";
        vnl_vector_fixed<double, 3> pd = odf.GetDirection(odf.GetPrincipleDiffusionDirection());
        overviewText += "Main Diffusion:\n     "+QString::number(pd[0])+"\n     "+QString::number(pd[1])+"\n     "+QString::number(pd[2])+"\n";

        m_Controls->m_OdfValuesTextEdit->setText(values);
        m_Controls->m_OverviewTextEdit->setVisible(true);
      }
      catch( mitk::Exception &e )
      {
        MITK_WARN << "LOCKED : " << e.what();
        m_Controls->m_ODFRenderWidget->setVisible(false);
        m_Controls->m_OdfBox->setVisible(false);
        m_Controls->m_OverviewTextEdit->setVisible(false);

        // reset the selection
        m_Controls->m_InputImageLabel->setText("<font color='green'>Click image to restore rendering!</font>");
      }
    }
    else if (dynamic_cast<mitk::TensorImage*>(m_ImageNode->GetData()) && ( m_ImageNode->GetData()->GetGeometry()->IsInside(world) ) )
    {
      m_Controls->m_ODFRenderWidget->setVisible(true);
      m_Controls->m_OdfBox->setVisible(false);


      const mitk::TensorImage* qball_image = dynamic_cast< mitk::TensorImage*>(m_ImageNode->GetData());

      // pixel access block
      try
      {
        // get access to the qball image data with explicitely allowing exceptions if memory locked
        mitk::ImageReadAccessor readAccess( qball_image, qball_image->GetVolumeData(0), mitk::ImageAccessorBase::ExceptionIfLocked );
        const float* qball_cPtr = static_cast< const float*>(readAccess.GetData());

        TensorImageType::IndexType ind;
        ind[0] = (int)(index[0]+0.5);
        ind[1] = (int)(index[1]+0.5);
        ind[2] = (int)(index[2]+0.5);

        // 6 - tensorsize
        // remaining computation - standard offset
        unsigned int offset_to_data = 6 * (ind[2] * img_dimension[1] * img_dimension[0] + ind[1] * img_dimension[0] + ind[0]);
        const float *pixel_data = qball_cPtr + offset_to_data;

        float tensorelems[6] = {
          *(pixel_data    ),
          *(pixel_data + 1),
          *(pixel_data + 2),
          *(pixel_data + 3),
          *(pixel_data + 4),
          *(pixel_data + 5),
        };

        itk::DiffusionTensor3D<float> tensor(tensorelems);
        odf.InitFromTensor(tensor);

        /** Array of eigen-values. */
        typedef itk::FixedArray<float, 3> EigenValuesArrayType;
        /** Matrix of eigen-vectors. */
        typedef itk::Matrix<float, 3, 3> MatrixType;
        typedef itk::Matrix<float, 3, 3> EigenVectorsMatrixType;

        EigenValuesArrayType eigenValues;
        EigenVectorsMatrixType eigenvectors;

        QString pos = QString::number(ind[0])+", "+QString::number(ind[1])+", "+QString::number(ind[2]);
        overviewText += "Coordinates: "+pos+"\n";
        overviewText += "FA: "+QString::number(tensor.GetFractionalAnisotropy())+"\n";
        overviewText += "RA: "+QString::number(tensor.GetRelativeAnisotropy())+"\n";
        overviewText += "Trace: "+QString::number(tensor.GetTrace())+"\n";
        tensor.ComputeEigenAnalysis(eigenValues,eigenvectors);
        overviewText += "Eigenvalues:\n     "+QString::number(eigenValues[2])+"\n     "+QString::number(eigenValues[1])+"\n     "+QString::number(eigenValues[0])+"\n";
        overviewText += "Main Diffusion:\n     "+QString::number(eigenvectors(2, 0))+"\n     "+QString::number(eigenvectors(2, 1))+"\n     "+QString::number(eigenvectors(2, 2))+"\n";
        overviewText += "Values:\n     "+QString::number(tensorelems[0])+"\n     "+QString::number(tensorelems[1])+"\n     "+QString::number(tensorelems[2])+"\n     "+QString::number(tensorelems[3])+"\n     "+QString::number(tensorelems[4])+"\n     "+QString::number(tensorelems[5])+"\n     "+"\n";
        m_Controls->m_OverviewTextEdit->setVisible(true);
      }
      // end pixel access block
      catch(mitk::Exception &e )
      {
        MITK_WARN << "LOCKED : " << e.what();
        m_Controls->m_ODFRenderWidget->setVisible(false);
        m_Controls->m_OdfBox->setVisible(false);
        m_Controls->m_OverviewTextEdit->setVisible(false);

        // reset the selection
        m_Controls->m_InputImageLabel->setText("<font color='green'>Click image to restore rendering!</font>");
      }
    }
    else
    {
      m_Controls->m_ODFRenderWidget->setVisible(false);
      m_Controls->m_OdfBox->setVisible(false);
      overviewText += "Please reinit image geometry.\n";
    }

    // proceed only if the render widget is visible which indicates that the
    // predecessing computations were successfull
    if( m_Controls->m_ODFRenderWidget->isVisible() )
    {
      m_Controls->m_ODFDetailsWidget->SetParameters(odf);

      switch(m_OdfNormalization)
      {
      case 0:
        odf = odf.MinMaxNormalize();
        break;
      case 1:
        odf = odf.MaxNormalize();
        break;
      case 2:
        odf = odf.MaxNormalize();
        break;
      default:
        odf = odf.MinMaxNormalize();
      }

      m_Controls->m_ODFRenderWidget->GenerateODF(odf);
      m_Controls->m_OverviewTextEdit->setText(overviewText.toStdString().c_str());
    }
  }
  catch(...)
  {
    QMessageBox::critical(0, "Error", "Data could not be analyzed. The image might be corrupted.");
  }
}

void QmitkODFDetailsView::OnSliceChanged(const itk::EventObject& /*e*/)
{
  UpdateOdf();
}
