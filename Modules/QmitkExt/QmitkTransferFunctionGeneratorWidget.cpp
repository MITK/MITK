#include "QmitkTransferFunctionGeneratorWidget.h"

#include <mitkTransferFunctionProperty.h>
#include <mitkRenderingManager.h>

#include <QFileDialog>

#include <SceneSerializationExports.h>
#include <mitkTransferFunctionPropertySerializer.h>
#include <mitkTransferFunctionPropertyDeserializer.h>


static char* presetNames[] = {  "choose an internal transferfunction preset",
                              "CT Generic",
                              "CT Black & White",
                              "CT Thorax large",
                              "CT Thorax small",
                              "CT Bone",
                              "CT Bone (with Gradient)",
                              "CT Cardiac",
                              "MR Generic" };

static int numPresetNames = sizeof( presetNames ) / sizeof( char * );

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
    thDelta = 100;
  }
   
  // Presets Tab
  {
    for(int r=0; r< numPresetNames; r++)
        m_TransferFunctionComboBox->insertItem( presetNames[r]);
    
    
    connect( m_TransferFunctionComboBox, SIGNAL( activated( int ) ), this, SLOT( OnMitkInternalPreset( int ) ) );

    connect( m_SavePreset, SIGNAL( clicked() ), this, SLOT( OnSavePreset() ) );
    
    connect( m_LoadPreset, SIGNAL( clicked() ), this, SLOT( OnLoadPreset() ) );
  }
  
  presetFileName = ".";
}




