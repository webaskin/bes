// BESDapResponse.h

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004-2009 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#ifndef I_BESDapResponse
#define I_BESDapResponse 1

#include <DAS.h>

#include "BESResponseObject.h"
#include "BESDataHandlerInterface.h"

using namespace libdap ;

/** @brief Represents an OPeNDAP DAP response object within the BES
 */
class BESDapResponse: public BESResponseObject {
    private:
#if 0
        DAS * _das;
#endif
        string d_dap_client_protocol;
        bool d_explicit_containers;

        string d_request_xml_base;

    protected:
        bool is_dap2();
        void read_contexts();
    public:
        BESDapResponse() :
            BESResponseObject(), d_dap_client_protocol("2.0"),
            d_explicit_containers(true), d_request_xml_base("")
        {
            read_contexts();
        }

        virtual ~BESDapResponse()
        {
        }

        /// Return the dap version string sent by the client (e.g., the OLFS)
        string get_dap_client_protocol() const { return d_dap_client_protocol; }

        /// Should containers be explicitly represented in the DD* responses?
        bool get_explicit_containers() const { return d_explicit_containers; }

        /// Return the xml:base URL for this request.
        string get_request_xml_base() const { return d_request_xml_base; }

        virtual void set_container(const string &cn) = 0;
	virtual void set_constraint( BESDataHandlerInterface &dhi ) ;
        virtual void clear_container() = 0;

        virtual void dump(ostream &strm) const;
};

#endif // I_BESDapResponse

