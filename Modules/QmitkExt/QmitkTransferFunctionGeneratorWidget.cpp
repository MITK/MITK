#include "QmitkTransferFunctionGeneratorWidget.h"

#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingManager.h>

#include <QFileDialog>

#include <SceneSerializationExports.h>
namespace mitk
{
extern bool SceneSerialization_EXPORT SerializeTransferFunction( const char * , TransferFunction::Pointer  );
extern TransferFunction::Pointer  SceneSerialization_EXPORT DeserializeTransferFunction( const char * );
 };
 
QmitkTransferFunctionGeneratorWidget::QmitkTransferFunctionGeneratorWidget(QWidget* parent,
    Qt::WindowFlags f) :
  QWidget(parent, f)
{
  histoGramm = NULL;

  this->setupUi(this);

  // LevelWindow Tab
  {
    connect( m_CrossLevelWindow, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( OnDeltaLevelWindow( int, int ) ) );
  }
  
  // Threshold Tab
  {
    connect( m_CrossThreshold, SIGNAL( SignalDeltaMove( int, int ) ), this, SLOT( OnDeltaThreshold( int, int ) ) );
    thDelta = 1;
  }
   
  // Presets Tab
  {
    m_TransferFunctionComboBox->insertItem( "choose a Transferfunction preset ...");
    
    m_TransferFunctionComboBox->insertItem( "CT Generic");
    m_TransferFunctionComboBox->insertItem( "CT Black & White");
    m_TransferFunctionComboBox->insertItem( "CT Cardiac");
    m_TransferFunctionComboBox->insertItem( "CT Bone");
    m_TransferFunctionComboBox->insertItem( "CT Bone (with Gradient)");
    
    connect( m_TransferFunctionComboBox, SIGNAL( activated( int ) ), this, SLOT( OnMitkInternalPreset( int ) ) );

    connect( m_SavePreset, SIGNAL( clicked() ), this, SLOT( OnSavePreset() ) );
    
    connect( m_LoadPreset, SIGNAL( clicked() ), this, SLOT( OnLoadPreset() ) );
  }
  
  
}


void QmitkTransferFunctionGeneratorWidget::OnSavePreset( )
{
  if(tfpToChange.IsNull())
    return;
    
  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  std::string fileName;

  QString fileNames = QFileDialog::getSaveFileName( this,"Choose a filename to save the transferfunction",".", "Transferfunction (*.xml)" );
  

  fileName=fileNames.ascii();
 
  LOG_INFO << "Saving Transferfunction under path: " << fileName;

  mitk::SerializeTransferFunction( fileName.c_str(),  tf );
                                           /*
  FILE *f=fopen("c:\\temp.txt","w");
  

  // grayvalue -> opacity
  {  
    mitk::TransferFunction::ControlPoints scalarOpacityPoints = tf->GetScalarOpacityPoints();
    fprintf(f,"// grayvalue->opacity \n"
              "{\n"
              "  vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();\n"
              "  f->RemoveAllPoints();\n");
    for ( mitk::TransferFunction::ControlPoints::iterator iter = scalarOpacityPoints.begin(); iter != scalarOpacityPoints.end(); ++iter )
    fprintf(f,"  f->AddPoint(%f,%f);\n",iter->first, iter->second);
    fprintf(f,"  f->Modified();\n"
              "}\n");
  }

  // gradient
  {
    mitk::TransferFunction::ControlPoints gradientOpacityPoints = tf->GetGradientOpacityPoints();
    fprintf(f,"// gradient at grayvalue->opacity \n"
              "{\n"
              "  vtkPiecewiseFunction *f=tf->GetGradientOpacityFunction();\n"
              "  f->RemoveAllPoints();\n");
    for ( mitk::TransferFunction::ControlPoints::iterator iter = gradientOpacityPoints.begin(); iter != gradientOpacityPoints.end(); ++iter )
    fprintf(f,"  f->AddPoint(%f,%f);\n",iter->first, iter->second);
    fprintf(f,"  f->Modified();\n"
              "}\n");
  }

  // color
  {
    mitk::TransferFunction::RGBControlPoints points = tf->GetRGBPoints();

    fprintf(f,"// grayvalue->color \n"
              "{\n"
              "  vtkColorTransferFunction *f=tf->GetColorTransferFunction();\n"
              "  f->RemoveAllPoints();\n");
    for ( mitk::TransferFunction::RGBControlPoints::iterator iter = points.begin(); iter != points.end(); ++iter )
    fprintf(f,"  f->AddRGBPoint(%f,%f,%f,%f);\n",iter->first, iter->second[0], iter->second[1], iter->second[2]);
    fprintf(f,"  f->Modified();\n"
              "}\n");
  }
  
  fclose(f);
  
  LOG_INFO << "saved under C:\\temp.txt";
                                             */
}


