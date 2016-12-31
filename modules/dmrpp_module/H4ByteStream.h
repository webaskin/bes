
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

#ifndef _H4ByteStream_h
#define _H4ByteStream_h 1

#include <string>
#include <vector>

namespace dmrpp {

/**
 * This class is used to encapsulate the state and behavior needed for reading
 * chunked data associated with a DAP variable. In particular it is based on the
 * semantics of an hdf4:byteStream object, which is used to represent a chunk of
 * data in a (potentially complex) HDF4/HDF5 file.
 */
class H4ByteStream {
private:
    std::string d_data_url;
    unsigned long long d_size;
    unsigned long long d_offset;
    std::string d_md5;
    std::string d_uuid;
    bool d_is_read;
    std::vector<unsigned int> d_chunk_position_in_array;

    // These are used only during the libcurl callback;
    // they are not duplicated by the copy ctor or assignment
    // operator.
    unsigned long long d_bytes_read;
    char *d_read_buffer;
    unsigned long long d_read_buffer_size;


protected:

    void _duplicate(const H4ByteStream &bs) {
        // See above
    	d_bytes_read = 0;
    	d_read_buffer = 0;
    	d_read_buffer_size = 0;
    	d_is_read = false;
    	// These vars are easy to duplicate.
        d_size   = bs.d_size;
        d_offset = bs.d_offset;
        d_md5    = bs.d_md5;
        d_uuid   = bs.d_uuid;
        d_data_url   = bs.d_data_url;
        d_chunk_position_in_array = bs.d_chunk_position_in_array;
    }

public:

    H4ByteStream():
    	d_data_url(""),
    	d_size(0),
    	d_offset(0),
    	d_md5(""),
    	d_uuid(""),
		d_is_read(false),
    	d_bytes_read(0),
    	d_read_buffer(0),
    	d_read_buffer_size(0) { }

    H4ByteStream(
			std::string data_url,
			unsigned long long size,
			unsigned long long offset,
			std::string md5,
			std::string uuid,
			std::string position_in_array = ""):
			d_data_url(data_url),
			d_size(size),
			d_offset(offset),
    		d_md5(md5),
			d_uuid(uuid),
			d_is_read(false),
	    	d_bytes_read(0),
	    	d_read_buffer(0),
	    	d_read_buffer_size(0) {
    	ingest_position_in_array(position_in_array);
    }

    H4ByteStream(const H4ByteStream &h4bs){
    	_duplicate(h4bs);
    }

    virtual ~H4ByteStream(){
		delete[] d_read_buffer;
    }


    /**
     * @brief Get the size of this byteStream's data block
     */
    virtual unsigned long long get_size() const { return d_size; }
    /**
     * @brief Get the offset to this byteStream's data block
     */
    virtual unsigned long long get_offset() const { return d_offset; }
    /**
     * @brief Get the md5 string for this byteStream's data block
     */
    virtual std::string get_md5() const { return d_md5; }
    /**
     * @brief Get the uuid string for this byteStream's data block
     */
    virtual std::string get_uuid() const { return d_uuid; }

    /**
     * @brief Get the data url string for this byteStream's data block
     */
    virtual std::string get_data_url() const { return d_data_url; }
    /**
     * @brief Get the data url string for this byteStream's data block
     */
   virtual void set_data_url(const std::string &data_url) {
    	d_data_url = data_url;
    }

   /**
    * @brief Get the size of the data block associated with this byteStream.
    */
   virtual unsigned long long get_bytes_read() const { return d_bytes_read; }

   /**
    * @brief Set the size of this byteStream's data block
    * @param size Size of the data in bytes
    */
   virtual void set_bytes_read(unsigned long long bytes_read) { d_bytes_read = bytes_read; }

   /**
    * @brief Sets the size of the internal read buffer.
    *
    * The memory management of the read buffer is managed internal to this
    * class. This means that calling this method will release any previously
    * allocated read buffer memory and then allocates a new memory block. Since
    * this method always dumps the exiting read buffer the bytes_read counter is
    * set to zero.
    *
    * @param size Size of the internal read buffer.
    */
   virtual void rbuf_size(unsigned long long size) {

		// Calling delete on a null pointer is fine, so we don't need to check
		// to see if this is the first call.
		delete[] d_read_buffer;
		d_read_buffer_size = 0;

		d_read_buffer = new char[size];
		d_read_buffer_size = size;
		set_bytes_read(0);
   }

   virtual void set_rbuf_to_size() { rbuf_size(d_size);}

   /**
    * Returns a pointer to the memory buffer for this byteStream. The
    * return value is NULL if no memory has been allocated.
    */
   virtual char *get_rbuf() {
   	return d_read_buffer;
   }

   /**
    * Returns the size, in bytes, of the read buffer for this byteStream.
    */
   virtual unsigned long long get_rbuf_size() {
   	return d_read_buffer_size;
   }

    /**
     * Parses the passed string as a set of space separated integer values
     * and stores them in the internal vector.
     * Since this is essentially a setter method any previous postion_in_array
     * content is discarded.
     * If the passed string parameter is empty then nothing is done.
     */
    virtual void ingest_position_in_array(std::string pia);
    virtual std::string get_curl_range_arg_string();
    virtual void dump(std::ostream & strm) const;
    virtual void read();
    virtual std::string to_string();
    virtual std::vector<unsigned int> get_position_in_array() const { return d_chunk_position_in_array;}
};

} // namespace dmrpp

#endif // _H4ByteStream_h