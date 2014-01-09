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
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkRicianNoiseModel.h>
#include <itkTractsToDWIImageFilter.h>
#include <itkAddArtifactsToDwiImageFilter.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkStickModel.h>
#include <mitkAstroStickModel.h>
#include <mitkDotModel.h>
#include <QThread>
#include <QObject>
#include <QTimer>
#include <QTime>

/*!
\brief View for fiber based diffusion software phantoms (Fiberfox).

\sa QmitkFunctionality
\ingroup Functionalities
*/

// Forward Qt class declarations

using namespace std;

class QmitkFiberfoxView;

class QmitkFiberfoxWorker : public QObject
{
    Q_OBJECT

public:

    QmitkFiberfoxWorker(QmitkFiberfoxView* view);
    int m_FilterType;

public slots:

    void run();

private:

    QmitkFiberfoxView*                  m_View;
};

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

    typedef itk::Image<double, 3>           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;
    typedef itk::Vector<double,3>           GradientType;
    typedef vector<GradientType>            GradientListType;

    template<int ndirs> vector<itk::Vector<double,3> > MakeGradientList();

protected slots:

    void SetOutputPath();
    void LoadParameters();
    void SaveParameters();

    void BeforeThread();
    void AfterThread();
    void KillThread();
    void UpdateSimulationStatus();

    void OnDrawROI();           ///< adds new ROI, handles interactors etc.
    void OnAddBundle();         ///< adds new fiber bundle to datastorage
    void OnFlipButton();        ///< negate one coordinate of the fiber waypoints in the selcted planar figure. needed in case of unresolvable twists
    void GenerateFibers();      ///< generate fibers from the selected ROIs
    void GenerateImage();       ///< generate artificial image from the selected fiber bundle
    void JoinBundles();         ///< merges selcted fiber bundles into one
    void CopyBundles();         ///< add copy of the selected bundle to the datamanager
    void ApplyTransform();    ///< rotate and shift selected bundles
    void AlignOnGrid();         ///< shift selected fiducials to nearest voxel center
    void Comp1ModelFrameVisibility(int index);///< only show parameters of selected fiber model type
    void Comp2ModelFrameVisibility(int index);///< only show parameters of selected non-fiber model type
    void Comp3ModelFrameVisibility(int index);///< only show parameters of selected non-fiber model type
    void Comp4ModelFrameVisibility(int index);///< only show parameters of selected non-fiber model type
    void ShowAdvancedOptions(int state);

    /** update fibers if any parameter changes */
    void OnFiberDensityChanged(int value);
    void OnFiberSamplingChanged(double value);
    void OnTensionChanged(double value);
    void OnContinuityChanged(double value);
    void OnBiasChanged(double value);
    void OnVarianceChanged(double value);
    void OnDistributionChanged(int value);
    void OnAddNoise(int value);
    void OnAddGhosts(int value);
    void OnAddDistortions(int value);
    void OnAddEddy(int value);
    void OnAddSpikes(int value);
    void OnAddAliasing(int value);
    void OnAddMotion(int value);
    void OnConstantRadius(int value);

