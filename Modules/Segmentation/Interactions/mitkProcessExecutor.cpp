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


#include <iostream>
#include <fstream>

#include <itksys/Process.h>
#include <itksys/SystemTools.hxx>

#include "mitkProcessExecutor.h"
//#include "mapFileDispatch.h"

namespace mitk
{
	namespace utilities
	{

		/*std::string
		ProcessExecutor::
		getOSDependendExecutableName(const core::String& name)
		{
#if defined(_WIN32)

			if (::map::core::FileDispatch::getExtension(name).empty())
			{
				return name + ".exe";
			}
			
			
				return name;
			

#else

			if (::map::core::FileDispatch::getPath(name).empty())
			{
				return "./" + name;
			}
			else
			{
				return name;
			}

#endif
		};*/

		int
		ProcessExecutor::
		getExitValue()
		{
			return this->_exitValue;
		};

		bool
		ProcessExecutor::
		execute(const std::string& executionPath, const ArgumentListType& argumentList)
		{
			//convert to char* array with terminating null element;
			const char** pArguments = new const char*[argumentList.size() + 1];
			pArguments[argumentList.size()] = nullptr;

			for (ArgumentListType::size_type index = 0; index < argumentList.size(); ++index)
			{
				pArguments[index] = argumentList[index].c_str();
			}

			bool normalExit = false;

			try
			{
				itksysProcess* processID = itksysProcess_New();
				itksysProcess_SetCommand(processID, pArguments);

				itksysProcess_SetWorkingDirectory(processID, executionPath.c_str());

				if (this->_SharedOutputPipes)
				{
					itksysProcess_SetPipeShared(processID, itksysProcess_Pipe_STDOUT, 1);
					itksysProcess_SetPipeShared(processID, itksysProcess_Pipe_STDERR, 1);
				}

				itksysProcess_Execute(processID);

				char* rawOutput = nullptr;
				int outputLength = 0;
				std::cout<< "in mitk executor" << std::endl;
				while (true)
				{
					int dataStatus = itksysProcess_WaitForData(processID, &rawOutput, &outputLength, nullptr);

					if (dataStatus == itksysProcess_Pipe_STDOUT)
					{
						std::string data(rawOutput, outputLength);
						this->InvokeEvent(mitk::events::ExternalProcessStdOutEvent(nullptr, data));
					}
					else if (dataStatus == itksysProcess_Pipe_STDERR)
					{
						std::string data(rawOutput, outputLength);
						this->InvokeEvent(mitk::events::ExternalProcessStdErrEvent(nullptr, data));
					}
					else
					{
						break;
					}
				}

				itksysProcess_WaitForExit(processID, nullptr);

				auto state = static_cast<itksysProcess_State_e>(itksysProcess_GetState(processID));

				normalExit = (state == itksysProcess_State_Exited);
				this->_exitValue = itksysProcess_GetExitValue(processID);
			}
			catch (...)
			{
				delete[] pArguments;
				throw;
			}

			delete[] pArguments;

			return normalExit;
		};

		/*bool
		ProcessExecutor::
		execute(const std::string& executionPath, const std::string& executableName,
				ArgumentListType argumentList)
		{
			::map::core::String executableName_OS = getOSDependendExecutableName(executableName);

			argumentList.insert(argumentList.begin(), executableName_OS);

			return execute(executionPath, argumentList);
		};*/

		ProcessExecutor::
		ProcessExecutor()
		{
			this->_exitValue = 0;
			this->_SharedOutputPipes = false;
		};

		ProcessExecutor::
		~ProcessExecutor()
		= default;

	}  // namespace utilities
}  // namespace map
