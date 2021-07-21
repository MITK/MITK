// -----------------------------------------------------------------------
// MatchPoint - DKFZ translational registration framework
//
// Copyright (c) German Cancer Research Center (DKFZ),
// Software development for Integrated Diagnostics and Therapy (SIDT).
// ALL RIGHTS RESERVED.
// See mapCopyright.txt or
// http://www.dkfz.de/en/sidt/projects/MatchPoint/copyright.html
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notices for more information.
//
//------------------------------------------------------------------------
/*!
// @file
// @version $Revision$ (last changed revision)
// @date    $Date$ (last change date)
// @author  $Author$ (last changed by)
// Subversion HeadURL: $HeadURL$
*/


#ifndef __MITK_PROCESS_EXECUTOR_H
#define __MITK_PROCESS_EXECUTOR_H


#include <vector>

#include <itkObject.h>

//#include "mapString.h"

//#include "mapMacros.h"
#include "mitkEvents.h"
//#include "mapClassMacros.h"
//#include "mapMAPUtilitiesExports.h"
#include <MitkSegmentationExports.h>

/** Set built-in type.  Creates member set"name"() (e.g., setVisibility()); */
#define maSetMacro(name,type) \
	virtual void set##name (const type& value) \
	{ \
		if (this->_##name != value) \
		{ \
			this->_##name = value; \
			this->Modified(); \
		} \
	}

#define maGetConstMacro(name,type) \
	virtual type get##name () const \
	{ \
		return this->_##name; \
	}

namespace mitk
{
	namespace events
	{
		/*!@class TaskBatchEvent1
		 * @brief Base event indicating an output of an external process (e.g. by ProcessExecutor). The output is stored in the comment.
		 * @ingroup Events
		 */
		maEventMacro(ExternalProcessOutputEvent, AnyMatchPointEvent, MITKSEGMENTATION_EXPORT);

		/*!@class TaskBatchEvent
		 * @brief Event indicating an standard output (STDOUT) of an external process (e.g. by ProcessExecutor). The output is stored in the comment.
		 * @ingroup Events
		 */
		maEventMacro(ExternalProcessStdOutEvent, ExternalProcessOutputEvent, MITKSEGMENTATION_EXPORT);
		/*!@class TaskBatchEvent
		* @brief Event indicating an error output of an external process (e.g. by ProcessExecutor). The output is stored in the comment.
		* @ingroup Events
		*/
		maEventMacro(ExternalProcessStdErrEvent, ExternalProcessOutputEvent, MITKSEGMENTATION_EXPORT);
	}  // namespace events

	namespace utilities
	{

		/*! Helper class that allows to execute an application with arguments.
		 * You may register an observer for an ExternalProcessOutputEvent, ExternalProcessStdOutEvent or ExternalProcessStdErrEvent
		 * in order to get notified of any output.
		 * @remark The events will only be invoked if the pipes are NOT(!) shared. By default the pipes are not shared.
		 */
		class MITKSEGMENTATION_EXPORT ProcessExecutor : public itk::Object
		{
		public:
			using Self = ProcessExecutor;
			using Superclass = ::itk::Object;
			using Pointer = ::itk::SmartPointer<Self>;
			using ConstPointer = ::itk::SmartPointer<const Self>;

			itkTypeMacro(ProcessExecutor, ::itk::Object);
			itkFactorylessNewMacro(Self);

			maSetMacro(SharedOutputPipes, bool);
			maGetConstMacro(SharedOutputPipes, bool);

			using ArgumentListType = std::vector<std::string>;

			bool execute(const std::string& executionPath, const std::string& executableName,
						 ArgumentListType argumentList);

			/**Executes the process. This version assumes that the executable name is the first argument in the argument list
			* and has already been converted to its OS dependent name via the static convert function of this class.*/
			bool execute(const std::string& executionPath, const ArgumentListType& argumentList);

			int getExitValue();

			//static std::string getOSDependendExecutableName(const core::String& name);

		protected:
			ProcessExecutor();
			~ProcessExecutor() override;

			int _exitValue;

			/*! specifies if the child process should share the output pipes (true) or not (false).
			 * If pipes are not shared the output will be passed by invoking ExternalProcessOutputEvents
			 * @remark The events will only be invoked if the pipes are NOT(!) shared.*/
			bool _SharedOutputPipes;
		private:
			ProcessExecutor(const Self& source) = delete;
			void operator=(const Self&) = delete;  //purposely not implemented
		};

	}  // namespace utilities
}  // namespace map
#endif
