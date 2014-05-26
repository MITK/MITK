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

// MITK
#include "QmitkFieldmapGeneratorView.h"
#include <QmitkStdMultiWidget.h>
#include <QmitkDataStorageComboBox.h>
#include <mitkNodePredicateDataType.h>
#include <itkFieldmapGeneratorFilter.h>
#include <mitkImage.h>

const std::string QmitkFieldmapGeneratorView::VIEW_ID = "org.mitk.views.fieldmapgenerator";

QmitkFieldmapGeneratorView::QmitkFieldmapGeneratorView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
{

}

QmitkFieldmapGeneratorView::~QmitkFieldmapGeneratorView()
{

}

void QmitkFieldmapGeneratorView::CreateQtPartControl( QWidget *parent )
{
    // build up qt view, unless already done
    if ( !m_Controls )
    {
        // create GUI widgets from the Qt Designer's .ui file
        m_Controls = new Ui::QmitkFieldmapGeneratorViewControls;
        m_Controls->setupUi( parent );

        m_Controls->m_SelectedImageBox->SetDataStorage(this->GetDataStorage());
        mitk::TNodePredicateDataType<mitk::Image>::Pointer isMitkImage = mitk::TNodePredicateDataType<mitk::Image>::New();
        m_Controls->m_SelectedImageBox->SetPredicate(isMitkImage);

        connect((QObject*) m_Controls->m_GenerateFieldmap, SIGNAL(clicked()), (QObject*) this, SLOT(GenerateFieldmap()));
        connect((QObject*) m_Controls->m_PlaceFieldSource, SIGNAL(clicked()), (QObject*) this, SLOT(PlaceFieldSource()));
        connect((QObject*) m_Controls->m_SourceVarianceBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnVarianceChanged(double)));
        connect((QObject*) m_Controls->m_SourceHeightBox, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(OnHeightChanged(double)));

    }
}

