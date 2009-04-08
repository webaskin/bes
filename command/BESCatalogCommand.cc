// BESCatalogCommand.cc

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

#include "BESCatalogCommand.h"
#include "BESTokenizer.h"
#include "BESResponseHandlerList.h"
#include "BESSyntaxUserError.h"
#include "BESDataNames.h"
#include "BESResponseNames.h"

/** @brief knows how to parse a show catalog request
 *
 * This class knows how to parse a show catalog request, building a sub response
 * handler that actually knows how to build the requested response object.
 *
 * A show request looks like:
 *
 * show catalog [for &lt;container&gt;];
 *
 * where container is a container in the tree that the user wants look 
 * into further, a container returned from a previous show catalog call.
 *
 * This parse method creates the sub response handler that knows how to create
 * the specified information.
 *
 * @param tokenizer holds on to the list of tokens to be parsed
 * @param dhi structure that holds request and response information
 * @throws BESSyntaxUserError if the response handler does not exist
 */
BESResponseHandler *
BESCatalogCommand::parse_request( BESTokenizer &tokenizer,
                                   BESDataHandlerInterface &dhi )
{
    dhi.data[CATALOG_OR_INFO] = _cmd ;

    string my_token = parse_options( tokenizer, dhi ) ;
    if( my_token == "for" )
    {
	string container = tokenizer.get_next_token() ;
	if( container == ";" || container == "" )
	{
	    tokenizer.parse_error( container + " not expected" ) ;
	}
	dhi.data[CONTAINER] = tokenizer.remove_quotes( container ) ;
	my_token = tokenizer.get_next_token() ;
    }
    if( my_token != ";" )
    {
	tokenizer.parse_error( my_token + " not expected" ) ;
    }

    dhi.action = CATALOG_RESPONSE ;
    BESResponseHandler *retResponse =
	BESResponseHandlerList::TheList()->find_handler( CATALOG_RESPONSE ) ;
    if( !retResponse )
    {
	string err( "Command " ) ;
	err += _cmd ;
	err += " does not have a registered response handler" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    return retResponse ;
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESCatalogCommand::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESCatalogCommand::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESCommand::dump( strm ) ;
    BESIndent::UnIndent() ;
}

