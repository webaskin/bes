// BESXMLDefineCommand.cc

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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301
 
// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "BESXMLDefineCommand.h"
#include "BESContainerStorageList.h"
#include "BESContainerStorage.h"
#include "BESXMLUtils.h"
#include "BESUtil.h"
#include "BESResponseNames.h"
#include "BESDataNames.h"
#include "BESSyntaxUserError.h"
#include "BESDebug.h"

BESXMLDefineCommand::BESXMLDefineCommand( const BESDataHandlerInterface &base_dhi )
    : BESXMLCommand( base_dhi ),
      _default_constraint( "" )
{
}

/** @brief parse a show command. No properties or children elements
 *
    &lt;define name="d" space="default"&gt;
        &lt;container name="c"&gt;
            &lt;constraint&gt;a valid ce&lt;/constraint&gt;
            &lt;attributes&gt;list of attributes&lt;/attributes&gt;
        &lt;/container&gt;
	&lt;aggregate handler="someHandler" cmd="someCommand" /&gt;
    &lt;/define&gt; 
 *
 * Requires the name property. The space property is optional. Requires at
 * least one container element. The container element requires the name
 * property. The constraint and attribute elements of container are
 * optional. The aggregate element is optional.
 *
 * @param node xml2 element node pointer
 */
void
BESXMLDefineCommand::parse_request( xmlNode *node )
{
    string value ;		// element value, should not be any
    string def_name ;		// definition name
    string def_space ;		// definition storage space
    string action ;		// element name, which is the request action
    map<string, string> props ;	// element properties. Should contain name
    				// and optionally space

    BESXMLUtils::GetNodeInfo( node, action, value, props ) ;
    if( action != DEFINE_RESPONSE_STR )
    {
	string err = "The specified command " + action
		     + " is not a set context command" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    _dhi.action = DEFINE_RESPONSE ;

    def_name = props["name"] ;
    if( def_name.empty() )
    {
	string err = action + " command: definition name missing" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    _dhi.data[DEF_NAME] = def_name ;
    _str_cmd = (string)"define " + def_name ;

    def_space = props["space"] ;
    if( !def_space.empty() )
    {
	_str_cmd += " in " + def_space ;
    }
    _dhi.data[STORE_NAME] = def_space ;

    int num_containers = 0 ;
    string child_name ;
    string child_value ;
    props.clear() ;
    xmlNode *child_node =
	BESXMLUtils::GetFirstChild( node, child_name, child_value, props ) ;
    while( child_node )
    {
	if( child_name == "constraint" )
	{
	    // default constraint for all containers
	    _default_constraint = child_value ;
	}
	else if( child_name == "container" )
	{
	    handle_container_element( action, child_node, child_value, props ) ;
	    num_containers++ ;
	}
	else if( child_name == "aggregate" )
	{
	    handle_aggregate_element( action, child_node, child_value, props ) ;
	}

	// get the next child element
	props.clear() ;
	child_name.clear() ;
	child_value.clear() ;
	child_node = BESXMLUtils::GetNextChild( child_node, child_name,
						child_value, props ) ;
    }

    if( num_containers < 1 )
    {
	string err = action + "The define element must contain at least "
	             + "one container element" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    _str_cmd += " as " ;
    bool first = true ;
    vector<string>::iterator i = _containers.begin() ;
    vector<string>::iterator e = _containers.end() ;
    for( ; i != e; i++ )
    {
	if( !first ) _str_cmd += "," ;
	_str_cmd += (*i) ;
	first = false ;
    }
    if( _constraints.size() )
    {
	_str_cmd += " with " ;
	first = true ;
	map<string,string>::iterator ci = _constraints.begin() ;
	map<string,string>::iterator ce = _constraints.end() ;
	for( ; ci != ce; ci++ )
	{
	    if( !first ) _str_cmd += "," ;
	    _str_cmd += (*ci).first + ".constraint=\"" + (*ci).second + "\"" ;
	    first = false ;
	    string attrs = _attributes[(*ci).first] ;
	    if( !attrs.empty() )
	    {
		_str_cmd += "," + (*ci).first + ".attributes=\"" + attrs + "\"";
	    }
	}
    }
    _str_cmd += ";" ;

    // now that we've set the action, go get the response handler for the
    // action
    BESXMLCommand::set_response() ;
}

/** @brief handle a container element of the define element
 *
        &lt;container name="c"&gt;
            &lt;constraint&gt;a valid ce&lt;/constraint&gt;
            &lt;attributes&gt;list of attributes&lt;/attributes&gt;
        &lt;/container&gt;
 *
 * The name is required. constraint and attribute sub elements are optional
 *
 * @param action we are working on
 * @param node xml node element for the container
 * @param value a value of the container element, should be empty
 * @param props properties of the container element
 */
void
BESXMLDefineCommand::handle_container_element( const string &action,
					       xmlNode *node,
					       const string &value,
					       map<string,string> &props )
{
    string name = props["name"] ;
    if( name.empty() )
    {
	string err = action + " command: container element missing name prop" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    _containers.push_back( name ) ;

    string space = props["space"] ;
    _stores[name] = space ;

    bool have_constraint = false ;
    bool have_attributes = false ;
    string child_name ;
    string child_value ;
    string constraint ;
    string attributes ;
    map<string,string> child_props ;
    xmlNode *child_node =
	BESXMLUtils::GetFirstChild( node, child_name, child_value, child_props);
    while( child_node )
    {
	if( child_name == "constraint" )
	{
	    if( child_props.size() )
	    {
		string err = action + " command: constraint element "
				    + "should not contain properties" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    if( child_value.empty() )
	    {
		string err = action + " command: attributes element "
				    + "missing value" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    if( have_constraint )
	    {
		string err = action + " command: container element "
				    + "contains multiple constraint elements" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    have_constraint = true ;
	    _constraints[name] = child_value ;
	}
	else if( child_name == "attributes" )
	{
	    if( child_props.size() )
	    {
		string err = action + " command: attributes element "
				    + "should not contain properties" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    if( child_value.empty() )
	    {
		string err = action + " command: attributes element "
				    + "missing value" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    if( have_attributes )
	    {
		string err = action + " command: container element "
				    + "contains multiple attributes elements" ;
		throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
	    }
	    have_attributes = true ;
	    _attributes[name] = child_value ;
	}

	// get the next child element
	props.clear() ;
	child_name.clear() ;
	child_value.clear() ;
	child_node = BESXMLUtils::GetNextChild( child_node, child_name,
						child_value, props ) ;
    }
}

/** @brief handle an aggregate element of the define element
 *
	&lt;aggregate handler="someHandler" cmd="someCommand" /&gt;
 *
 * The handler and cmd properties are required
 *
 * @param action we are working on
 * @param node xml node element for the container
 * @param value a value of the container element, should be empty
 * @param props properties of the container element
 */
void
BESXMLDefineCommand::handle_aggregate_element( const string &action,
					       xmlNode *node,
					       const string &value,
					       map<string,string> &props )
{
    string handler = props["handler"] ;
    string cmd = props["cmd"] ;
    if( handler.empty() )
    {
	string err = action + " command: must specify aggregation handler" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }
    if( cmd.empty() )
    {
	string err = action + " command: must specify aggregation cmd" ;
	throw BESSyntaxUserError( err, __FILE__, __LINE__ ) ;
    }

    _dhi.data[AGG_HANDLER] = handler ;
    _dhi.data[AGG_CMD] = cmd ;
    _str_cmd += " aggregate using " + handler + " by " + cmd ;
}

/** @brief prepare the define command by making sure the containers exist
 */
void
BESXMLDefineCommand::prep_request()
{
    vector<string>::iterator i = _containers.begin() ;
    vector<string>::iterator e = _containers.end() ;
    for( ; i != e; i++ )
    {
	// look for the specified container
	BESContainer *c = 0 ;

	// first see if a particilar store is being used
	string store = _stores[(*i)] ;
	if( !store.empty() )
	{
	    BESContainerStorage *cs = 
		BESContainerStorageList::TheList()->find_persistence( store ) ;
	    if( cs ) c = cs->look_for( (*i) ) ;
	}
	else
	{
	    c = BESContainerStorageList::TheList()->look_for( (*i) ) ;
	}
	if( !c && BESContainerStorageList::TheList()->isnice() == false )
	{
	    string s = (string)"Could not find the container "
		       + (*i) ;
	    throw BESSyntaxUserError( s, __FILE__, __LINE__ ) ;
	}

	string constraint = _constraints[(*i)] ;
	string attrs = _attributes[(*i)] ;
	if( constraint.empty() ) constraint = _default_constraint ;
	c->set_constraint( constraint ) ;
	c->set_attributes( attrs ) ;
	_dhi.containers.push_back( c ) ;
	BESDEBUG( "xml", "define using container: " << endl << *c << endl ) ;
    }
}

/** @brief dumps information about this object
 *
 * Displays the pointer value of this instance
 *
 * @param strm C++ i/o stream to dump the information to
 */
void
BESXMLDefineCommand::dump( ostream &strm ) const
{
    strm << BESIndent::LMarg << "BESXMLDefineCommand::dump - ("
			     << (void *)this << ")" << endl ;
    BESIndent::Indent() ;
    BESXMLCommand::dump( strm ) ;
    BESIndent::UnIndent() ;
}

BESXMLCommand *
BESXMLDefineCommand::CommandBuilder( const BESDataHandlerInterface &base_dhi )
{
    return new BESXMLDefineCommand( base_dhi ) ;
}

