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

#ifndef mitkModelFitParameter_h
#define mitkModelFitParameter_h

#include <mitkImage.h>
#include "mitkModelFitUIDHelper.h"

#include "MitkModelFitExports.h"

namespace mitk
{
    namespace modelFit
    {
        /**
         *  @brief Data class that stores all information about a modelfit parameter.
     * Such a parameter is going to be fitted for the according model.
         */
        class MITKMODELFIT_EXPORT Parameter : public itk::LightObject
        {
        public:
      mitkClassMacroItkParent(Parameter, itk::LightObject)
            itkSimpleNewMacro(Parameter)

      enum Type
        {
          ParameterType,
          DerivedType,
          CriterionType,
          EvaluationType
        };

            Parameter();

            /** ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME */
            std::string name;
            /** ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME */
            Type type;
            /** ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME */
            std::string unit;
            /** ModelFitConstants::PARAMETER_SCALE_PROPERTY_NAME */
            float scale;
            /** Corresponding image */
            mitk::Image::ConstPointer image;
        };

        /** Extracts the parameter information stored in the data instance and returns it as parameter instance.
     * If the data does not encode an parameter/fit result a Null pointer will be returned.
         *  @param data The data instance that contains parameter information that should be extracted.
         *  @return The newly created parameter instance on success or NULL otherwise.
         */
        MITKMODELFIT_EXPORT Parameter::Pointer ExtractParameterFromData(const mitk::BaseData* data);
    }
}

#endif // mitkModelFitParameter_h