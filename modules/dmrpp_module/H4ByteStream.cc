
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of the BES

// Copyright (c) 2016 OPeNDAP, Inc.
// Author: Nathan Potter <ndp@opendap.org>
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
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <sstream>
#include <cstdlib>
#include <BESDebug.h>
#include <BESError.h>

#include "H4ByteStream.h"
#include "DmrppUtil.h"

using namespace std;

namespace dmrpp {

void
H4ByteStream::ingest_position_in_array(string pia){
	BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  BEGIN" << " -  Parsing: " << pia << "'" << endl);

	if(!pia.empty()){
		BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  string '"<< pia << "' is not empty." << endl);
		// Clear the thing if it's got stuff in it.
		if(d_chunk_position_in_array.size())
			d_chunk_position_in_array.clear();

		string open_sqr_brckt("[");
		string close_sqr_brckt("]");
		string comma(",");
		size_t strPos = 0;
		string strVal;

		// Drop leading square bracket
		if (!pia.compare(0, 1, open_sqr_brckt)){
			pia.erase(0, 1);
			BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  dropping leading '[' result: '"<< pia << "'" << endl);
		}

		// Drop trailing square bracket
		if (!pia.compare(pia.length()-1, 1, close_sqr_brckt)){
			pia.erase(pia.length()-1, 1);
			BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  dropping trailing ']' result: '"<< pia << "'" << endl);
		}

		// Is it multi-valued? We check for commas  to find out.
		if((strPos = pia.find(comma)) != string::npos){
			BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  Position string appears to contain multiple values..." << endl);
			// Process comma delimited content
			while ((strPos = pia.find(comma)) != string::npos) {
				strVal = pia.substr(0, strPos);
				BESDEBUG("dmrpp", __PRETTY_FUNCTION__ << " -  Parsing: " << strVal << endl);
				d_chunk_position_in_array.push_back(strtol(strVal.c_str(),NULL,10));
				pia.erase(0, strPos + comma.length());
			}
		}
		// A single value, remains after multi-valued processing, or there was only
		// Every a single value, so let's ingest that!
		BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  Position string appears to have a single value..." << endl);
		d_chunk_position_in_array.push_back(strtol(pia.c_str(),NULL,10));

	}
	BESDEBUG("dmrpp", "H4ByteStream::ingest_position_in_array() -  END" << " -  Parsed " << pia << "'" << endl);

}

/**
 * @brief Returns a curl range argument.
 * The libcurl requires a string argument for range-ge activitys, this method
 * constructs one in the required syntax from the offset and size information
 * for this byteStream.
 *
 */
std::string
H4ByteStream::get_curl_range_arg_string(){
    ostringstream range;   // range-get needs a string arg for the range
    range << d_offset << "-" << d_offset + d_size - 1;
    return range.str();
}

/**
 *
 *  unsigned long long d_size;
 *  unsigned long long d_offset;
 *  std::string d_md5;
 *  std::string d_uuid;
 *  std::string d_data_url;
 *  std::vector<unsigned int> d_chunk_position_in_array;
 *
 */
void
H4ByteStream::dump(ostream &oss) const {
    oss << "H4ByteStream";
    oss << "[data_url='" <<  d_data_url  << "']";
    oss << "[offset=" <<  d_offset  << "]";
    oss << "[size=" <<  d_size  << "]";
    oss << "[md5=" <<  d_md5  << "]";
    oss << "[uuid=" <<  d_uuid  << "]";
    oss << "[chunk_position_in_array=(";
	for(unsigned long i=0; i<d_chunk_position_in_array.size();i++){
		if(i) oss << ",";
		oss << d_chunk_position_in_array[i];
	}
	oss << ")]";
}

void
H4ByteStream::read(){
	if(d_is_read){
	    BESDEBUG("dmrpp", "H4ByteStream::"<< __func__ <<"() - Already been read! Returning." << endl);
		return;
	}

    set_rbuf_to_size();
    // First cut at subsetting; read the whole thing and then subset that.
    BESDEBUG("dmrpp", "H4ByteStream::"<< __func__ <<"() - Reading  " << get_size() << " bytes from "<< get_data_url() << ": " << get_curl_range_arg_string() << endl);
    curl_read_byteStream(get_data_url(), get_curl_range_arg_string(), dynamic_cast<H4ByteStream*>(this));
    // If the expected byte count was not read, it's an error.
    if (get_size() != get_bytes_read()) {
        ostringstream oss;
        oss << "H4ByteStream: Wrong number of bytes read for '" << to_string() << "'; expected " << get_size()
            << " but found " << get_bytes_read() << endl;
        throw BESError(oss.str(), BES_INTERNAL_ERROR, __FILE__, __LINE__);
    }

}






std::string
H4ByteStream::to_string(){
	std::ostringstream oss;
	dump(oss);
	return oss.str();
}




} // namespace dmrpp