void QmitkTransferFunctionGeneratorWidget::OnLoadPreset( )
{
  if(tfpToChange.IsNull())
    return;

  std::string fileName;

  QString fileNames = QFileDialog::getOpenFileName( this,"Choose a file to open the transferfunction from",".", "Transferfunction (*.xml)"  );
  

  fileName=fileNames.ascii();

  LOG_INFO << "Loading Transferfunction from path: " << fileName;

  mitk::TransferFunction::Pointer tf = mitk::DeserializeTransferFunction(fileName.c_str());

  if(tf.IsNotNull())
  {
    if( histoGramm )
      tf->InitializeByItkHistogram( histoGramm );

    tfpToChange->SetValue( tf );
    
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    emit SignalUpdateCanvas();

    /*
    vtkFloatingPointType* dp = tf->GetScalarOpacityFunction()->GetDataPointer();
    for (int i = 0; i < tf->GetScalarOpacityFunction()->GetSize(); i++)
    {
      LOG_INFO << "x: " << dp[i * 2] << " y: " << dp[i * 2 + 1];
    }
    */
  
  }

}


void QmitkTransferFunctionGeneratorWidget::OnMitkInternalPreset( int mode )
{
  if(tfpToChange.IsNull())
    return;

  //first item is only information
  if( --mode == -1 )
    return;

  //revert to first item
  m_TransferFunctionComboBox->setCurrentIndex( 0 );

  // -- Creat new TransferFunction
  tfpToChange->GetValue()->SetTransferFunctionMode( mode );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalUpdateCanvas();
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaLevelWindow(int dx, int dy)
{
  //std::string infoText;
  
  
//  m_InfoThreshold->setText( QString( x.c_str() ) );

  if(tfpToChange.IsNull())
    return;

  thPos += dx;
  thDelta += dy;
  
  if(thDelta < 1)
    thDelta = 1;
    
  if(thDelta > 256)
    thDelta = 256;
    
  if(thPos < histoMinimum)
    thPos = histoMinimum;
    
  if(thPos > histoMaximum)
    thPos = histoMaximum;
                                                      /*
  LOG_INFO << "threshold pos: " << thPos << " delta: " << thDelta;

  LOG_INFO << "histoMinimum: " << histoMinimum << " max: " << histoMaximum;
                                                        */
  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();
  
  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint(thPos-thDelta,0);
    f->AddPoint(thPos-thDelta+1,1);
    f->AddPoint(thPos+thDelta-1,1);
    f->AddPoint(thPos+thDelta,0);
    f->Modified();
  }  

  // gradient at grayvalue->opacity
  {  
    vtkPiecewiseFunction *f=tf->GetGradientOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint( 0, 1.0 );
    f->Modified();
  }
/*
  // grayvalue->color
  {  
    vtkColorTransferFunction *ctf=tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();
    ctf->AddRGBPoint( 0, 1.0, 1.0, 1.0 );
    ctf->Modified();
  }
  */
  tf->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalUpdateCanvas();
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaThreshold(int dx, int dy)
{
  //std::string infoText;
  
  
//  m_InfoThreshold->setText( QString( x.c_str() ) );

  if(tfpToChange.IsNull())
    return;

  thPos += dx;
  thDelta += dy;
  
  if(thDelta < 1)
    thDelta = 1;
    
  if(thDelta > 256)
    thDelta = 256;
    
  if(thPos < histoMinimum)
    thPos = histoMinimum;
    
  if(thPos > histoMaximum)
    thPos = histoMaximum;
          /*
  LOG_INFO << "threshold pos: " << thPos << " delta: " << thDelta;

  LOG_INFO << "histoMinimum: " << histoMinimum << " max: " << histoMaximum;
            */
  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();
  
  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint(thPos-    thDelta,0);
    //f->AddPoint(thPos-0.5*thDelta,0);
    //f->AddPoint(thPos+0.5*thDelta,1);
    f->AddPoint(thPos+    thDelta,1);
    f->Modified();
  }  

  // gradient at grayvalue->opacity
  {  
    vtkPiecewiseFunction *f=tf->GetGradientOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint( 0, 1.0 );
    f->Modified();
  }
/*
  // grayvalue->color
  {  
    vtkColorTransferFunction *ctf=tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();
    ctf->AddRGBPoint( 0, 1.0, 1.0, 1.0 );
    ctf->Modified();
  }
  */

  tf->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalUpdateCanvas();

}



QmitkTransferFunctionGeneratorWidget::~QmitkTransferFunctionGeneratorWidget()
{
}


void QmitkTransferFunctionGeneratorWidget::SetDataTreeNode(mitk::DataTreeNode* node)
{
  histoGramm = NULL;

  if (node)
  {
    tfpToChange = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction"));
    
    if(!tfpToChange)
    {
      if (! dynamic_cast<mitk::Image*>(node->GetData()))
      {
        LOG_WARN << "QmitkTransferFunctionGeneratorWidget::SetDataTreeNode called with non-image node";
        return;
      }
      
      node->SetProperty("TransferFunction", tfpToChange = mitk::TransferFunctionProperty::New() );
      dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction"));
    }
    
    mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

    if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
      tf->InitializeByItkHistogram( histoGramm = image->GetScalarHistogram() );
                  
    thPos = ( tf->GetMin() + tf->GetMax() ) / 2; 
    
    histoMinimum = tf->GetMin();
    histoMaximum = tf->GetMax();
  }
  else
  {
    tfpToChange = 0;
  }
}