void QmitkFieldmapGeneratorView::OnVarianceChanged(double value)
{
    if (m_SelectedSource.IsNotNull())
    {
        m_SelectedSource->SetProperty("pointsize", mitk::FloatProperty::New(value));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkFieldmapGeneratorView::OnHeightChanged(double value)
{
    if (m_SelectedSource.IsNotNull())
    {
        m_SelectedSource->SetProperty("color", mitk::ColorProperty::New(0, 0, value/100.0));
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkFieldmapGeneratorView::PlaceFieldSource()
{
    if (m_Controls->m_SelectedImageBox->GetSelectedNode().IsNull() || !m_MultiWidget)
    {
        m_Controls->m_WorldPosLabel->setText("-");
        m_Controls->m_IndexLabel->setText("-");
        return;
    }

    mitk::Point3D index;
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_SelectedImageBox->GetSelectedNode()->GetData());
    mitk::BaseGeometry* geom = img->GetGeometry();

    if ( geom->IsInside(m_WorldPoint) )
    {
        img->GetGeometry()->WorldToIndex(m_WorldPoint, index);
        mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
        pointSet->InsertPoint(0, m_WorldPoint);


        mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_Controls->m_SelectedImageBox->GetSelectedNode());

        mitk::DataNode::Pointer node = mitk::DataNode::New();
        node->SetData(pointSet);
        QString name = QString("FieldSource_%1").arg(children->size());
        node->SetName(name.toStdString());
        node->SetVisibility(true);

        float minSpacing;
        if(geom->GetSpacing()[0]<geom->GetSpacing()[1] && geom->GetSpacing()[0]<geom->GetSpacing()[2])
            minSpacing = geom->GetSpacing()[0];
        else if (geom->GetSpacing()[1] < geom->GetSpacing()[2])
            minSpacing = geom->GetSpacing()[1];
        else
            minSpacing = geom->GetSpacing()[2];

        node->SetProperty("pointsize", mitk::FloatProperty::New(minSpacing*5));
        node->SetProperty("color", mitk::ColorProperty::New(0, 0, 1));

        GetDataStorage()->Add(node, m_Controls->m_SelectedImageBox->GetSelectedNode());
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void QmitkFieldmapGeneratorView::GenerateFieldmap()
{
    if (m_Controls->m_SelectedImageBox->GetSelectedNode().IsNull())
        return;

    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_SelectedImageBox->GetSelectedNode()->GetData());
    mitk::BaseGeometry* geom = img->GetGeometry();

    typedef itk::Image< double, 3 > FieldMapType;
    itk::Vector<double>                 spacing;              ///< output image spacing
    mitk::Point3D                       origin;               ///< output image origin
    itk::FieldmapGeneratorFilter< FieldMapType >::MatrixType                directionMatrix;      ///< output image rotation
    itk::FieldmapGeneratorFilter< FieldMapType >::OutputImageRegionType     imageRegion;          ///< output image size

    spacing = geom->GetSpacing();
    origin = geom->GetOrigin();
    imageRegion.SetSize(0, img->GetDimension(0));
    imageRegion.SetSize(1, img->GetDimension(1));
    imageRegion.SetSize(2, img->GetDimension(2));
    for (int r=0; r<3; r++)
        for (int c=0; c<3; c++)
            directionMatrix[r][c]=geom->GetMatrixColumn(c)[r]/spacing[c];

    std::vector< double >               variances;
    std::vector< double >               heights;
    std::vector< mitk::Point3D >        worldPositions;

    mitk::DataStorage::SetOfObjects::ConstPointer children = GetDataStorage()->GetDerivations(m_Controls->m_SelectedImageBox->GetSelectedNode());
    for( mitk::DataStorage::SetOfObjects::const_iterator it = children->begin(); it != children->end(); ++it )
        if(dynamic_cast<mitk::PointSet*>((*it)->GetData()))
        {
            float var = 0; (*it)->GetPropertyValue("pointsize", var); variances.push_back(var);
            float color[3]; (*it)->GetColor(color); heights.push_back(color[2]*100);
            mitk::PointSet* ps = dynamic_cast<mitk::PointSet*>((*it)->GetData());
            mitk::Point3D point;
            ps->GetPointIfExists(0, &point);
            worldPositions.push_back(point);
        }

    vnl_vector_fixed< double, 3 > gradient, offset;
    gradient[0] = m_Controls->m_xGradientBox->value();
    gradient[1] = m_Controls->m_yGradientBox->value();
    gradient[2] = m_Controls->m_zGradientBox->value();
    offset[0] = m_Controls->m_xOffsetBox->value();
    offset[1] = m_Controls->m_yOffsetBox->value();
    offset[2] = m_Controls->m_zOffsetBox->value();

    itk::FieldmapGeneratorFilter< FieldMapType >::Pointer filter = itk::FieldmapGeneratorFilter< FieldMapType >::New();
    filter->SetHeights(heights);
    filter->SetVariances(variances);
    filter->SetWorldPositions(worldPositions);
    filter->SetSpacing(spacing);
    filter->SetOrigin(origin);
    filter->SetDirectionMatrix(directionMatrix);
    filter->SetImageRegion(imageRegion);
    filter->SetGradient(gradient);
    filter->SetOffset(offset);
    filter->Update();

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    mitk::Image::Pointer image = mitk::Image::New();
    image->InitializeByItk(filter->GetOutput(0));
    image->SetVolume(filter->GetOutput(0)->GetBufferPointer());
    resultNode->SetData( image );
    resultNode->SetName(m_Controls->m_SelectedImageBox->GetSelectedNode()->GetName()+"_Fieldmap");
    GetDataStorage()->Add(resultNode);
}

void QmitkFieldmapGeneratorView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
    m_MultiWidget = &stdMultiWidget;

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::Pointer command = itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::New();
        command->SetCallbackFunction( this, &QmitkFieldmapGeneratorView::OnSliceChanged );
        m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::Pointer command = itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::New();
        command->SetCallbackFunction( this, &QmitkFieldmapGeneratorView::OnSliceChanged );
        m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }

    {
        mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
        itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::Pointer command = itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::New();
        command->SetCallbackFunction( this, &QmitkFieldmapGeneratorView::OnSliceChanged );
        m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
    }
}

void QmitkFieldmapGeneratorView::StdMultiWidgetNotAvailable()
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

void QmitkFieldmapGeneratorView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
    if (m_Controls->m_SelectedImageBox->GetSelectedNode().IsNotNull())
        m_Controls->m_SelectedImageBox->GetSelectedNode()->RemoveObserver( m_PropertyObserverTag );

    m_Controls->m_InputData->setTitle("Please Select Reference Image");
    m_SelectedSource = NULL;

    // iterate selection
    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if ( node.IsNotNull() && (dynamic_cast<mitk::PointSet*>(node->GetData())) )
        {
            m_Controls->m_SourceNameLabel->setText(node->GetName().c_str());
            m_SelectedSource = node;
            float variance = 0; node->GetPropertyValue("pointsize", variance);
            m_Controls->m_SourceVarianceBox->setValue(variance);

            float color[3]; node->GetColor(color);
            m_Controls->m_SourceHeightBox->setValue(color[2]*100);
        }
    }

    if (m_Controls->m_SelectedImageBox->GetSelectedNode().IsNotNull())
    {
        itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::Pointer command = itk::ReceptorMemberCommand<QmitkFieldmapGeneratorView>::New();
        command->SetCallbackFunction( this, &QmitkFieldmapGeneratorView::OnSliceChanged );
        m_PropertyObserverTag = m_Controls->m_SelectedImageBox->GetSelectedNode()->AddObserver( itk::ModifiedEvent(), command );

        m_Controls->m_InputData->setTitle("Reference Image");
    }
}

void QmitkFieldmapGeneratorView::OnSliceChanged(const itk::EventObject& /*e*/)
{
    if (m_Controls->m_SelectedImageBox->GetSelectedNode().IsNull() || !m_MultiWidget)
    {
        m_Controls->m_WorldPosLabel->setText("-");
        m_Controls->m_IndexLabel->setText("-");
        return;
    }

    m_WorldPoint = m_MultiWidget->GetCrossPosition();
    QString posSting = QString::number(m_WorldPoint[0]); posSting += ", "; posSting += QString::number(m_WorldPoint[1]); posSting += ", "; posSting += QString::number(m_WorldPoint[2]);
    m_Controls->m_WorldPosLabel->setText(posSting.toStdString().c_str());

    mitk::Point3D index;
    mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(m_Controls->m_SelectedImageBox->GetSelectedNode()->GetData());

    if ( m_Controls->m_SelectedImageBox->GetSelectedNode()->GetData()->GetGeometry()->IsInside(m_WorldPoint) )
    {
        img->GetGeometry()->WorldToIndex(m_WorldPoint, index);
        posSting = QString::number(index[0]); posSting += ", "; posSting += QString::number(index[1]); posSting += ", "; posSting += QString::number(index[2]);
         m_Controls->m_IndexLabel->setText(posSting.toStdString().c_str());
    }
}
