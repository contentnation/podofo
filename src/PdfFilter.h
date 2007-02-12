/***************************************************************************
 *   Copyright (C) 2006 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _PDF_FILTER_H_
#define _PDF_FILTER_H_

#include "PdfDefines.h"

namespace PoDoFo {

class PdfDictionary;
class PdfOutputStream;

#define FILTER_INTERNAL_BUFFER_SIZE 4096

/** Every filter in PoDoFo has to implement this interface.
 * 
 *  The two methods Encode and Decode have to be implemented 
 *  for every filter.
 *
 *  The output buffers are malloc'ed in the functions and have
 *  to be free'd by the caller.
 */
class PODOFO_API PdfFilter {
 public:
    /** Construct and initialize a new filter
     */
    PdfFilter();

    /** All classes with virtual functions need a virtual destructor
     */
    virtual ~PdfFilter() {};

    /** Check wether encoding is implemented for this filter.
     * 
     *  \returns true if the filter is able to encode data
     */
    virtual bool CanEncode() const = 0; 

    /** Encodes a buffer using a filter. The buffer will malloc'ed and
     *  has to be free'd by the caller.
     *
     *  This function uses BeginEncode()/EncodeBlock()/EndEncode()
     *  internally, so it's not safe to use when progressive encoding
     *  is in progress.
     *
     *  \param pInBuffer input buffer
     *  \param lInLen    length of the input buffer
     *  \param ppOutBuffer pointer to the buffer of the encoded data
     *  \param plOutLen pointer to the length of the output buffer
     */
    void Encode( const char* pInBuffer, long lInLen, char** ppOutBuffer, long* plOutLen ) const;

    /** Begin progressively encoding data using this filter.
     *
     *  This method sets the filter's output stream and may
     *  perform other operations defined by particular filter
     *  implementations. It calls BeginEncodeImpl().
     *
     *  \param pOutput encoded data will be written to this stream.
     *
     *  Call EncodeBlock() to encode blocks of data and use EndEncode
     *  to finish the encoding process.
     *
     *  \see EncodeBlock
     *  \see EndEncode
     */
    inline void BeginEncode( PdfOutputStream* pOutput );

    /** Encode a block of data and write it to the PdfOutputStream
     *  specified by BeginEncode. Ownership of the block is not taken
     *  and remains with the caller.
     *
     *  The filter implementation need not immediately process the buffer,
     *  and might internally buffer some or all of it. However, if it does
     *  this the buffer's contents will be copied, so it is guaranteed to be
     *  safe to free the passed buffer after this call returns.
     *
     *  This method is a wrapper around EncodeBlockImpl().
     *
     *  BeginEncode() must be called before this function.
     *
     *  \param pBuffer pointer to a buffer with data to encode
     *  \param lLen length of data to encode.
     *
     *  Call EndEncode() after all data has been encoded
     *
     *  \see BeginEncode
     *  \see EndEncode
     */
    inline void EncodeBlock( const char* pBuffer, long lLen );

    /**
     *  Finish encoding of data and reset the stream's state.
     *
     *  \see BeginEncode
     *  \see EncodeBlock
     */
    inline void EndEncode();

    /** Check wether the decoding is implemented for this filter.
     * 
     *  \returns true if the filter is able to decode data
     */
    virtual bool CanDecode() const = 0; 

    /** Decodes a buffer using a filter. The buffer will malloc'ed and
     *  has to be free'd by the caller.
     *  
     *  \param pInBuffer input buffer
     *  \param lInLen    length of the input buffer
     *  \param ppOutBuffer pointer to the buffer of the decoded data
     *  \param plOutLen pointer to the length of the output buffer
     *  \param pDecodeParms optional pointer to an decode parameters dictionary
     *                      containing additional information to decode the data.
     *                      This pointer must be NULL if no decode parameter dictionary
     *                      is available.
     */
    virtual void Decode( const char* pInBuffer, long lInLen, char** ppOutBuffer, long* plOutLen, const PdfDictionary* pDecodeParms = NULL ) const = 0;

    /** Type of this filter.
     *  \returns the type of this filter
     */
    virtual EPdfFilter GetType() const = 0;

