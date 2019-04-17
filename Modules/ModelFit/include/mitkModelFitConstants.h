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

#ifndef _MITK_MODEL_FIT_CONSTANTS_H_
#define _MITK_MODEL_FIT_CONSTANTS_H_

#include <string>

#include "MitkModelFitExports.h"

namespace mitk
{
    struct MITKMODELFIT_EXPORT ModelFitConstants
    {
        /**
        * Name of the "root" property for all information concerning model fitting.
        */
        static const std::string MODEL_FIT_PROPERTY_NAME();

        /**
        * Name of the "uid" property for all data objects concerning model fitting.
        */
        static const std::string UID_PROPERTY_NAME();

        /**
        * modelfit.input.variables ist eine Map der Variablennamen, die einem Eingabebild zu eigen sind und
        * unabhängig von einem Fit sind.
        * Deren Wert ist ein Array, welches die Variablenwerte für die einzelnen Zeitschritte enthält.
        * Die Länge eines solchen Arrays muss entweder der Anzahl der Zeitschritte dieses Bildes entsprechen
        * oder genau 1 sein, wenn der Wert für alle Zeitschritte dieses Bildes gleich ist.
        * Beispiele:
        * ["x1":[580], "x2":[3060], "x3":[41]]      (1 Zeitschritt, 3 Variablen)
        * ["TE":[2.47, 5.85, 9.23, 12.61], "TI":[10.3]] (4 Zeitschritte, 2 Variablen)
        */
        static const std::string INPUT_VARIABLES_PROPERTY_NAME();

        /**
        * modelfit.parameter.name ist der Bezeichner des Parameters, wie es im Funktions-String (modelfit.model.function) vorkommt.
        */
        static const std::string PARAMETER_NAME_PROPERTY_NAME();

        /**
        * modelfit.parameter.unit ist die Einheit des Parameters und dient nur der formatierten Ausgabe. Standardwert: ""
        */
        static const std::string PARAMETER_UNIT_PROPERTY_NAME();

        /**
        * modelfit.parameter.scale ist die Skalierung des Parameters. Standardwert: 1
        */
        static const std::string PARAMETER_SCALE_PROPERTY_NAME();

        /**
        * modelfit.parameter.type ist der type des parameters. Standardwert: PARAMETER_TYPE_VALUE_PARAMETER
        */
        static const std::string PARAMETER_TYPE_PROPERTY_NAME();

        /**
        * modelfit.parameter.type Wert für normale Parameters.
        */
        static const std::string PARAMETER_TYPE_VALUE_PARAMETER();

        /**
        * modelfit.parameter.type Wert für derived Parameters.
        */
        static const std::string PARAMETER_TYPE_VALUE_DERIVED_PARAMETER();

        /**
        * modelfit.parameter.type Wert für Crtierion-Parameters.
        */
        static const std::string PARAMETER_TYPE_VALUE_CRITERION();

        /**
        * modelfit.parameter.type Wert für Evaluation-Parameters.
        */
        static const std::string PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER();

        /**
        * modelfit.model.type ist der Bezeichner des Modelltyps und dient nur der formatierten Ausgabe
        */
        static const std::string MODEL_TYPE_PROPERTY_NAME();

        /**
        * modelfit.model.name ist der Bezeichner des Fits und dient nur der formatierten Ausgabe.
        */
        static const std::string MODEL_NAME_PROPERTY_NAME();

        /**
        * modelfit.model.function ist der Funktions-String, der geparsed wird, um die Kurve zu plotten.
        */
        static const std::string MODEL_FUNCTION_PROPERTY_NAME();

        /**
        * modelfit.model.functionClass ist die ID der Modellklasse.
        */
        static const std::string MODEL_FUNCTION_CLASS_PROPERTY_NAME();

        /**
        * modelfit.model.x ist der Name der Variable, die die x-Werte stellt, wie sie im Funktions-String vorkommt. Wird nur gebraucht wenn modelfit.model.function definiert ist.
        */
        static const std::string MODEL_X_PROPERTY_NAME();

