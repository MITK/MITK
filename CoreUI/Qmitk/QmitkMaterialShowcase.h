#ifndef QMITKMATERIALSHOWCASE_H
#define QMITKMATERIALSHOWCASE_H

#include <QmitkRenderWindow.h>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "QmitkRenderWindow.h"
#include "mitkCommon.h"
#include "mitkDataTree.h"
#include "mitkDataTreeNode.h"
#include "mitkMaterialProperty.h"
#include "mitkSurface.h"
#include "QtGui/QMenubarUpdatedEvent"
#include "vtkRenderer.h"
#include "vtkTextActor.h"

#include "ui_QmitkMaterialShowcase.h"


class QMITK_EXPORT QmitkMaterialShowcase : public QWidget, public Ui::QmitkMaterialShowcase
{
    Q_OBJECT

public:
    QmitkMaterialShowcase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~QmitkMaterialShowcase();

    virtual mitk::MaterialProperty * GetMaterialProperty();
    virtual unsigned int GetShowcaseNumber();

public slots:
    virtual void SetMaterialProperty( mitk::MaterialProperty * property );
    virtual void SetColor( mitk::MaterialProperty::Color color );
    virtual void SetColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue );
    virtual void SetColorCoefficient( vtkFloatingPointType coefficient );
    virtual void SetSpecularColor( mitk::MaterialProperty::Color color );
    virtual void SetSpecularColor( vtkFloatingPointType red, vtkFloatingPointType green, vtkFloatingPointType blue );
    virtual void SetSpecularCoefficient( vtkFloatingPointType specularCoefficient );
    virtual void SetSpecularPower( vtkFloatingPointType specularPower );
    virtual void SetOpacity( vtkFloatingPointType opacity );
    virtual void SetLineWidth( float lineWidth );
    virtual void UpdateRenderWindow();
    virtual void SetRepresentation( mitk::MaterialProperty::RepresentationType representation );
    virtual void SetInterpolation( mitk::MaterialProperty::InterpolationType interpolation );
    virtual void SetShowcaseNumber( unsigned int number );

signals:
    void Selected( QmitkMaterialShowcase* );

protected:
    mitk::Surface::Pointer m_Surface;
    mitk::DataTree::Pointer m_DataTree;
    mitk::DataTreeNode::Pointer m_DataTreeNode;
    mitk::MaterialProperty::Pointer m_MaterialProperty;
    unsigned int m_ShowcaseNumber;
    vtkTextActor* m_TextActor;
    vtkRenderer* m_TextRenderer;

    virtual void mousePressEvent( QMouseEvent * e );
    virtual void showEvent( QShowEvent * e );

protected slots:
  virtual void languageChange() {}

private:
    //void init();
    //void destroy();

};

#endif // QMITKMATERIALSHOWCASE_H
