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

#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
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
#include <mitkShImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkDiffusionFunctionCollection.h>
#include <vtkRenderWindowInteractor.h>

const std::string QmitkODFDetailsView::VIEW_ID = "org.mitk.views.odfdetails";

QmitkODFDetailsView::QmitkODFDetailsView()
  : QmitkAbstractView()
  , m_Controls(nullptr)
  , m_OdfNormalization(0)
  , m_ImageNode(nullptr)
{

}

QmitkODFDetailsView::~QmitkODFDetailsView()
{
}

void QmitkODFDetailsView::Activated()
{
}

void QmitkODFDetailsView::Deactivated()
{
}

void QmitkODFDetailsView::Visible()
{

}

void QmitkODFDetailsView::Hidden()
{

}

void QmitkODFDetailsView::SetFocus()
{
  this->m_Controls->m_OverviewTextEdit->setFocus();
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

    m_SliceChangeListener.RenderWindowPartActivated(this->GetRenderWindowPart());
    connect(&m_SliceChangeListener, SIGNAL(SliceChanged()), this, SLOT(OnSliceChanged()));
  }
}

void QmitkODFDetailsView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  m_Controls->m_InputData->setTitle("Please Select Input Data");
  m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");

  m_ImageNode = nullptr;

  // iterate selection
  for (mitk::DataNode::Pointer node: nodes)
  {
    if( node.IsNotNull() && (dynamic_cast<mitk::OdfImage*>(node->GetData()) || dynamic_cast<mitk::TensorImage*>(node->GetData()) || dynamic_cast<mitk::ShImage*>(node->GetData())) )
    {
      m_Controls->m_InputImageLabel->setText(node->GetName().c_str());
      m_ImageNode = node;
    }
  }

  UpdateOdf();
  if (m_ImageNode.IsNotNull())
  {
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

    bool dir_color = false;
    m_ImageNode->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.ColourisationModeBit", dir_color );

    bool toggle_tensor = false;
    m_ImageNode->GetBoolProperty( "DiffusionCore.Rendering.OdfVtkMapper.SwitchTensorView", toggle_tensor );

    mitk::Point3D world = this->GetRenderWindowPart()->GetSelectedPosition();
    mitk::Point3D cont_index;
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_ImageNode->GetData());
    img->GetGeometry()->WorldToIndex(world, cont_index);
    itk::Index<3> index;
    index[0] = static_cast<int>(cont_index[0]+0.5);
    index[1] = static_cast<int>(cont_index[1]+0.5);
    index[2] = static_cast<int>(cont_index[2]+0.5);

    float sum = 0;
    float max = itk::NumericTraits<float>::NonpositiveMin();
    float min = itk::NumericTraits<float>::max();
    QString values;
    QString overviewText;

    // check if dynamic_cast successfull and if the crosshair position is inside of the geometry of the ODF data
    // otherwise possible crash for a scenario with multiple nodes
    if (dynamic_cast<mitk::OdfImage*>(m_ImageNode->GetData()) && ( m_ImageNode->GetData()->GetGeometry()->IsInside(world) ) )
    {
      itk::OrientationDistributionFunction<float, ODF_SAMPLING_SIZE> odf;
      m_Controls->m_ODFRenderWidget->setVisible(true);
      m_Controls->m_OdfBox->setVisible(true);

      try
      {
        const mitk::OdfImage* Odf_image = dynamic_cast< mitk::OdfImage* >( m_ImageNode->GetData() );

        mitk::ImagePixelReadAccessor<mitk::OdfImage::PixelType, 3> pixel_read(Odf_image, Odf_image->GetVolumeData(0));
        auto pixel_data = pixel_read.GetPixelByIndex(index);

        for (unsigned int i=0; i<odf.GetNumberOfComponents(); i++)
        {
          auto val = pixel_data[i];
          odf[i] = val;
          values += QString::number(i)+": "+QString::number(static_cast<double>(val))+"\n";
          sum += val;
          if (val>max)
            max = val;
          if (val<min)
            min = val;
        }
        float mean = sum/odf.GetNumberOfComponents();

        float stdev = 0;
        for (unsigned int i=0; i<odf.GetNumberOfComponents(); i++)
        {
          float diff = odf[i] - mean;
          stdev += diff*diff;
        }
        stdev = std::sqrt(stdev/(odf.GetNumberOfComponents()-1));

        QString pos = QString::number(index[0])+", "+QString::number(index[1])+", "+QString::number(index[2]);
        overviewText += "Coordinates: "+pos+"\n";
        overviewText += "GFA: "+QString::number(static_cast<double>(odf.GetGeneralizedFractionalAnisotropy()))+"\n";
        overviewText += "Sum: "+QString::number(static_cast<double>(sum))+"\n";
        overviewText += "Mean: "+QString::number(static_cast<double>(mean))+"\n";
        overviewText += "Stdev: "+QString::number(static_cast<double>(stdev))+"\n";
        overviewText += "Min: "+QString::number(static_cast<double>(min))+"\n";
        overviewText += "Max: "+QString::number(static_cast<double>(max))+"\n";
        vnl_vector_fixed<double, 3> pd = odf.GetDirection(odf.GetPrincipalDiffusionDirectionIndex());
        overviewText += "Main Diffusion:\n     "+QString::number(pd[0])+"\n     "+QString::number(pd[1])+"\n     "+QString::number(pd[2])+"\n";

        m_Controls->m_OdfValuesTextEdit->setText(values);
        m_Controls->m_OverviewTextEdit->setVisible(true);
        m_Controls->m_ODFDetailsWidget->SetParameters<ODF_SAMPLING_SIZE>(odf);
        m_Controls->m_ODFRenderWidget->GenerateODF<ODF_SAMPLING_SIZE>(odf, m_OdfNormalization, dir_color);
        m_Controls->m_OverviewTextEdit->setText(overviewText.toStdString().c_str());
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
      itk::OrientationDistributionFunction<float, 5000> odf;
      m_Controls->m_ODFRenderWidget->setVisible(true);
      m_Controls->m_OdfBox->setVisible(false);
      const mitk::TensorImage* Odf_image = dynamic_cast< mitk::TensorImage*>(m_ImageNode->GetData());

      // pixel access block
      try
      {
        mitk::ImagePixelReadAccessor<mitk::TensorImage::PixelType, 3> pixel_read(Odf_image, Odf_image->GetVolumeData(0));
        auto tensorelems = pixel_read.GetPixelByIndex(index);

        TensorPixelType tensor(tensorelems);
        if (!toggle_tensor)
          odf.InitFromTensor(tensor);
        else
        {
          odf.InitFromEllipsoid(tensor);
          m_OdfNormalization = 1;
        }

        /** Array of eigen-values. */
        typedef itk::FixedArray<float, 3> EigenValuesArrayType;
        /** Matrix of eigen-vectors. */
        typedef itk::Matrix<float, 3, 3> EigenVectorsMatrixType;

        EigenValuesArrayType eigenValues;
        EigenVectorsMatrixType eigenvectors;

        QString pos = QString::number(index[0])+", "+QString::number(index[1])+", "+QString::number(index[2]);
        overviewText += "Coordinates: "+pos+"\n";
        overviewText += "FA: "+QString::number(tensor.GetFractionalAnisotropy())+"\n";
        overviewText += "RA: "+QString::number(tensor.GetRelativeAnisotropy())+"\n";
        overviewText += "Trace: "+QString::number(tensor.GetTrace())+"\n";
        tensor.ComputeEigenAnalysis(eigenValues,eigenvectors);
        overviewText += "Eigenvalues:\n     "+QString::number(static_cast<double>(eigenValues[2]))+"\n     "+QString::number(static_cast<double>(eigenValues[1]))+"\n     "+QString::number(static_cast<double>(eigenValues[0]))+"\n";
        overviewText += "Main Diffusion:\n     "+QString::number(static_cast<double>(eigenvectors(2, 0)))+"\n     "+QString::number(static_cast<double>(eigenvectors(2, 1)))+"\n     "+QString::number(static_cast<double>(eigenvectors(2, 2)))+"\n";
        overviewText += "Values:\n     "+QString::number(static_cast<double>(tensorelems[0]))+"\n     "+QString::number(static_cast<double>(tensorelems[1]))+"\n     "+QString::number(static_cast<double>(tensorelems[2]))+"\n     "+QString::number(static_cast<double>(tensorelems[3]))+"\n     "+QString::number(static_cast<double>(tensorelems[4]))+"\n     "+QString::number(static_cast<double>(tensorelems[5]))+"\n     "+"\n";
        m_Controls->m_OverviewTextEdit->setVisible(true);

        m_Controls->m_ODFDetailsWidget->SetParameters<5000>(odf);
        m_Controls->m_ODFRenderWidget->GenerateODF<5000>(odf, m_OdfNormalization, dir_color);
        m_Controls->m_OverviewTextEdit->setText(overviewText.toStdString().c_str());
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
    else if (dynamic_cast<mitk::ShImage*>(m_ImageNode->GetData()) && ( m_ImageNode->GetData()->GetGeometry()->IsInside(world) ) )
    {
      itk::OrientationDistributionFunction<float, 5000> odf;
      m_Controls->m_ODFRenderWidget->setVisible(true);
      m_Controls->m_OdfBox->setVisible(false);

      mitk::ShImage::Pointer Odf_image = dynamic_cast< mitk::ShImage*>(m_ImageNode->GetData());
      mitk::Image::Pointer image = dynamic_cast< mitk::ShImage*>(m_ImageNode->GetData());
      QString coeff_string = "";
      switch (Odf_image->ShOrder())
      {
      case 2:
      {
        auto img = mitk::convert::GetItkShFromShImage<6>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      case 4:
      {
        auto img = mitk::convert::GetItkShFromShImage<15>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      case 6:
      {
        auto img = mitk::convert::GetItkShFromShImage<28>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      case 8:
      {
        auto img = mitk::convert::GetItkShFromShImage<45>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      case 10:
      {
        auto img = mitk::convert::GetItkShFromShImage<66>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      case 12:
      {
        auto img = mitk::convert::GetItkShFromShImage<91>(image);
        auto coeffs = img->GetPixel(index);
        for (unsigned int i=0; i<coeffs.Size(); ++i)
          coeff_string += QString::number(static_cast<double>(coeffs[i])) + " ";
        mitk::sh::SampleOdf(coeffs.GetVnlVector(), odf);
        break;
      }
      default :
        mitkThrow() << "SH order larger 12 not supported";
      }

      for (unsigned int i=0; i<odf.GetNumberOfComponents(); i++)
      {
        auto val = odf[i];
            values += QString::number(i)+": "+QString::number(static_cast<double>(val))+"\n";
        sum += val;
        if (val>max)
          max = val;
        if (val<min)
          min = val;
      }
      float mean = sum/odf.GetNumberOfComponents();

      float stdev = 0;
      for (unsigned int i=0; i<odf.GetNumberOfComponents(); i++)
      {
        float val = odf[i];
        float diff = val - mean;
        stdev += diff*diff;
      }
      stdev = std::sqrt(stdev/(odf.GetNumberOfComponents()-1));

      QString pos = QString::number(index[0])+", "+QString::number(index[1])+", "+QString::number(index[2]);
      overviewText += "Coordinates: "+pos+"\n";
      overviewText += "SH order: "+QString::number(static_cast<double>(Odf_image->ShOrder()))+"\n";
      overviewText += "Num. Coefficients: "+QString::number(static_cast<double>(Odf_image->NumCoefficients()))+"\n";
      overviewText += "Coefficients: "+coeff_string+"\n";
      overviewText += "GFA: "+QString::number(static_cast<double>(odf.GetGeneralizedFractionalAnisotropy()))+"\n";
      overviewText += "Sum: "+QString::number(static_cast<double>(sum))+"\n";
      overviewText += "Mean: "+QString::number(static_cast<double>(mean))+"\n";
      overviewText += "Stdev: "+QString::number(static_cast<double>(stdev))+"\n";
      overviewText += "Min: "+QString::number(static_cast<double>(min))+"\n";
      overviewText += "Max: "+QString::number(static_cast<double>(max))+"\n";
      vnl_vector_fixed<double, 3> pd = odf.GetDirection(odf.GetPrincipalDiffusionDirectionIndex());
      overviewText += "Main Diffusion:\n     "+QString::number(pd[0])+"\n     "+QString::number(pd[1])+"\n     "+QString::number(pd[2])+"\n";

      m_Controls->m_OdfValuesTextEdit->setText(values);
      m_Controls->m_OverviewTextEdit->setVisible(true);
      m_Controls->m_ODFDetailsWidget->SetParameters<5000>(odf);
      m_Controls->m_ODFRenderWidget->GenerateODF<5000>(odf, m_OdfNormalization, dir_color);
      m_Controls->m_OverviewTextEdit->setText(overviewText.toStdString().c_str());
    }
    else
    {
      m_Controls->m_ODFRenderWidget->setVisible(false);
      m_Controls->m_OdfBox->setVisible(false);
      overviewText += "Please reinit image geometry.\n";
    }


  }
  catch(...)
  {
    QMessageBox::critical(nullptr, "Error", "Data could not be analyzed. The image might be corrupted.");
  }
}

void QmitkODFDetailsView::OnSliceChanged()
{
  UpdateOdf();
}
