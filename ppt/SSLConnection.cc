// SSLConnection.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.org> and Jose Garcia <jgarcia@ucar.org>
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

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>

using std::cout ;
using std::endl ;
using std::flush ;

#include "SSLConnection.h"
#include "PPTException.h"

SSLConnection::SSLConnection( )
    : _method( NULL ),
      _context( NULL ),
      _connection( NULL ),
      _connected( false )
{
}
    
SSLConnection::~SSLConnection()
{
}

void
SSLConnection::closeConnection()
{
    if( _connected && _connection )
    {
	if( SSL_shutdown( _connection ) == 0 )
	{
	    SSL_shutdown( _connection ) ;
	}
    }
    SSL_clear( _connection ) ;

    if( _context ) SSL_CTX_free( _context ) ; _context = NULL ;
    _connected = false ;
    // FIX: Do I need to delete/free the connection object as well?
    _connection = NULL ;
}

void
SSLConnection::send( const string &buf )
{
    if( _connected )
    {
	int len = SSL_write( _connection, (void *)buf.c_str(), buf.length() ) ;
	if( len <= 0 )
	{
	    string msg = "FAILED to write to SSL connection\n" ;
	    msg += ERR_error_string( ERR_get_error(), NULL ) ;
	    throw PPTException( msg ) ;
	}
    }
}

bool
SSLConnection::receive( ostream *strm )
{
    bool isDone = false ;
    if( _connected )
    {
	char retbuf[1024] ;
	int retlen = SSL_read( _connection, (void *)retbuf, 1024 ) ;
	if( retlen <= 0 )
	{
	    if( retlen == 0 )
	    {
		isDone = true ;
	    }
	    else
	    {
		string msg = "FAILED to read from SSL connection\n" ;
		msg += ERR_error_string( ERR_get_error(), NULL ) ;
		throw PPTException( msg ) ;
	    }
	}
	else
	{
	    retbuf[retlen] = '\0' ;
	    *strm << retbuf << flush ;
	}
    }

    return isDone ;
}
