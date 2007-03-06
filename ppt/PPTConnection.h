// PPTConnection.h

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

#ifndef PPTConnection_h
#define PPTConnection_h 1

#include "Connection.h"

class Socket ;

class PPTConnection : public Connection
{
private:
    int				_timeout ;
    				PPTConnection()
				    : _timeout( 0 ) {}
protected:
				PPTConnection( int timeout )
				    : _timeout( timeout ) {}

    void			writeBuffer( const string &buffer ) ;
    int				readBuffer( char *inBuff ) ;
    int				readBufferNonBlocking( char *inBuff ) ;
public:
    virtual			~PPTConnection() {}

    virtual void		initConnection() = 0 ;
    virtual void		closeConnection() = 0 ;

    virtual void		send( const string &buffer ) ;
    virtual void		sendExit() ;
    virtual bool		receive( ostream *strm = 0 ) ;

    virtual void		dump( ostream &strm ) const ;
} ;

#endif // PPTConnection_h

