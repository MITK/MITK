/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <vtkSphereSource.h>
#include <vtkPolyData.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <mitkCameraController.h>
#include <mitkOpenGLRenderer.h>


/**
 * A material showcase simply displays a sphere which is visualized with some
 * material property. This widget is used by the QmitkMaterialEditor dialog.
 */

void QmitkMaterialShowcase::init()
{
    m_ShowcaseNumber = 0;
    m_MaterialProperty = new mitk::MaterialProperty();
    vtkSphereSource* sphereSource = vtkSphereSource::New();
    sphereSource->SetThetaResolution(25);
    sphereSource->SetPhiResolution(25);
    sphereSource->Update();
    vtkPolyData* sphere = sphereSource->GetOutput();
    m_Surface = mitk::Surface::New();
    m_Surface->SetVtkPolyData( sphere );
    m_DataTreeNode = mitk::DataTreeNode::New();                                
    m_DataTreeNode->SetData( m_Surface );
    m_DataTree = mitk::DataTree::New();
    mitk::DataTreePreOrderIterator it( m_DataTree );    
    it.Add( m_DataTreeNode );
    m_SelectableGLWidget->GetRenderer()->SetData( &it );
    m_SelectableGLWidget->GetRenderer()->SetMapperID( 2 );
}

void QmitkMaterialShowcase::destroy()
{
    if (m_MaterialProperty != NULL)
        delete m_MaterialProperty;
}
void QmitkMaterialShowcase::SetMaterialProperty( mitk::MaterialProperty* property )
{
    if (m_MaterialProperty != NULL)
        delete m_MaterialProperty;
    
    m_MaterialProperty = new mitk::MaterialProperty( *property );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}


void QmitkMaterialShowcase::SetColor( mitk::MaterialProperty::Color color )
{
    m_MaterialProperty->SetColor( color );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}

void  QmitkMaterialShowcase::SetColor( VTK_FLOAT_TYPE red, VTK_FLOAT_TYPE green, VTK_FLOAT_TYPE blue )
{
    m_MaterialProperty->SetColor( red, green, blue );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow(); 
}
    
void  QmitkMaterialShowcase::SetColorCoefficient( VTK_FLOAT_TYPE coefficient )
{
    m_MaterialProperty->SetColorCoefficient( coefficient );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}
    
void  QmitkMaterialShowcase::SetSpecularColor( mitk::MaterialProperty::Color color )
{
    m_MaterialProperty->SetSpecularColor( color );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}
    
void  QmitkMaterialShowcase::SetSpecularColor( VTK_FLOAT_TYPE red, VTK_FLOAT_TYPE green, VTK_FLOAT_TYPE blue )
{
    m_MaterialProperty->SetSpecularColor( red, green, blue );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}
    
void  QmitkMaterialShowcase::SetSpecularCoefficient( VTK_FLOAT_TYPE specularCoefficient )
{
    m_MaterialProperty->SetSpecularCoefficient( specularCoefficient );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}
    
void  QmitkMaterialShowcase::SetSpecularPower( VTK_FLOAT_TYPE specularPower )
{
    m_MaterialProperty->SetSpecularPower( specularPower );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}
    
void  QmitkMaterialShowcase::SetOpacity( VTK_FLOAT_TYPE opacity )
{
    m_MaterialProperty->SetOpacity( opacity );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}


void QmitkMaterialShowcase::UpdateRenderWindow()
{
    m_SelectableGLWidget->GetRenderWindow()->Update();
}


mitk::MaterialProperty* QmitkMaterialShowcase::GetMaterialProperty()
{
    return m_MaterialProperty;
}


void QmitkMaterialShowcase::SetRepresentation( mitk::MaterialProperty::RepresentationType representation )
{
    m_MaterialProperty->SetRepresentation( representation );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}


void QmitkMaterialShowcase::SetInterpolation( mitk::MaterialProperty::InterpolationType interpolation ) 
{
    m_MaterialProperty->SetInterpolation( interpolation );
    m_DataTreeNode->SetProperty( "material", m_MaterialProperty );
    this->UpdateRenderWindow();
}


void QmitkMaterialShowcase::mousePressEvent( QMouseEvent * e )
{
    QWidget::mousePressEvent( e );
    emit Selected( this );  
}


void QmitkMaterialShowcase::SetShowcaseNumber( unsigned int number )
{
    m_ShowcaseNumber = number;
}


unsigned int QmitkMaterialShowcase::GetShowcaseNumber()
{
    return m_ShowcaseNumber;
}
