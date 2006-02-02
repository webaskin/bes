// OPeNDAPGetCommand.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.org>
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

#include "OPeNDAPGetCommand.h"
#include "OPeNDAPTokenizer.h"
#include "DODSResponseHandlerList.h"
#include "DODSDefineList.h"
#include "DODSDefine.h"
#include "OPeNDAPParserException.h"
#include "OPeNDAPDataNames.h"

/** @brief knows how to parse a get request
 *
 * This class knows how to parse a get request, building a sub response
 * handler that actually knows how to build the requested response
 * object, such as das, dds, data, ddx, etc...
 *
 * A get request looks like:
 *
 * get &lt;response_type&gt; for &lt;def_name&gt; [return as &lt;ret_name&gt;;
 *
 * where response_type is the type of response being requested, for example
 * das, dds, dods.
 * where def_name is the name of the definition that has already been created,
 * like a view into the data
 * where ret_name is the method of transmitting the response. This is
 * optional.
 *
 * This parse method creates the sub response handler, retrieves the
 * definition information and finds the return object if one is specified.
 *
 * @param tokenizer holds on to the list of tokens to be parsed
 * @param dhi structure that holds request and response information
 * @throws OPeNDAPParserException if there is a problem parsing the request
 */
DODSResponseHandler *
OPeNDAPGetCommand::parse_request( OPeNDAPTokenizer &tokenizer,
                                  DODSDataHandlerInterface &dhi )
{
    string def_name ;

    string my_token = parse_options( tokenizer, dhi ) ;

    /* First we will make sure that the developer has not over-written this
     * command to work with a sub command. In other words, they have a new
     * command called "get something" that is parsed differently than the
     * default get command. Look up get.something
     */

    string newcmd = _cmd + "." + my_token ;
    OPeNDAPCommand *cmdobj = OPeNDAPCommand::find_command( newcmd ) ;
    if( cmdobj && cmdobj != OPeNDAPCommand::TermCommand )
    {
	return cmdobj->parse_request( tokenizer, dhi ) ;
    }

    /* No subcommand - so proceed as a default get command
     */
    DODSResponseHandler *retResponse =
	DODSResponseHandlerList::TheList()->find_handler( my_token ) ;
    if( !retResponse )
    {
	string err( "Command " ) ;
	err += _cmd + " " + my_token ;
	err += " does not have a registered response handler" ;
	throw OPeNDAPParserException( err ) ;
    }
    dhi.action = my_token ;

    my_token = tokenizer.get_next_token() ;
    if( my_token != "for" )
    {
	tokenizer.parse_error( my_token + " not expected\n" ) ;
    }
    else
    {
	def_name = tokenizer.get_next_token() ;

	my_token = tokenizer.get_next_token() ;
	if( my_token == "return" )
	{
	    my_token = tokenizer.get_next_token() ;
	    if( my_token != "as" )
	    {
		tokenizer.parse_error( my_token + " not expected, expecting \"as\"" ) ;
	    }
	    else
	    {
		my_token = tokenizer.get_next_token() ;
		dhi.data[RETURN_CMD] =
		    tokenizer.remove_quotes( my_token ) ;

		my_token = tokenizer.get_next_token() ;
		if( my_token != ";" )
		{
		    tokenizer.parse_error( my_token + " not expected, expecting ';'" ) ;
		}
	    }
	}
	else
	{
	    if( my_token != ";" )
	    {
		tokenizer.parse_error( my_token + " not expected, expecting \"return\" or ';'" ) ;
	    }
	}
    }

    DODSDefine *d = DODSDefineList::TheList()->find_def( def_name ) ;
    if( !d )
    {
	throw OPeNDAPParserException( (string)"Unable to find definition " + def_name ) ;
    }

    DODSDefine::containers_iterator i = d->first_container() ;
    DODSDefine::containers_iterator ie = d->end_container() ;
    while( i != ie )
    {
	dhi.containers.push_back( *i ) ;
	i++ ;
    }
    dhi.data[AGG_CMD] = d->aggregation_command ;
    dhi.data[AGG_HANDLER] = d->aggregation_handler ;

    return retResponse ;
}

// $Log: OPeNDAPGetCommand.cc,v $
