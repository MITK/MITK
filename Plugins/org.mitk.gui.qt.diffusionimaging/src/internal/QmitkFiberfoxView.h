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



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkAbstractView.h>
#include "ui_QmitkFiberfoxViewControls.h"
#include <itkVectorImage.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkFiberBundleX.h>
#include <mitkPlanarEllipse.h>

/*!
\brief View for fiber based diffusion software phantoms (Fiberfox).

\sa QmitkFunctionality
\ingroup Functionalities
*/

// Forward Qt class declarations

using namespace std;

class QmitkFiberfoxView : public QmitkAbstractView
{

    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const string VIEW_ID;

    QmitkFiberfoxView();
    virtual ~QmitkFiberfoxView();

    virtual void CreateQtPartControl(QWidget *parent);
    void SetFocus();

    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
    typedef itk::Vector<double,3>           GradientType;
    typedef vector<GradientType>            GradientListType;

    template<int ndirs> vector<itk::Vector<double,3> > MakeGradientList() ;

protected slots:

    void OnDrawROI();       ///< adds new ROI, handles interactors etc.
    void OnAddBundle();     ///< adds new fiber bundle to datastorage
    void OnFlipButton();    ///< negate one coordinate of the fiber waypoints in the selcted planar figure. needed in case of unresolvable twists
    void GenerateFibers();  ///< generate fibers from the selected ROIs
    void GenerateImage();   ///< generate artificial image from the selected fiber bundle
    void JoinBundles();     ///< merges selcted fiber bundles into one
    void CopyBundles();      ///< add copy of the selected bundle to the datamanager
    void TransformBundles(); ///< rotate and shift selected bundles

    /** update fibers if any parameter changes */
    void OnFiberDensityChanged(int value);
    void OnFiberSamplingChanged(int value);
    void OnTensionChanged(double value);
    void OnContinuityChanged(double value);
    void OnBiasChanged(double value);
    void OnVarianceChanged(double value);
    void OnDistributionChanged(int value);
    void OnAddT2Smearing(int value);
    void OnAddGibbsRinging(int value);
    void OnConstantRadius(int value);

protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&);

    GradientListType GenerateHalfShell(int NPoints);    ///< generate vectors distributed over the halfsphere

    Ui::QmitkFiberfoxViewControls* m_Controls;

    void UpdateGui();   ///< enable/disbale buttons etc. according to current datamanager selection
    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
    void EnableCrosshairNavigation();   ///< enable crosshair navigation if planar figure interaction ends
    void DisableCrosshairNavigation();  ///< disable crosshair navigation if planar figure interaction starts
    void NodeAdded( const mitk::DataNode* node );   ///< add observers
    void NodeRemoved(const mitk::DataNode* node);   ///< remove observers

    /** structure to keep track of planar figures and observers */
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

    std::map<mitk::DataNode*, QmitkPlanarFigureData>    m_DataNodeToPlanarFigureData;   ///< map each planar figure uniquely to a QmitkPlanarFigureData
    mitk::Image::Pointer                                m_TissueMask;                   ///< mask defining which regions of the image should contain signal and which are containing only noise
    mitk::DataNode::Pointer                             m_SelectedFiducial;             ///< selected planar ellipse
    mitk::DataNode::Pointer                             m_SelectedImage;
    mitk::DataNode::Pointer                             m_SelectedBundle;
    mitk::DataNode::Pointer                             m_SelectedDWI;
    vector< mitk::DataNode::Pointer >                   m_SelectedBundles;
};
