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

#ifndef Q_MOC_RUN
#include <mitkFiberBundle.h>
#include <mitkPlanarEllipse.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkRicianNoiseModel.h>
#include <itkTractsToDWIImageFilter.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkStickModel.h>
#include <mitkAstroStickModel.h>
#include <mitkDotModel.h>
#include <mitkFiberfoxParameters.h>
#include <itkStatisticsImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>
#endif

#include <QThread>
#include <QObject>
#include <QTimer>
#include <QTime>

/*!
\brief View for fiber based diffusion software phantoms (Fiberfox). See "Fiberfox: Facilitating the creation of realistic white matter software phantoms" (DOI: 10.1002/mrm.25045) for details.
*/

// Forward Qt class declarations

class QmitkFiberfoxView;

class QmitkFiberfoxWorker : public QObject
{
    Q_OBJECT

public:

    QmitkFiberfoxWorker(QmitkFiberfoxView* view);

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

    static const std::string VIEW_ID;

    QmitkFiberfoxView();
    virtual ~QmitkFiberfoxView();

    virtual void CreateQtPartControl(QWidget *parent) override;
    void SetFocus() override;

    typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
    typedef itk::Vector<double,3>           GradientType;
    typedef std::vector<GradientType>       GradientListType;
    typedef itk::VectorImage< short, 3 >    ItkDwiType;
    typedef itk::Image<double, 3>           ItkDoubleImgType;
    typedef itk::Image<float, 3>            ItkFloatImgType;
    typedef itk::Image<unsigned char, 3>    ItkUcharImgType;

    template<int ndirs> std::vector<itk::Vector<double,3> > MakeGradientList();

protected slots:

    void SetOutputPath();           ///< path where image is automatically saved to after the simulation is finished
    void LoadParameters();          ///< load fiberfox parameters
    void SaveParameters();          ///< save fiberfox parameters

    void BeforeThread();
    void AfterThread();
    void KillThread();              ///< abort simulation
    void UpdateSimulationStatus();  ///< print simulation progress and satus messages

    void OnDrawROI();               ///< adds new ROI, handles interactors etc.
    void OnAddBundle();             ///< adds new fiber bundle to datastorage
    void OnFlipButton();            ///< negate one coordinate of the fiber waypoints in the selcted planar figure. needed in case of unresolvable twists
    void GenerateFibers();          ///< generate fibers from the selected ROIs
    void GenerateImage();           ///< start image simulation
    void JoinBundles();             ///< merges selcted fiber bundles into one
    void CopyBundles();             ///< add copy of the selected bundle to the datamanager
    void ApplyTransform();          ///< rotate and shift selected bundles
    void AlignOnGrid();             ///< shift selected fiducials to nearest voxel center
    void Comp1ModelFrameVisibility(int index);  ///< only show parameters of selected signal model for compartment 1
    void Comp2ModelFrameVisibility(int index);  ///< only show parameters of selected signal model for compartment 2
    void Comp3ModelFrameVisibility(int index);  ///< only show parameters of selected signal model for compartment 3
    void Comp4ModelFrameVisibility(int index);  ///< only show parameters of selected signal model for compartment 4
    void ShowAdvancedOptions(int state);

    /** update fibers if any parameter changes */
    void OnFiberDensityChanged(int value);
    void OnFiberSamplingChanged(double value);
    void OnTensionChanged(double value);
    void OnContinuityChanged(double value);
    void OnBiasChanged(double value);
    void OnVarianceChanged(double value);
    void OnDistributionChanged(int value);
    void OnConstantRadius(int value);

    /** update GUI elements */
    void OnAddNoise(int value);
    void OnAddGhosts(int value);
    void OnAddDistortions(int value);
    void OnAddEddy(int value);
    void OnAddSpikes(int value);
    void OnAddAliasing(int value);
    void OnAddMotion(int value);
    void OnMaskSelected(int value);
    void OnFibSelected(int value);
    void OnTemplateSelected(int value);

protected:

    /// \brief called by QmitkAbstractView when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

    GradientListType GenerateHalfShell(int NPoints);    ///< generate vectors distributed over the halfsphere

    Ui::QmitkFiberfoxViewControls* m_Controls;

    void SimulateForExistingDwi(mitk::DataNode* imageNode);     ///< add artifacts to existing diffusion weighted image
    void SimulateImageFromFibers(mitk::DataNode* fiberNode);    ///< simulate new diffusion weighted image
    FiberfoxParameters UpdateImageParameters(bool all=true, bool save=false);  ///< update fiberfox paramater object
    void UpdateGui();                                           ///< enable/disbale buttons etc. according to current datamanager selection
    void PlanarFigureSelected( itk::Object* object, const itk::EventObject& );
    void EnableCrosshairNavigation();               ///< enable crosshair navigation if planar figure interaction ends
    void DisableCrosshairNavigation();              ///< disable crosshair navigation if planar figure interaction starts
    void NodeAdded( const mitk::DataNode* node ) override;   ///< add observers
    void NodeRemoved(const mitk::DataNode* node) override;   ///< remove observers
    void SaveParameters(QString filename);

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
    mitk::DataNode::Pointer                             m_SelectedFiducial;             ///< selected planar ellipse
    mitk::DataNode::Pointer                             m_SelectedImageNode;
    std::vector< mitk::DataNode::Pointer >              m_SelectedBundles;
    std::vector< mitk::DataNode::Pointer >              m_SelectedBundles2;
    std::vector< mitk::DataNode::Pointer >              m_SelectedFiducials;
    std::vector< mitk::DataNode::Pointer >              m_SelectedImages;

    QString m_ParameterFile;    ///< parameter file name

    // GUI thread
    QmitkFiberfoxWorker     m_Worker;   ///< runs filter
    QThread                 m_Thread;   ///< worker thread
    bool                    m_ThreadIsRunning;
    QTimer*                 m_SimulationTimer;
    QTime                   m_SimulationTime;
    QString                 m_SimulationStatusText;

    /** Image filters that do all the simulations. */
    itk::TractsToDWIImageFilter< short >::Pointer           m_TractsToDwiFilter;

    friend class QmitkFiberfoxWorker;
};
