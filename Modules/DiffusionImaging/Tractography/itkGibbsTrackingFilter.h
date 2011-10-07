#ifndef itkGibbsTrackingFilter_h
#define itkGibbsTrackingFilter_h

#include "itkProcessObject.h"
#include "itkVectorContainer.h"
#include "itkImage.h"

#include "GibbsTracking/pcRJMCMC.cpp"
#include "GibbsTracking/auxilary_classes.cpp"

#include <fstream>

namespace itk{

  template< class TInputQBallImage, class TInputROIImage >
  class GibbsTrackingFilter :
  public ProcessObject{
  public:
    typedef GibbsTrackingFilter Self;
    typedef ProcessObject Superclass;
    typedef SmartPointer< Self > Pointer;
    typedef SmartPointer< const Self > ConstPointer;

    itkNewMacro(Self);
    itkTypeMacro( GibbsTrackingFilter, ProcessObject );

    /** Types for the DWI Input Image **/
    typedef TInputQBallImage InputQBallImageType;

    /** Types for the Mask Image **/
    typedef TInputROIImage MaskImageType;
    typedef typename MaskImageType::Pointer MaskImageTypePointer;

    typedef std::vector< itk::Point<float, 3> > FiberTractType;
    typedef std::vector< FiberTractType > FiberBundleType;

    itkSetMacro( TempStart, float );
    itkGetMacro( TempStart, float );

    itkSetMacro( TempEnd, float );
    itkGetMacro( TempEnd, float );

    itkSetMacro( NumIt, int );
    itkGetMacro( NumIt, int );

    itkSetMacro( ParticleWeight, float );
    itkGetMacro( ParticleWeight, float );

    /** width of particle sigma (std-dev of gaussian around center) **/
    itkSetMacro( ParticleWidth, float );
    itkGetMacro( ParticleWidth, float );

    /** length of particle from midpoint to ends **/
    itkSetMacro( ParticleLength, float );
    itkGetMacro( ParticleLength, float );

    itkSetMacro( ChempotConnection, float );
    itkGetMacro( ChempotConnection, float );

    itkSetMacro( ChempotParticle, float );
    itkGetMacro( ChempotParticle, float );

    itkSetMacro( InexBalance, float );
    itkGetMacro( InexBalance, float );

    itkSetMacro( Chempot2, float );
    itkGetMacro( Chempot2, float );

    itkSetMacro( FiberLength, int );
    itkGetMacro( FiberLength, int );

    itkSetMacro( AbortTracking, bool );
    itkGetMacro( AbortTracking, bool );

    itkSetMacro( CurrentStep, unsigned long );
    itkGetMacro( CurrentStep, unsigned long );

    itkSetMacro( SubtractMean, bool);
    itkGetMacro( SubtractMean, bool);

    /** Set/Get the Odf Input Image **/
    itkSetInputMacro(OdfImage, InputQBallImageType, 0);
    itkGetInputMacro(OdfImage, InputQBallImageType, 0);

    /** Set/Get the Input mask image **/
    itkSetMacro(MaskImage, MaskImageTypePointer);
    itkGetMacro(MaskImage, MaskImageTypePointer);

    itkGetMacro(NumParticles, unsigned long);
    itkGetMacro(NumConnections, unsigned long);
    itkGetMacro(NumAcceptedFibers, int);
    itkGetMacro(ProposalAcceptance, float);
    itkGetMacro(Steps, unsigned int);

    /** Entry Point For the Algorithm:  Is invoked when Update() is called
    either directly or through itk pipeline propagation
    **/
    void GenerateData();

    /** override the Process Object Update because we don't have a
    dataobject as an outpgnome themeut.  We can change this later by wrapping the
    tractcontainer in a dataobject decorator and letting the Superclass
    know about it.
    **/
    struct StochasticTractGenerationCallbackStruct{
      Pointer Filter;
    };

    virtual void Update(){
      this->GenerateData();
    }

    FiberBundleType* GetFiberBundle();
    float GetMemoryUsage();

  protected:

    GibbsTrackingFilter();
    virtual ~GibbsTrackingFilter();

    void ComputeFiberCorrelation();

    void BuildFibers(float* points, int numPoints);

    // Input Images
    typename InputQBallImageType::Pointer m_ItkQBallImage;
    typename MaskImageType::Pointer m_MaskImage;

    // Tracking parameters
    float   m_TempStart;  // Start temperature
    float   m_TempEnd;  // End temperature
    unsigned long m_NumIt;  // Total number of iterations
    unsigned long m_CurrentStep;  // current tracking step
    float   m_ParticleWeight; //w (unitless)
    float   m_ParticleWidth;  //sigma  (mm)
    float   m_ParticleLength; // ell (mm)
    float   m_ChempotConnection;  // gross L (chemisches potential)
    float   m_ChempotParticle;// unbenutzt (immer null, wenn groesser dann insgesamt weniger teilchen)
    float   m_InexBalance;    // gewichtung zwischen den lambdas
    // -5 ... 5 -> nur intern ... nur extern,default 0
    float   m_Chempot2;       // typischerweise 0,
    // korrektur fuer das geschaetzte integral
    int     m_FiberLength;
    bool    m_AbortTracking;
    bool    m_SubtractMean;
    int     m_NumAcceptedFibers;
    volatile bool    m_BuildFibers;
    unsigned int    m_Steps;
    float   m_Memory;
    float   m_ProposalAcceptance;

    RJMCMC* m_Sampler;
    FiberBundleType m_FiberBundle;
    unsigned long m_NumParticles;
    unsigned long m_NumConnections;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGibbsTrackingFilter.cpp"
#endif

#endif
