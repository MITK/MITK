/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkAbstractView.h>
#include "ui_QmitkFiberBasedSoftwarePhantomViewControls.h"
#include <itkVectorImage.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkFiberBundleX.h>
#include <mitkPlanarEllipse.h>

/*!
\brief QmitkFiberBasedSoftwarePhantomView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/

// Forward Qt class declarations

using namespace std;

class QmitkFiberBasedSoftwarePhantomView : public QmitkAbstractView
{

    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const string VIEW_ID;

    QmitkFiberBasedSoftwarePhantomView();
    virtual ~QmitkFiberBasedSoftwarePhantomView();

    virtual void CreateQtPartControl(QWidget *parent);
    void SetFocus();

    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
    typedef itk::Image<float, 3>            ItkFloatImgType;
    typedef itk::Vector<double,3>           GradientType;
    typedef vector<GradientType>            GradientListType;

    template<int ndirs> vector<itk::Vector<double,3> > MakeGradientList() ;

protected slots:

    void OnDrawCircle();
    void OnAddBundle();
    void OnFlipButton();
    void GenerateFibers();
    void GenerateImage();
    void OnFiberDensityChanged(int value);
    void OnFiberSamplingChanged(int value);
    void OnTensionChanged(double value);
    void OnContinuityChanged(double value);
    void OnBiasChanged(double value);
    void JoinBundles();
    void OnVarianceChanged(double value);
    void OnDistributionChanged(int value);

protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&);

    GradientListType GenerateHalfShell(int NPoints);

    Ui::QmitkFiberBasedSoftwarePhantomViewControls* m_Controls;

    void UpdateGui();

private:

    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
    void EnableCrosshairNavigation();
    void DisableCrosshairNavigation();
    void NodeAdded( const mitk::DataNode* node );
    void NodeRemoved(const mitk::DataNode* node);

    struct QmitkPlanarFigureData
    {
        QmitkPlanarFigureData()
            : m_Figure(0)
            , m_EndPlacementObserverTag(0)
            , m_SelectObserverTag(0)
            , m_StartInteractionObserverTag(0)
            , m_EndInteractionObserverTag(0)
            , m_Flipped(0)
        {
        }

        mitk::PlanarFigure* m_Figure;
        unsigned int m_EndPlacementObserverTag;
        unsigned int m_SelectObserverTag;
        unsigned int m_StartInteractionObserverTag;
        unsigned int m_EndInteractionObserverTag;
        unsigned int m_Flipped;
    };

    std::map<mitk::DataNode*, QmitkPlanarFigureData>    m_DataNodeToPlanarFigureData;
    mitk::Image::Pointer                                m_TissueMask;
    mitk::DataNode::Pointer                             m_SelectedFiducial;
    mitk::DataNode::Pointer                             m_SelectedImage;
    mitk::DataNode::Pointer                             m_SelectedBundle;
    vector< mitk::DataNode::Pointer >                   m_SelectedBundles;
};
