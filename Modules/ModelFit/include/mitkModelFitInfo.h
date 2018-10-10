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

#ifndef mitkModelFitInfo_h
#define mitkModelFitInfo_h

#include <itkMutexLockHolder.h>
#include <itkSimpleFastMutexLock.h>

#include <mitkDataStorage.h>

#include "mitkModelFitConstants.h"
#include "mitkModelFitParameter.h"
#include "mitkModelFitStaticParameterMap.h"
#include "mitkScalarListLookupTable.h"
#include "mitkModelFitUIDHelper.h"
#include "mitkModelParameterizerBase.h"
#include "mitkModelTraitsInterface.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  namespace modelFit
  {
    /**
     *	@brief	Data class that stores all information about a modelfit that is relevant to the
     *			visualization and stored as properties in the result nodes.
     */
    class MITKMODELFIT_EXPORT ModelFitInfo : public itk::LightObject
    {
    public:
      typedef mitk::NodeUIDType UIDType;
      typedef std::vector<Parameter::Pointer> ParamListType;
      typedef ParamListType::const_iterator ConstIterType;

      mitkClassMacroItkParent(ModelFitInfo, itk::LightObject)
      itkSimpleNewMacro(ModelFitInfo)

      ModelFitInfo() :
        x(mitk::ModelFitConstants::MODEL_X_VALUE_DEFAULT()),
        xAxisName(mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT()),
        yAxisName(mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT())
      {
      }

      /**
       *	@brief		Adds the given parameter to this fit's parameter list if it doesn't
       *				exist already.
       *	@param p	The param that should be added to this fit's parameter list.
       */
      void AddParameter(Parameter::Pointer p);

      /**
       *	@brief		Searches for the parameter with the given name and type in the fit's
       *				parameter list and returns it.
       *	@param name	The name of the desired parameter.
       *	@param type The type of the desired parameter.
       *	@return		The parameter with the given name on success or NULL otherwise.
       */
      Parameter::ConstPointer GetParameter(const std::string& name, const Parameter::Type& type)
      const;

      /**
       *	@brief		Searches for the parameter with the given name and type in the fit's
       *				parameter list and deletes it if it exists.
       *	@param name	The name of the desired parameter.
       *	@param type The type of the desired parameter.
       */
      void DeleteParameter(const std::string& name, const Parameter::Type& type);

      /**Return const reference to the parameter list.*/
      const ParamListType& GetParameters() const;

      /** ModelFitConstants::MODEL_NAME_PROPERTY_NAME */
      std::string modelName;
      /** ModelFitConstants::MODEL_TYPE_PROPERTY_NAME */
      std::string modelType;
      /** ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME */
      std::string function;
      /** ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME */
      std::string functionClassID;
      /** ModelFitConstants::MODEL_X_PROPERTY_NAME */
      std::string x;

      /** ModelFitConstants::XAXIS_NAME_PROPERTY_NAME */
      std::string xAxisName;
      /** ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME */
      std::string xAxisUnit;

      /** ModelFitConstants::YAXIS_NAME_PROPERTY_NAME */
      std::string yAxisName;
      /** ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME */
      std::string yAxisUnit;

      /** ModelFitConstants::FIT_UID_PROPERTY_NAME */
      UIDType uid;
      /** ModelFitConstants::FIT_NAME_PROPERTY_NAME */
      std::string fitName;
      /** ModelFitConstants::FIT_TYPE_PROPERTY_NAME */
      std::string fitType;
      /** ModelFitConstants::FIT_STATIC_PARAMETERS_PROPERTY_NAME */
      StaticParameterMap staticParamMap;
      /** ModelFitConstants::FIT_INPUT_ROIUID_PROPERTY_NAME */
      UIDType roiUID;
      /** ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME */
      UIDType inputUID;
      /** ModelFitConstants::FIT_INPUT_DATA_PROPERTY_NAME */
      ScalarListLookupTable inputData;
      mitk::Image::Pointer inputImage;

    private:
      typedef ParamListType::iterator IterType;
      typedef itk::MutexLockHolder<itk::SimpleFastMutexLock> LockType;

      ParamListType parameterList;
      itk::SimpleFastMutexLock mutex;
    };

    /**
     *	@brief Reads the string property with the given name from the data of the given node
     *  and	returns its value. Throws a ModelFitException if the property doesn't exist.
     *	@param node	The node whose property value should be returned.
     *	@param prop	The name of the property that should be read.
     *	@return		The value of the found property.
     *	@throw ModelFitException	If the property doesn't exist or returns an empty string.
     */
    MITKMODELFIT_EXPORT const std::string GetMandatoryProperty(const mitk::DataNode* node,
        const std::string& prop);

    /**
    *	@brief		Reads the string property with the given name from the given base data and
    *				returns its value. Throws a ModelFitException if the property doesn't exist.
    *	@param data	The data whose property value should be returned.
    *	@param prop	The name of the property that should be read.
    *	@return		The value of the found property.
    *	@throw ModelFitException	If the property doesn't exist or returns an empty string.
    */
    MITKMODELFIT_EXPORT const std::string GetMandatoryProperty(const mitk::BaseData* data,
      const std::string& prop);

    /**
     *	@brief	Creates a new ModelFitInfo instance from the nodes in the passed storage.
     * The fit will be identified by the passed UID. Returns the instance on
     *					success.
     *	@param uid		The uid of the fit that should get its ModelFitInfo created and which identifies the nodes in the storage.
     *	@param storage	Pointer to the data storage containing any potential relevantThe nodes.
     *	@return			The newly created modelfit on success or NULL otherwise.
     */
    MITKMODELFIT_EXPORT ModelFitInfo::Pointer CreateFitInfoFromNode(const ModelFitInfo::UIDType& uid,
        const mitk::DataStorage* storage);

    /** creates a new ModelFitInfo instance from a passed modal instance and his traits instance*
    *	@param usedModel	Pointer to a model which was used for a fit, which should get a fit info created.
    *	@param modelTraits	Pointer to traits interface for the model that was used for the fit.
    * @param inputImage Pointer to the input image. If it has no UID yet, a property will be added to the node.
    * @param fitType String identifying the type of the fit (e.g. ROI based or voxel based)
    * @param fitName Optional human readable name of the fit.
    * @param roiUID UID of the ROI, if one was used.
    *	@return			The newly created modelfit on success or NULL otherwise.*/
    MITKMODELFIT_EXPORT ModelFitInfo::Pointer CreateFitInfoFromModelParameterizer(
      const ModelParameterizerBase* usedParameterizer, mitk::BaseData* inputImage,
      const std::string& fitType, const std::string& fitName = "", const NodeUIDType roiUID = "");
    /** @overload
     Overloaded version that allows additional definition of optional input data for the fit.*/
    MITKMODELFIT_EXPORT ModelFitInfo::Pointer CreateFitInfoFromModelParameterizer(
      const ModelParameterizerBase* usedParameterizer, mitk::BaseData* inputImage,
      const std::string& fitType, const ScalarListLookupTable& inputData, const std::string& fitName = "",
      const NodeUIDType roiUID = "");

    /** Returns all nodes that belong to the fit indicated by the passed UID.
     *	@param fitUID	The uid of the fit that is relevant for the query.
     *	@param storage	Pointer to the data storage containing any potential relevant nodes.
     *	@return			The set of found nodes or null if storage is not valid.
     */
    MITKMODELFIT_EXPORT DataStorage::SetOfObjects::ConstPointer GetNodesOfFit(
      const ModelFitInfo::UIDType& fitUID,
      const mitk::DataStorage* storage);

    typedef std::set<ModelFitInfo::UIDType> NodeUIDSetType;
    /** Returns the UIDs of all fits that are derived (directly or indirectly from the passed node).
     *	@param node	The node which defines the parent node. It will be searched in his derived nodes for fits.
     *	@param storage	Pointer to the data storage containing any potential relevant nodes.
     *	@return	The set of found uid will be returned.
     */
    MITKMODELFIT_EXPORT NodeUIDSetType GetFitUIDsOfNode(const mitk::DataNode* node,
        const mitk::DataStorage* storage);

  }
}

#endif // mitkModelFit_h