        /**
        * Default value for MODEL_X_PROPERTY_NAME.
        */
        static const std::string MODEL_X_VALUE_DEFAULT();

        /**
        * modelfit.xaxis.name ist der Bezeichner der x-Achse und dient nur der formatierten Ausgabe. Standardwert: "Time"
        */
        static const std::string XAXIS_NAME_PROPERTY_NAME();

        /**
        * Default value for XAXIS_NAME_PROPERTY_NAME.
        */
        static const std::string XAXIS_NAME_VALUE_DEFAULT();

        /**
        * modelfit.xaxis.unit ist die Einheit der x-Achse und dient nur der formatierten Ausgabe. Standardwert: "ms"
        */
        static const std::string XAXIS_UNIT_PROPERTY_NAME();

        /**
        * modelfit.xaxis.name ist der Bezeichner der x-Achse und dient nur der formatierten Ausgabe. Standardwert: "Intensity"
        */
        static const std::string YAXIS_NAME_PROPERTY_NAME();

        /**
        * Default value for YAXIS_NAME_PROPERTY_NAME.
        */
        static const std::string YAXIS_NAME_VALUE_DEFAULT();

        /**
        * modelfit.xaxis.unit ist die Einheit der x-Achse und dient nur der formatierten Ausgabe. Standardwert: ""
        */
        static const std::string YAXIS_UNIT_PROPERTY_NAME();

        /**
        * modelfit.fit.uid ist eine einzigartige ID (unabhängig von Ausführungsrechner, Session oder Applikationsinstanz),
        * die einem Fit vergeben wird um alle zugehörigen Bilder eindeutig zu kennzeichnen.
        */
        static const std::string FIT_UID_PROPERTY_NAME();

        /**
        * modelfit.fit.name is the human readable name of the fit. Use UID, if you want a unique identifier. This is just
        * used for visualization purposes.
        */
        static const std::string FIT_NAME_PROPERTY_NAME();

        /**
        * modelfit.fit.type defines the type of model fitting; e.g. pixel based or ROI based. Thus it determines the meaning of other fit specific informations.
        */
        static const std::string FIT_TYPE_PROPERTY_NAME();

        static const std::string FIT_TYPE_VALUE_PIXELBASED();

    static const std::string FIT_TYPE_VALUE_ROIBASED();

        /**
        * modelfit.fit.input.imageUID defines the UID of the image that is used directly or indirectly (then it is source for input.data) to make the fit.
        */
        static const std::string FIT_INPUT_IMAGEUID_PROPERTY_NAME();

        /**
        * modelfit.fit.input.roiUID defines the UID of the ROI that is used to make the fit. If not set no Mask was used or specified.
        */
        static const std::string FIT_INPUT_ROIUID_PROPERTY_NAME();

        /**
        * modelfit.fit.input.data defines the data signal that is used to make the fit and was extracted from the input image (e.g. in ROIbased fit).
        */
        static const std::string FIT_INPUT_DATA_PROPERTY_NAME();

        /**
        * modelfit.fit.staticParameters ist eine Map der Variablennamen, die im Funktions-String vorkommen und
        * nicht mit modelfit.input.variables abgedeckt sind. Dies können z.B. Konstanten sein oder Variablen,
        * die sich (abhängig vom Fit) über den Zeitverlauf ändern, wie z.B. der Mittelwert einer Maske über einen Arterienquerschnitt.
        * (Entspricht den StaticParameters in mitk::ModelBase)
        * Der Wert der Variablen ist ein Array, welches die Variablenwerte enthält.
        * Beispiel:
        * ["AIF":[2, 8, 8, 4, 5], "tau":[0.42]] (insgesamt 5 Zeitschritte, 2 Variablen)
        */
        static const std::string FIT_STATIC_PARAMETERS_PROPERTY_NAME();


    };
}

#endif
