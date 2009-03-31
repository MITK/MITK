#include "QmitkVolumeRenderingView.h"

#include <QComboBox>

#include <mitkProperties.h>
#include <mitkNodePredicateDataType.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkHistogramGenerator.h"

void QmitkVolumeRenderingView::CreateQtPartControl(QWidget* parent)
{
  QVBoxLayout* layout = new QVBoxLayout(parent);

  m_DataStorageComboBox = new QmitkDataStorageComboBox( this->GetDataStorage(),
                                                        new mitk::NodePredicateDataType("Image"),
                                                        parent );
 layout->addWidget(m_DataStorageComboBox);

  m_TransferFunctionWidget = new QmitkTransferFunctionWidget(parent);
  layout->addWidget(m_TransferFunctionWidget, 1);

  if (m_DataStorageComboBox->count() > 0)
    this->comboIndexChanged(0);

  connect(m_DataStorageComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboIndexChanged(int)));

  // !!! Transfer Function Changed
  connect( m_TransferFunctionWidget, SIGNAL( SignalTransferFunctionModeChanged( int ) ), this, SLOT( OnTransferFunctionModeChanged( int ) ) );
}

void QmitkVolumeRenderingView::comboIndexChanged(int index)
{
  mitk::DataTreeNode::Pointer node = m_DataStorageComboBox->GetSelectedNode();

  node->SetProperty("volumerendering", mitk::BoolProperty::New(true));

  m_TransferFunctionWidget->SetDataTreeNode(node);
}

void QmitkVolumeRenderingView::OnTransferFunctionModeChanged( int mode )
{
  //set Transfer Function to Data Node
  mitk::DataTreeNode::Pointer node = m_DataStorageComboBox->GetSelectedNode();
 
  if( node )
  {
      //Create new Transfer Function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();

      //-- Create Histogramm (for min / max Calculation)
      if( mode == 8 ) 
      {
        if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
          tf->InitializeHistogram( image );
      }

      // -- Creat new TransferFunction
      tf->SetTransferFunctionMode( mode );

      node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
      //m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
      m_TransferFunctionWidget->SetDataTreeNode(node);
  }
}

void QmitkVolumeRenderingView::SetFocus()
{

}

QmitkVolumeRenderingView::~QmitkVolumeRenderingView()
{

}

