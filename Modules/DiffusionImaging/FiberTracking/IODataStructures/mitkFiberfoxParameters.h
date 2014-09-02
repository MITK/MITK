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

#ifndef _MITK_FiberfoxParameters_H
#define _MITK_FiberfoxParameters_H

#include <mitkFiberfoxNoTemplateParameters.h>
#include <mitkDiffusionNoiseModel.h>
#include <mitkDiffusionSignalModel.h>
#include <mitkRicianNoiseModel.h>
#include <mitkChiSquareNoiseModel.h>

#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkAstroStickModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkRawShModel.h>

namespace mitk {

/**
  * \brief Datastructure to manage the Fiberfox signal generation parameters.
  *
  */
template< class ScalarType = double >
class FiberfoxParameters
{
public:

    typedef itk::Image<double, 3>                           ItkDoubleImgType;
    typedef itk::Image<unsigned char, 3>                    ItkUcharImgType;
    typedef DiffusionSignalModel<ScalarType>                DiffusionModelType;
    typedef std::vector< DiffusionModelType* >              DiffusionModelListType;
    typedef DiffusionNoiseModel<ScalarType>                 NoiseModelType;

    FiberfoxParameters();
    ~FiberfoxParameters();

    /** Get same parameter object with different template parameter */
    template< class OutType >
    FiberfoxParameters< OutType > CopyParameters()
    {
        FiberfoxParameters< OutType > out;

        out.m_FiberGen = m_FiberGen;
        out.m_SignalGen = m_SignalGen;
        out.m_Misc = m_Misc;

        if (m_NoiseModel!=NULL)
        {
            if (dynamic_cast<mitk::RicianNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::RicianNoiseModel<OutType>();
            else if (dynamic_cast<mitk::ChiSquareNoiseModel<ScalarType>*>(m_NoiseModel))
                out.m_NoiseModel = new mitk::ChiSquareNoiseModel<OutType>();
            out.m_NoiseModel->SetNoiseVariance(m_NoiseModel->GetNoiseVariance());
        }

        for (unsigned int i=0; i<m_FiberModelList.size()+m_NonFiberModelList.size(); i++)
        {
            mitk::DiffusionSignalModel<OutType>* outModel = NULL;
            mitk::DiffusionSignalModel<ScalarType>* signalModel = NULL;
            if (i<m_FiberModelList.size())
                signalModel = m_FiberModelList.at(i);
            else
                signalModel = m_NonFiberModelList.at(i-m_FiberModelList.size());

            if (dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel))
                outModel = new mitk::StickModel<OutType>(dynamic_cast<mitk::StickModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel))
                outModel = new mitk::TensorModel<OutType>(dynamic_cast<mitk::TensorModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel))
                outModel = new mitk::RawShModel<OutType>(dynamic_cast<mitk::RawShModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel))
                outModel = new mitk::BallModel<OutType>(dynamic_cast<mitk::BallModel<ScalarType>*>(signalModel));
            else if (dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel))
                outModel = new mitk::AstroStickModel<OutType>(dynamic_cast<mitk::AstroStickModel<ScalarType>*>(signalModel));
            else  if (dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel))
                outModel = new mitk::DotModel<OutType>(dynamic_cast<mitk::DotModel<ScalarType>*>(signalModel));

            if (i<m_FiberModelList.size())
                out.m_FiberModelList.push_back(outModel);
            else
                out.m_NonFiberModelList.push_back(outModel);
        }

        return out;
    }

    /** Not templated parameters */
    FiberGenerationParameters           m_FiberGen;             ///< Fiber generation parameters
    SignalGenerationParameters          m_SignalGen;            ///< Signal generation parameters
    MiscFiberfoxParameters              m_Misc;                 ///< GUI realted and I/O parameters

    /** Templated parameters */
    DiffusionModelListType              m_FiberModelList;       ///< Intra- and inter-axonal compartments.
    DiffusionModelListType              m_NonFiberModelList;    ///< Extra-axonal compartments.
    NoiseModelType*                     m_NoiseModel;           ///< If != NULL, noise is added to the image.

    void PrintSelf();                           ///< Print parameters to stdout.
    void SaveParameters(std::string filename);       ///< Save image generation parameters to .ffp file.
    void LoadParameters(std::string filename);       ///< Load image generation parameters from .ffp file.
};
}

#include "mitkFiberfoxParameters.cpp"

#endif

