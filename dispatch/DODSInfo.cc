// DODSInfo.cc

// 2004 Copyright University Corporation for Atmospheric Research

#include <sstream>
#include <iostream>
#include <fstream>

using std::ostringstream ;
using std::cout ;
using std::ifstream ;

#include "DODSInfo.h"
#include "TheDODSKeys.h"

/** @brief constructs a DODSInfo object for the specified type.
 *
 * @param otype type of data represented by this response object
 */
DODSInfo::DODSInfo( ObjectType otype )
    : _strm( 0 ),
      _buffered( true ),
      _header( false ),
      _is_http( true ),
      _otype( otype )
{
}

/** @brief constructs a DODSInfo object for the specified type where the
 * output is specified as either http or not.
 *
 * @param is_http is the output to be in http format
 * @param otype type of data represented by this response object
 */
DODSInfo::DODSInfo( bool is_http, ObjectType otype )
    : _strm( 0 ),
      _buffered( true ),
      _header( false ),
      _is_http( is_http ),
      _otype( otype )
{
}

DODSInfo::~DODSInfo()
{
    if( _buffered && _strm ) delete _strm ;
}

/** @brief initialize the response object to determine if the information is
 * to be buffered or not.
 *
 * Using the key passed, if the key is the empty string then the key in the
 * dods initialization file DODS.Info.Buffered is found to determine if the
 * information is to be buffered or not. If the key is not empty then it is
 * used as the key in the dods initialization file to determine whether
 * buffering should occur or not.
 *
 * If the key is set to true, True, TRUE, yes, Yes, or YES then the
 * information will be buffered, otherwise it will not be buffered.
 *
 * If the information is not to be buffered then the output stream is set to
 * cout.
 *
 * @param key key in the dods initialization file used to determine if
 * information is to be buffered or not.
 */
void
DODSInfo::initialize( string key )
{
    if( key == "" )
    {
	key = "DODS.Info.Buffered" ;
    }

    bool found = false ;
    string b = TheDODSKeys->get_key( key, found ) ;
    if( b == "true" || b == "True" || b == "TRUE" ||
	b == "yes" || b == "Yes" || b == "YES" )
    {
	_strm = new ostringstream ;
	_buffered = true ;
    }
    else
    {
	_strm = &cout ;
	_buffered = false ;
    }
}

/** @brief add data to this informational object. If buffering is not set then
 * the information is output directly to the output stream.
 *
 * @param s information to be added to this response object
 */
void
DODSInfo::add_data( const string &s )
{
    if( !_header && !_buffered && _is_http )
    {
	set_mime_text( stdout, _otype ) ;
	_header = true ;
    }
    (*_strm) << s ;
}

/** @brief add data from a file to the informational object.
 *
 * Adds data from a file to the informational object using the file
 * specified by the passed key string. The key is found from the dods
 * initialization file.
 *
 * If the key does not exist in the initialization file then this
 * information is added to the informational object, no excetion is thrown.
 *
 * If the file does not exist then this information is added to the
 * informational object, no exception is thrown.
 *
 * @param key Key from the initialization file specifying the file to be
 * @param name naem information to add to error messages
 * loaded.
 */
void
DODSInfo::add_data_from_file( const string &key, const string &name )
{
    bool found = false ;
    string file = TheDODSKeys->get_key( key, found ) ;
    if( found == false )
    {
	add_data( name + " file key " + key + " not found, information not available\n" ) ;
    }
    else
    {
	ifstream ifs( file.c_str() ) ;
	if( !ifs )
	{
	    add_data( name + " file " + file + "not found, information not available\n" ) ;
	}
	else
	{
	    char line[4096] ;
	    while( !ifs.eof() )
	    {
		ifs.getline( line, 4096 ) ;
		if( !ifs.eof() )
		{
		    add_data( line ) ;
		    add_data( "\n" ) ;
		}
	    }
	    ifs.close() ;
	}
    }
}

/** @brief print the information from this informational object to the
 * specified FILE descriptor
 *
 * If the information was not buffered then this method does nothing,
 * otherwise the information is output to the specified FILE descriptor.
 *
 * @param out output to this file descriptor if information buffered.
 */
void
DODSInfo::print(FILE *out)
{
    if( _buffered )
    {
	fprintf( out, "%s", ((ostringstream *)_strm)->str().c_str() ) ;
    }
}

// $Log: DODSInfo.cc,v $
// Revision 1.5  2005/04/07 19:55:17  pwest
// added add_data_from_file method to allow for information to be added from a file, for example when adding help information from a file
//
// Revision 1.4  2004/12/15 17:39:03  pwest
// Added doxygen comments
//
// Revision 1.3  2004/09/09 17:17:12  pwest
// Added copywrite information
//
// Revision 1.2  2004/07/09 16:10:29  pwest
// Removed static var in DODSContainerPersistence to check if strict or nice
// had been already set. In DODSInfo only using one key to see if information
// buffered or not unless a different key is passed in from child class.
//
// Revision 1.1  2004/06/30 20:16:24  pwest
// dods dispatch code, can be used for apache modules or simple cgi script
// invocation or opendap daemon. Built during cedar server development.
//