    inline PdfOutputStream* GetStream() const throw() { return m_pOutputStream; }

 protected:
    /**
     * Indicate that the filter has failed, and will be non-functional until BeginEncode()
     * is next called. Call this instead of EndEncode() if something went wrong. It clears
     * the stream output but otherwise does nothing.
     *
     * After this method is called futher calls to EncodeBlock() and
     * EndEncode() before the next BeginEncode() are guaranteed to throw
     * without calling their virtual implementations.
     */
    inline void FailEncode() throw();

    /** Real implementation of `BeginEncode()'. NEVER call this method directly.
     *
     *  By default this function does nothing. If your filter needs to do setup for encoding,
     *  you should override this method.
     *
     *  PdfFilter ensures that a valid stream is available when this method is called, and
     *  that EndEncode() was called since the last BeginEncode()/EncodeBlock().
     *
     * \see BeginEncode */
    virtual void BeginEncodeImpl( ) { }

    /** Real implementation of `EncodeBlock()'. NEVER call this method directly.
     *
     *  You must override this method to encode the buffer passed by the caller.
     *
     *  You are not obliged to immediately process any or all of the data in
     *  the passed buffer, but you must ensure that you have processed it and
     *  written it out by the end of EndEncodeImpl(). You must copy the buffer
     *  if you're going to store it, as ownership is not transferred to the
     *  filter and the caller may free the buffer at any time.
     *
     *  PdfFilter ensures that a valid stream is available when this method is
     *  called, ensures that BeginEncode() has been called, and ensures that
     *  EndEncode() has not been called since the last BeginEncode().
     *
     * \see EncodeBlock */
    virtual void EncodeBlockImpl( const char* pBuffer, long lLen ) =0;

    /** Real implementation of `EndEncode()'. NEVER call this method directly.
     *
     * By the time this method returns, all filtered data must be written to the stream
     * and the filter must be in a state where BeginEncode() can be safely called.
     *
     *  PdfFilter ensures that a valid stream is available when this method is
     *  called, and ensures that BeginEncodeImpl() has been called.
     *
     * \see EndEncode */
    virtual void EndEncodeImpl() { }

    unsigned char    m_buffer[FILTER_INTERNAL_BUFFER_SIZE];

 private:
    PdfOutputStream* m_pOutputStream;
};

void PdfFilter::BeginEncode( PdfOutputStream* pOutput )
{
    if ( m_pOutputStream )
        // oops, user didn't call EndEncode() on previous run!
        RAISE_ERROR( ePdfError_InternalLogic );
    m_pOutputStream = pOutput;
    BeginEncodeImpl();
}

void PdfFilter::EncodeBlock( const char* pBuffer, long lLen )
{
    if ( !m_pOutputStream )
        // oops, user forgot to call BeginEncode() or is using a failed filter
        RAISE_ERROR( ePdfError_InternalLogic );
    EncodeBlockImpl(pBuffer, lLen);
}

void PdfFilter::EndEncode()
{
    if ( !m_pOutputStream )
        // oops, user forgot to call BeginEncode() or is using a failed filter
        RAISE_ERROR( ePdfError_InternalLogic );
    EndEncodeImpl();
    m_pOutputStream = NULL;
}

void PdfFilter::FailEncode() throw()
{
    m_pOutputStream = NULL;
}


/** A factory to create a filter object for a filter GetType from the EPdfFilter enum.
 * 
 *  All filters should be created using this factory which does also caching of filter
 *  instances.
 */
class PODOFO_API PdfFilterFactory {
 public:
    /** Create a filter from an enum. 
     *  The filter is cached and may not be delted!
     *
     *  \param eFilter the GetType of filter that should be created.
     *
     *  \returns a new PdfFilter allocated using new or NULL if no 
     *           filter is available for this GetType.
     */
    static const PdfFilter* Create( const EPdfFilter eFilter );

 private:
    /** static cache of filter objects
     */
    static std::map<EPdfFilter,PdfFilter*> s_mapFilters;
};

};

#endif /* _PDF_FILTER_H_ */
