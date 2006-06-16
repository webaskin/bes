// BESDataResponseHandler.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
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
 
// (c) COPYRIGHT University Corporation for Atmostpheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "BESDataResponseHandler.h"
#include "DDS.h"
#include "cgi_util.h"
#include "BESRequestHandlerList.h"

BESDataResponseHandler::BESDataResponseHandler( string name )
    : BESResponseHandler( name )
{
}

BESDataResponseHandler::~BESDataResponseHandler( )
{
}

/** @brief executes the command 'get data for &lt;def_name&gt;' by 
 * executing the request for each container in the specified definition
 *
 * For each container in the specified defnition go to the request
 * handler for that container and have it add to the OPeNDAP DDS data response
 * object. The data response object is created within this method and passed
 * to the request handler list.
 *
 * @param dhi structure that holds request and response information
 * @throws BESHandlerException if there is a problem building the
 * response object
 * @throws BESResponseException upon fatal error building the response
 * object
 * @see _BESDataHandlerInterface
 * @see DDS
 * @see BESRequestHandlerList
 * @see BESDefine
 */
void
BESDataResponseHandler::execute( BESDataHandlerInterface &dhi )
{
    // NOTE: It is the responsbility of the specific request handler to set
    // the BaseTypeFactory. It is set to NULL here
    _response = new DDS( NULL, "virtual" ) ;
    BESRequestHandlerList::TheList()->execute_each( dhi ) ;
}

/** @brief transmit the response object built by the execute command
 *
 * If a response object was built then transmit it using the send_data
 * method on the specified transmitter object.
 *
 * @param transmitter object that knows how to transmit specific basic types
 * @param dhi structure that holds the request and response information
 * @see DDS
 * @see BESTransmitter
 * @see _BESDataHandlerInterface
 */
void
BESDataResponseHandler::transmit( BESTransmitter *transmitter,
                               BESDataHandlerInterface &dhi )
{
    if( _response )
    {
	DDS *dds = dynamic_cast<DDS *>(_response) ;
	transmitter->send_data( *dds, dhi ) ;
    }
}

BESResponseHandler *
BESDataResponseHandler::DataResponseBuilder( string handler_name )
{
    return new BESDataResponseHandler( handler_name ) ;
}