protected:

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&);

    GradientListType GenerateHalfShell(int NPoints);    ///< generate vectors distributed over the halfsphere

    Ui::QmitkFiberfoxViewControls* m_Controls;

    void SimulateForExistingDwi(mitk::DataNode* imageNode);
    void SimulateImageFromFibers(mitk::DataNode* fiberNode);
    void UpdateImageParameters();                   ///< update iamge generation paaremeter struct
    void UpdateGui();                               ///< enable/disbale buttons etc. according to current datamanager selection
    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
    void EnableCrosshairNavigation();               ///< enable crosshair navigation if planar figure interaction ends
    void DisableCrosshairNavigation();              ///< disable crosshair navigation if planar figure interaction starts
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

    /** structure storing the image generation parameters */
    struct ImageParameters {
        itk::ImageRegion<3>                 imageRegion;
        itk::Vector<double,3>               imageSpacing;
        itk::Point<double,3>                imageOrigin;
        itk::Matrix<double, 3, 3>           imageDirection;
        unsigned int                        numGradients;
        double                              b_value;
        unsigned int                        repetitions;
        double                              signalScale;
        double                              tEcho;
        double                              tLine;
        double                              tInhom;
        double                              axonRadius;
        unsigned int                        interpolationShrink;
        double                              kspaceLineOffset;
        bool                                addGibbsRinging;
        double                              eddyStrength;
        double                              comp3Weight;
        double                              comp4Weight;
        int                                 spikes;
        double                              spikeAmplitude;
        double                              wrap;
        itk::Vector<double,3>               translation;
        itk::Vector<double,3>               rotation;
        bool                                doSimulateRelaxation;
        bool                                doSimulateEddyCurrents;
        bool                                doDisablePartialVolume;
        bool                                doAddMotion;
        bool                                randomMotion;

        mitk::DiffusionNoiseModel<double>* noiseModel;
        mitk::DiffusionNoiseModel<short>* noiseModelShort;
        mitk::DiffusionSignalModel<double>::GradientListType  gradientDirections;
        itk::TractsToDWIImageFilter< short >::DiffusionModelList fiberModelList, nonFiberModelList;
        QString signalModelString, artifactModelString;
        ItkDoubleImgType::Pointer           frequencyMap;
        ItkUcharImgType::Pointer            maskImage;
        mitk::DataNode::Pointer             resultNode;
        mitk::DataNode::Pointer             parentNode;
        QString                             outputPath;
    };

    ImageParameters                                     m_ImageGenParameters;
    ImageParameters                                     m_ImageGenParametersBackup;

    std::map<mitk::DataNode*, QmitkPlanarFigureData>    m_DataNodeToPlanarFigureData;   ///< map each planar figure uniquely to a QmitkPlanarFigureData
    mitk::DataNode::Pointer                             m_SelectedFiducial;             ///< selected planar ellipse
    mitk::DataNode::Pointer                             m_SelectedImage;
    mitk::DataNode::Pointer                             m_SelectedDWI;
    vector< mitk::DataNode::Pointer >                   m_SelectedBundles;
    vector< mitk::DataNode::Pointer >                   m_SelectedBundles2;
    vector< mitk::DataNode::Pointer >                   m_SelectedFiducials;
    vector< mitk::DataNode::Pointer >                   m_SelectedImages;

    // intra and inter axonal compartments
    mitk::StickModel<double> m_StickModel1;
    mitk::StickModel<double> m_StickModel2;
    mitk::TensorModel<double> m_ZeppelinModel1;
    mitk::TensorModel<double> m_ZeppelinModel2;
    mitk::TensorModel<double> m_TensorModel1;
    mitk::TensorModel<double> m_TensorModel2;

    // extra axonal compartment models
    mitk::BallModel<double> m_BallModel1;
    mitk::BallModel<double> m_BallModel2;
    mitk::AstroStickModel<double> m_AstrosticksModel1;
    mitk::AstroStickModel<double> m_AstrosticksModel2;
    mitk::DotModel<double> m_DotModel1;
    mitk::DotModel<double> m_DotModel2;

    QString m_ParameterFile;
    QString m_OutputPath;

    // GUI thread
    QmitkFiberfoxWorker     m_Worker;   ///< runs filter
    QThread                 m_Thread;   ///< worker thread
    itk::TractsToDWIImageFilter< short >::Pointer           m_TractsToDwiFilter;
    itk::AddArtifactsToDwiImageFilter< short >::Pointer     m_ArtifactsToDwiFilter;
    bool                    m_ThreadIsRunning;
    QTimer*                 m_SimulationTimer;
    QTime                   m_SimulationTime;
    QString                 m_SimulationStatusText;

    friend class QmitkFiberfoxWorker;
};
