/**
 * @file
 *
 * Copyright (C) 2013 Max Oberberger <max@oberbergers.de>
 *
 * Last modified: 2013 February 23, 20:05:02 by max
 *
 * This file is part of ba-hla.
 * 
 * ba-hla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ba-hla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ba-hla.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXAMPLECPPFEDERATE_H_
#define EXAMPLECPPFEDERATE_H_

#include "ExampleFedAmb.h"
#include <RTI/RTIambassador.h>
#include <memory>

#define READY_TO_RUN "ReadyToRun"

using namespace rti1516;

/**
 * Example Federate implementation
 */
class ExampleCPPFederate
{
	public:
		std::auto_ptr<rti1516::RTIambassador> rtiamb; /*!< RTI ambassador to comunicate with RTI */
		ExampleFedAmb      *fedamb; /*!< Own Federate Ambassador implementation */

		// public methods //
        //! Constructor
		ExampleCPPFederate();
        //! Destructor
		virtual ~ExampleCPPFederate();
        //! Main Simulation Method
		void runFederate( std::string federateName, 
                          std::string fom, 
                          std::string address );

	private:
		/// fom handles
		ObjectClassHandle      aHandle;
		AttributeHandle        aaHandle;
		AttributeHandle        abHandle;
		AttributeHandle        acHandle;
		InteractionClassHandle xHandle;
		ParameterHandle        xaHandle;
		ParameterHandle        xbHandle;

        std::wstring convertStringToWstring(const std::string& str);
        rti1516::VariableLengthData toVariableLengthData(const char* s);

		void initializeHandles();
		void waitForUser();
		void enableTimePolicy();
		void publishAndSubscribe();
		ObjectInstanceHandle registerObject();
		void updateAttributeValues( ObjectInstanceHandle objectHandle );
		void sendInteraction();
		void advanceTime( double timestep );
		void deleteObject( ObjectInstanceHandle objectHandle );
		double getLbts();
};

#endif /* EXAMPLECPPFEDERATE_H_ */