void QmitkTransferFunctionGeneratorWidget::OnSavePreset( )
{
  if(tfpToChange.IsNull())
    return;
    
  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  std::string fileName;
  std::string fileNameOutput;

  presetFileName = QFileDialog::getSaveFileName( this,"Choose a filename to save the transferfunction",presetFileName, "Transferfunction (*.xml)" );
  

  fileName=presetFileName.ascii();
 
  LOG_INFO << "Saving Transferfunction under path: " << fileName;
  
  fileNameOutput= ReduceFileName(fileName);

  if ( mitk::TransferFunctionPropertySerializer::SerializeTransferFunction( fileName.c_str(),  tf ))
    m_InfoPreset->setText( QString( (std::string("saved ")+ fileNameOutput).c_str() ) );
  else
    m_InfoPreset->setText( QString( std::string("saving failed").c_str() ) );
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
  std::string fileNameOutput;

  presetFileName = QFileDialog::getOpenFileName( this,"Choose a file to open the transferfunction from",presetFileName, "Transferfunction (*.xml)"  );

  fileName=presetFileName.ascii();

  LOG_INFO << "Loading Transferfunction from path: " << fileName;
  
  fileNameOutput= ReduceFileName(fileName);

  mitk::TransferFunction::Pointer tf = mitk::TransferFunctionPropertyDeserializer::DeserializeTransferFunction(fileName.c_str());

  if(tf.IsNotNull())
  {
    /*
    if( histoGramm )
      tf->InitializeByItkHistogram( histoGramm );
    */
    
    tfpToChange->SetValue( tf );
   
    m_InfoPreset->setText( QString( (std::string("loaded ")+ fileNameOutput).c_str() ) );
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
  m_InfoPreset->setText( QString( (std::string("selected ")+ std::string(presetNames[mode+1])).c_str() ) );
}


static double transformationGlocke ( double x )
{
  double z = 0.1;
  
  double a = 2 - 2 * z;
  
  double b = 2 * z - 1;
  
  x = a * x + b;
  
  return x;
}

static double stepFunctionGlocke ( double x )
{
  x = 1-(2*x -1.0); // map [0.5;1] to [0,1]
  x = x * ( 3*x - 2*x*x ); // apply smoothing function
  
  x = x * x;
  
  return x;
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaLevelWindow(int dx, int dy)      // bell
{
  //std::string infoText;
  
//  m_InfoThreshold->setText( QString( x.c_str() ) );

  if(tfpToChange.IsNull())
    return;

  thPos += dx;
  thDelta -= dy;
  
  if(thDelta < 1)
    thDelta = 1;
    
  if(thDelta > 1024)
    thDelta = 1024;
    
  if(thPos < histoMinimum)
    thPos = histoMinimum;
    
  if(thPos > histoMaximum)
    thPos = histoMaximum;

  std::stringstream ss;
  
  ss << "Click on the cross and move the mouse"<<"\n"
     <<"\n"
     << "center at " << thPos << "\n"
     << "width " << thDelta * 2;
     
  m_InfoLevelWindow->setText( QString( ss.str().c_str() ) );

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();
  
  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    
    for( int r = 0; r<= 6; r++)
    {
      double relPos = (r / 6.0) * 0.5 + 0.5;
      f->AddPoint(thPos+thDelta*(-transformationGlocke(relPos)),stepFunctionGlocke(relPos));
      f->AddPoint(thPos+thDelta*( transformationGlocke(relPos)),stepFunctionGlocke(relPos));
    }

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

static double stepFunctionThreshold ( double x )
{
  x = 0.5*x + 0.5; // map [-1;1] to [0,1]
  x = x * ( 3*x - 2*x*x ); // apply smoothing function

  x = x * x;

  return x;
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaThreshold(int dx, int dy)   // LEVELWINDOW
{

  if(tfpToChange.IsNull())
    return;

  thPos += dx;
  thDelta += dy;
  
  if(thDelta < 1)
    thDelta = 1;
    
  if(thDelta > 1024)
    thDelta = 1024;
    
  if(thPos < histoMinimum)
    thPos = histoMinimum;
    
  if(thPos > histoMaximum)
    thPos = histoMaximum;
          /*
  LOG_INFO << "threshold pos: " << thPos << " delta: " << thDelta;

  LOG_INFO << "histoMinimum: " << histoMinimum << " max: " << histoMaximum;
            */
 

  std::stringstream ss;
  
  ss << "Click on the cross and move the mouse"<<"\n"
     <<"\n"
     << "threshold at " << thPos << "\n"
     << "width " << thDelta * 2;
     
  m_InfoThreshold->setText( QString( ss.str().c_str() ) );

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();
  
  // grayvalue->opacity
  {   
    vtkPiecewiseFunction *f=tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    
    for( int r = 1; r<= 4; r++)
    {
      double relPos = r / 4.0;
      f->AddPoint(thPos+thDelta*(-relPos),stepFunctionThreshold(-relPos));
      f->AddPoint(thPos+thDelta*( relPos),stepFunctionThreshold( relPos));
    }
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

std::string QmitkTransferFunctionGeneratorWidget::ReduceFileName(std::string fileNameLong )
{
  if (fileNameLong.length()< 40)
    return fileNameLong;
  
  //LOG_INFO <<" fileName > 20 ";
  
  std::string fileNameShort;
  std::string fileNameRevert;
  
  for(int i=0; i< fileNameLong.length(); i++)
  {
    if(i<3)
    {
      char x= fileNameLong[i];
      fileNameShort= fileNameShort+x;
    }
    if(i==3)
    {
      fileNameShort= fileNameShort+"...";
      break;
    }
  }
  //LOG_INFO <<" fileNameShort: " << fileNameShort.c_str();
  for(int i=fileNameLong.length()-1; i>= 0; i--)
  {
    std::string x=std::string("")+fileNameLong[i];
    
    if ( x.compare("/")==0 || x.compare("\\")==0)
    {
      fileNameRevert= "/" + fileNameRevert;
      break;
    }
      
    if (i>=fileNameLong.length()-24)
    {
      fileNameRevert= x+ fileNameRevert;
      //LOG_INFO <<" fileNameRevert: " << fileNameRevert.c_str();
    }
    else
    {
      fileNameRevert= "/..." + fileNameRevert;
      break;
    }
   
  }
  
  return fileNameShort+fileNameRevert;
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
    {
//      tf->InitializeByItkHistogram( histoGramm = image->GetScalarHistogram() );
      histoMinimum= image->GetScalarValueMin();
      histoMaximum= image->GetScalarValueMax();
    }
            
    thPos = ( histoMinimum + histoMaximum ) / 2; 
    
  }
  else
  {
    tfpToChange = 0;
    m_InfoPreset->setText( QString( "" ) );
    
  }
}



