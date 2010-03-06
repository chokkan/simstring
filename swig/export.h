#ifndef __EXPORT_H__
#define __EXPORT_H__

#include <string>
#include <vector>

/** 
 * \addtogroup swig_interface SimString SWIG interface
 * @{
 *
 *  The SimString SWIG interface.
 */

/**
 * Similarity measures.
 */
enum {
    /// Exact matching.
    exact,
    /// Dice coefficient.
    dice,
    /// Cosine coefficient.
    cosine,
    /// Jaccard coefficient.
    jaccard,
    /// Overlap coefficient.
    overlap,
};

/**
 * SimString database writer.
 */
class writer
{
protected:
    void *m_dbw;
    void *m_gen;
    bool m_unicode;

public:
    /**
     * Creates a new database.
     *  This function creates an instance of SimString database writer
     *  for creating a new database. If this function failes to open
     *  the database, it throws SWIG_IOError.
     *  
     *  @param  filename    The database filename.
     *  @param  n           The unit of character n-grams.
     *  @param  be          \c true to represent a begin and end of strings
     *                      in character n-grams.
     *  @param  unicode     \c true to use Unicode mode. In Unicode mode,
     *                      wide (\c wchar_t) characters are used in n-grams.
     *  @throw  SWIG_IOError
     */
    writer(const char *filename, int n = 3, bool be = false, bool unicode = false);
    
    /**
     * Destructs the writer.
     *  Destructing a writer object automatically closes the database.
     *  @throw  SWIG_IOError
     */
    virtual ~writer();
    
    /**
     * Inserts a string into the database.
     *  @param  string      A string to be inserted to the database. This
     *                      argument must be a null-terminated byte stream.
     *                      If the database is created with Unicode mode, this
     *                      function assumes that the byte stream is encoded in
     *                      UTF-8, and converts it into a \c wchar_t string.
     *  @throw  SWIG_IOError
     */
    void insert(const char *string);
    
    /**
     * Closes the database.
     *  This function flushes and closes the database. If this function failes
     *  to close the database, it throws SWIG_IOError.
     *  @throw  SWIG_IOError
     */
    void close();
};

/**
 * SimString database reader.
 */
class reader
{
protected:
    void *m_dbr;

public:
    /**
     * Opens a database for retrieving strings.
     *  This function creates an instance of SimString database reader
     *  by opening an existing database. If this function failes to open
     *  the database, it throws SWIG_IOError.
     *
     *  @param  filename    The database filename.
     *  @throw  SWIG_IOError
     */
    reader(const char *filename);

    /**
     * Destructs the database reader.
     *  Destructing the reader object automatically closes the database.
     */
    virtual ~reader();

    /**
     * Retrieves strings that are similar to the query string.
     *  This function retrieves strings whose similarity with the query string
     *  are no smaller than a threshold. Before calling this function, set the
     *  similarity measure and threshold to \ref measure and \ref threshold
     *  attributes of the reader object.
     *  
     *  @param  query       The query string. This argument must be a
     *                      null-terminated byte stream. If the database was
     *                      created with Unicode mode, this function assumes
     *                      that the byte stream is encoded in UTF-8, and
     *                      converts it into a wchar_t string.
     *  @return             The array of strings retrieved for the query.
     *                      If the database was created with Unicode mode,
     *                      this function returns strings in UTF-8.
     *  @see    measure     The similarity function used by this function.
     *  @see    threshold   The similarity value used by this function.
     */
    std::vector<std::string> retrieve(const char *query);

    /**
     * Closes a database.
     */
    void close();

public:
    /**
     * Similarity measure.
     *  Specify a similarity measure for approximate string retrieval used
     *  by retrieve() function.
     *  @see    exact, cosine, dice, jaccard, overlap
     */
    int measure;

    /**
     * Threshold for the similarity measure.
     *  Specify a threshold for approximate string retrieval used by
     *  retrieve() function.
     */
    double threshold;
};

/** @} */

/**
@mainpage SimString SWIG interface

@section intro Introduction

This document describes a SWIG interface that bridges SimString with various
programing languages including Python and Ruby. Although SimString currently
distribution provides SWIG wrappers for Python and Ruby, it may be easy to
build libraries for other languages.

SimString module provides two simple classes ::writer and ::reader.
In the ::writer class, one can create a SimString database using the
constructor writer::writer, and call the member function writer::insert for
inserting a string into the database.
In the ::reader class, one can open an existing SimString database with the
constructor reader::reader, specify a similarity measure and threshold with
two attributes reader::measure and reader::threshold, and call the member
function reader::retrieve for performing approximate string matching.

SimString module always uses 8-bit null-terminated byte streams in
writer::insert and reader::retrieve functions. The encoding of byte streams
can be arbitrary, but must be UTF-8 for a database in Unicode mode.

@section api Documentation

- @ref swig_interface "SWIG interface"

@section language Language-specific Notes

@subsection language-ruby Ruby
- Because of the naming convention, the initial letter of a name is
  capitalized as follows:
  the module name (Simstring), class names (Writer and Reader),
  and similarity measures (Exact, Dice, Cosine, Jaccard, Overlap).

@section sample Sample Programs

@subsection python Python

A basic sample.

@include python/sample.py

A Unicode sample.

@include python/sample_unicode.py

@subsection ruby Ruby

A basic sample.

@include ruby/sample.rb

A Unicode sample.

@include ruby/sample_unicode.rb

*/

#endif/*__EXPORT_H__*/
