#include <stdint.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ngram.h"
#include "query.h"
#include "cdbpp.h"
#include "memory_mapped_file.h"

#define	DASTRING_NAME           "DAString"
#define	DASTRING_COPYRIGHT      "Copyright (c) 2009 Naoaki Okazaki"
#define	DASTRING_MAJOR_VERSION  0
#define DASTRING_MINOR_VERSION  2
#define DASTRING_STREAM_VERSION 1

namespace dastring
{

enum {
    BYTEORDER_CHECK = 0x62445371,
};


/**
 * A writer for an n-gram database.
 *  @param  string_tmpl             The type of a string.
 *  @param  value_tmpl              The value type.
 *                                  This is required to be an integer type.
 *  @param  ngram_generator_tmpl    The type of an n-gram generator.
 */
template <
    class string_tmpl,
    class value_tmpl,
    class ngram_generator_tmpl
>
class ngramdb_writer_base
{
public:
    /// The type representing a string.
    typedef string_tmpl string_type;
    /// The type of values associated with key strings.
    typedef value_tmpl value_type;
    /// The function type for generating n-grams from a key string.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The type representing a character.
    typedef typename string_type::value_type char_type;
    /// The type of an array of n-grams.
    typedef std::vector<string_type> ngrams_type;
    /// The vector type of values associated with an n-gram.
    typedef std::vector<value_type> values_type;
    /// The type implementing an index (associations from n-grams to values).
    typedef std::map<string_type, values_type> index_type;
    /// The vector of indices for different n-gram sizes.
    typedef std::vector<index_type> indices_type;

protected:
    /// The vector of indices.
    indices_type m_indices;
    /// The n-gram generator.
    const ngram_generator_type& m_gen;
    /// The error message.
    std::stringstream m_error;

public:
    /**
     * Constructs an object.
     *  @param  gen             The n-gram generator.
     */
    ngramdb_writer_base(const ngram_generator_type& gen)
        : m_gen(gen)
    {
    }

    /**
     * Destructs an object.
     */
    virtual ~ngramdb_writer_base()
    {
    }

    /**
     * Clears the database.
     */
    void clear()
    {
        m_indices.clear();
        m_error.str("");
    }

    /**
     * Checks whether the database is empty.
     *  @return bool    \c true if the database is empty, \c false otherwise.
     */
    bool empty()
    {
        return m_indices.empty();
    }

    /**
     * Returns the maximum length of keys in the n-gram database.
     *  @return int     The maximum length of keys.
     */
    int max_length() const
    {
        return (int)m_indices.size();
    }

    /**
     * Checks whether an error has occurred.
     *  @return bool    \c true if an error has occurred.
     */
    bool fail() const
    {
        return !m_error.str().empty();
    }

    /**
     * Returns an error message.
     *  @return std::string The string of the error message.
     */
    std::string error() const
    {
        return m_error.str();
    }

    /**
     * Inserts a string to the n-gram database.
     *  @param  key         The key string.
     *  @param  value       The value associated with the string.
     */
    bool insert(const string_type& key, const value_type& value)
    {
        // Generate n-grams from the key string.
        ngrams_type ngrams;
        m_gen(key, std::back_inserter(ngrams));
        if (ngrams.empty()) {
            return false;
        }

        // Resize the index array for the number of the n-grams;
        // we build an index for each n-gram number.
        if (m_indices.size() < ngrams.size()) {
            m_indices.resize(ngrams.size());
        }
        index_type& index = m_indices[ngrams.size()-1];

        // Store the associations from the n-grams to the value.
        typename ngrams_type::const_iterator it;
        for (it = ngrams.begin();it != ngrams.end();++it) {
            const string_type& ngram = *it;
            typename index_type::iterator iti = index.find(ngram);
            if (iti == index.end()) {
                // Create a new posting array.
                values_type v(1);
                v[0] = value;
                index.insert(typename index_type::value_type(ngram, v));
            } else {
                // Append the value to the existing posting array.
                iti->second.push_back(value);
            }
        }

        return true;
    }

    /**
     * Stores the n-gram database to files.
     *  @param  name        The prefix of file names.
     */
    bool store(const std::string& base)
    {
        for (int i = 0;i < (int)m_indices.size();++i) {
            if (!m_indices[i].empty()) {
                std::stringstream ss;
                ss << base << '.' << i+1 << ".cdb";
                bool b = this->store(ss.str(), m_indices[i]);
                if (!b) {
                    return false;
                }
            }
        }

        return true;
    }

protected:
    bool store(const string_type& name, const index_type& index)
    {
        // Open the database file with binary mode.
        std::ofstream ofs(name.c_str(), std::ios::binary);
        if (ofs.fail()) {
            m_error << "Failed to open a file for writing: " << name;
            return false;
        }

        try {
            // Open a CDB++ writer.
            cdbpp::builder dbw(ofs);

            // Put associations: n-gram -> values.
            typename index_type::const_iterator it;
            for (it = index.begin();it != index.end();++it) {
                // Put an association from an n-gram to its values. 
                dbw.put(
                    it->first.c_str(),
                    sizeof(char_type) * it->first.length(),
                    &it->second[0],
                    sizeof(it->second[0]) * it->second.size()
                    );
            }

        } catch (const cdbpp::builder_exception& e) {
            m_error << "CDB++ error: " << e.what();
            return false;
        }

        return true;
    }
};



template <
    class string_tmpl,
    class ngram_generator_tmpl
>
class writer_base :
    public ngramdb_writer_base<string_tmpl, uint32_t, ngram_generator_tmpl>
{
public:
    /// The type representing a string.
    typedef string_tmpl string_type;
    /// The type of values associated with key strings.
    typedef uint32_t value_type;
    /// The function type for generating n-grams from a key string.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The type representing a character.
    typedef typename string_type::value_type char_type;
    /// The type of the base class.
    typedef ngramdb_writer_base<string_tmpl, uint32_t, ngram_generator_tmpl> base_type;

protected:
    std::string m_name;
    std::ofstream m_ofs;
    int m_num_entries;

public:
    /**
     * Constructs an object.
     *  @param  gen             The n-gram generator.
     */
    writer_base(const ngram_generator_type& gen)
        : base_type(gen), m_num_entries(0)
    {
    }

    /**
     * Constructs an object by opening a database.
     *  @param  gen             The n-gram generator.
     *  @param  name            The name of the database.
     */
    writer_base(
        const ngram_generator_type& gen,
        const std::string& name
        )
        : base_type(gen), m_num_entries(0)
    {
        this->open(name);
    }

    /**
     * Destructs an object.
     */
    virtual ~writer_base()
    {
        close();
    }

    /**
     * Opens a database.
     *  @param  name            The name of the database.
     *  @return bool            \c true if the database is successfully opened,
     *                          \c false otherwise.
     */
    bool open(const std::string& name)
    {
        m_num_entries = 0;

        // Open the master file for writing.
        m_ofs.open(name.c_str(), std::ios::binary);
        if (m_ofs.fail()) {
            this->m_error << "Failed to open a file for writing: " << name;
            return false;
        }

        // Reserve the region for a file header.
        if (!this->write_header(m_ofs)) {
            m_ofs.close();
            return false;
        }

        m_name = name;
        return true;
    }

    /**
     * Closes the database.
     *  @param  name            The name of the database.
     *  @return bool            \c true if the database is successfully opened,
     *                          \c false otherwise.
     */
    bool close()
    {
        bool b = true;

        // Write the n-gram database to files.
        if (!m_name.empty()) {
            b &= this->store(m_name);
        }

        // Finalize the file header.
        if (m_ofs.is_open()) {
            b &= this->write_header(m_ofs);
        }

        // Close the file.
        m_ofs.close();

        // Initialize the members.
        m_name.clear();
        m_num_entries = 0;
        return b;
    }

    /**
     * Inserts a string to the database.
     *  @param  key         The string to be inserted.
     */
    bool insert(const string_type& key)
    {
        // This will be the offset address to access the key string.
        value_type off = (value_type)(std::streamoff)m_ofs.tellp();

        // Write the key string to the master file.
        m_ofs.write(key.c_str(), sizeof(char_type) * (key.length()+1));
        if (m_ofs.fail()) {
            this->m_error << "Failed to write a string to the master file.";
            return false;
        }
        ++m_num_entries;

        // Insert the n-grams of the key string to the database.
        return base_type::insert(key, off);
    }

protected:
    bool write_header(std::ofstream& ofs)
    {
        uint32_t num_entries = m_num_entries;
        uint32_t max_length = (uint32_t)this->max_length();
        uint32_t size = (uint32_t)m_ofs.tellp();

        // Seek to the beginning of the master file, to which the file header
        // is to be written.
        ofs.seekp(0);
        if (ofs.fail()) {
            this->m_error << "Failed to seek the file pointer for the master file.";
            return false;
        }

        // Write the file header.
        m_ofs.write("SSDB", 4);
        write_uint32(size);
        write_uint32(DASTRING_STREAM_VERSION);
        write_uint32(BYTEORDER_CHECK);
        write_uint32(num_entries);
        write_uint32(max_length);
        if (ofs.fail()) {
            this->m_error << "Failed to write a file header to the master file.";
            return false;
        }

        return true;
    }

    inline void write_uint32(uint32_t value)
    {
        m_ofs.write(reinterpret_cast<const char *>(&value), sizeof(value));
    }
};



template <
    class string_tmpl,
    class value_tmpl
>
class ngramdb_reader_base
{
public:
    typedef string_tmpl string_type;
    typedef value_tmpl value_type;
    typedef typename string_type::value_type char_type;
    typedef std::basic_stringstream<char_type> stringstream_type;
    typedef std::basic_fstream<char_type> fstream_type;
    typedef std::vector<string_type> ngrams_type;
    typedef std::set<value_type> results_type;
    //typedef std::map<value_type, int> candidates_type;
    typedef cdbpp::cdbpp index_type;

    struct result_similarity_type
    {
        value_type value;
        double sim;

        result_similarity_type()
            : value(0), sim(0)
        {
        }

        result_similarity_type(value_type v, double s)
            : value(v), sim(s)
        {
        }
    };

    typedef std::vector<result_similarity_type> results_similarity_type;
    
protected:
    struct posting_type
    {
        int num;
        const value_type* values;

        friend bool operator<(
            const posting_type& x, 
            const posting_type& y
            )
        {
            return (x.num < y.num);
        }
    };
    typedef std::vector<posting_type> postings_type;

    struct database_type
    {
        memory_mapped_file  image;
        index_type          index;
    };

    struct candidate_type
    {
        value_type  value;
        int         num;

        candidate_type(value_type v, int n)
            : value(v), num(n)
        {
        }
    };
    typedef std::vector<candidate_type> candidates_type;

protected:
    typedef std::vector<database_type> databases_type;
    databases_type m_dbs;
    int m_max_length;
    std::string m_name;

public:
    ngramdb_reader_base()
    {
    }

    virtual ~ngramdb_reader_base()
    {
    }

    void open(const std::string& name, int max_length)
    {
        m_name = name;
        m_max_length = max_length;
        m_dbs.resize(max_length);
    }

    void close()
    {
        m_name.clear();
        //m_dbs.clear();
    }

    template <class query_type>
    void search(const query_type& query, results_type& results)
    {
        int i;
        const int qlen = query.length();

        // Allocate a vector of postings corresponding to n-gram queries.
        postings_type posts(qlen);

        // Compute the range of n-gram lengths for the candidate strings;
        // in other words, we do not have to search for strings whose n-gram
        // lengths are out of this range.
        const int xmin = std::max(query.min_length(), 1);
        const int xmax = std::min(query.max_length(), m_max_length);

        // Loop for each length in the range.
        for (int xlen = xmin;xlen <= xmax;++xlen) {
            // Access to the n-gram index for the length.
            index_type& index = open_index(m_name, xlen);
            if (!index.is_open()) {
                // Ignore an empty index.
                continue;
            }

            // Search for string entries that match to each query n-gram.
            // Note that we do not traverse each entry here, but only obtain
            // the number of and the pointer to the entries.
            typename query_type::const_iterator it;
            for (it = query.begin(), i = 0;it != query.end();++it, ++i) {
                size_t vsize;
                const void *values = index.get(
                    it->c_str(),
                    sizeof(char_type) * it->length(),
                    &vsize
                    );
                posts[i].num = (int)(vsize / sizeof(value_type));
                posts[i].values = reinterpret_cast<const value_type*>(values);
            }

            // Sort the query n-grams by ascending order of their frequencies.
            // This reduces the number of initial candidates.
            std::sort(posts.begin(), posts.end());

            // The minimum number of n-gram matches required for the query.
            const int mmin = query.min_match(xlen);
            // A candidate must match to one of n-grams in these queries.
            const int min_queries = qlen - mmin + 1;

            // Collect candidates that match to the initial queries.
            candidates_type cands;
            for (i = 0;i < min_queries;++i) {
                candidates_type tmp;
                typename candidates_type::const_iterator itc = cands.begin();
                const value_type* p = posts[i].values;
                const value_type* last = posts[i].values + posts[i].num;

                while (itc != cands.end() || p != last) {
                    if (itc == cands.end() || (p != last && itc->value > *p)) {
                        tmp.push_back(candidate_type(*p, 1));
                        ++p;
                    } else if (p == last || (itc != cands.end() && itc->value < *p)) {
                        tmp.push_back(candidate_type(itc->value, itc->num));
                        ++itc;
                    } else {
                        tmp.push_back(candidate_type(itc->value, itc->num+1));
                        ++itc;
                        ++p;
                    }
                }
                std::swap(cands, tmp);
            }

            // No initial candidate is found.
            if (cands.empty()) {
                continue;
            }

            // Count the number of matches with remaining queries.
            for (;i < qlen;++i) {
                candidates_type tmp;
                typename candidates_type::const_iterator itc;
                const value_type* first = posts[i].values;
                const value_type* last = posts[i].values + posts[i].num;

                // For each active candidate.
                for (itc = cands.begin();itc != cands.end();++itc) {
                    int num = itc->num;
                    if (std::binary_search(first, last, itc->value)) {
                        ++num;
                    }

                    if (mmin <= num) {
                        // This candidate has sufficient matches.
                        results.insert(itc->value);
                    } else if (num + (qlen - i - 1) >= mmin) {
                        // This candidate still has the chance.
                        tmp.push_back(candidate_type(itc->value, num));
                    }
                }
                std::swap(cands, tmp);

                // Exit the loop if all candidates are pruned.
                if (cands.empty()) {
                    break;
                }
            }

            if (!cands.empty()) {
                throw std::logic_error("error in candidate pruning");
            }
        }
    }

    template <class query_type>
    void search_similarity(const query_type& query, results_similarity_type& results)
    {
        int i;
        const int qlen = query.length();

        // Allocate a vector of postings corresponding to n-gram queries.
        postings_type posts(qlen);

        // Compute the range of n-gram lengths for the candidate strings;
        // in other words, we do not have to search for strings whose n-gram
        // lengths are out of this range.
        const int xmin = std::max(query.min_length(), 1);
        const int xmax = std::min(query.max_length(), m_max_length);

        // Loop for each length in the range.
        for (int xlen = xmin;xlen <= xmax;++xlen) {
            // Access to the n-gram index for the length.
            index_type& index = open_index(m_name, xlen);
            if (!index.is_open()) {
                // Ignore an empty index.
                continue;
            }

            // Search for string entries that match to each query n-gram.
            // Note that we do not traverse each entry here, but only obtain
            // the number of and the pointer to the entries.
            typename query_type::const_iterator it;
            for (it = query.begin(), i = 0;it != query.end();++it, ++i) {
                size_t vsize;
                const void *values = index.get(
                    it->c_str(),
                    sizeof(char_type) * it->length(),
                    &vsize
                    );
                posts[i].num = (int)(vsize / sizeof(value_type));
                posts[i].values = reinterpret_cast<const value_type*>(values);
            }

            // Sort the query n-grams by ascending order of their frequencies.
            // This reduces the number of initial candidates.
            std::sort(posts.begin(), posts.end());

            // The minimum number of n-gram matches required for the query.
            const int mmin = query.min_match(xlen);
            // A candidate must match to one of n-grams in these queries.
            const int min_queries = qlen - mmin + 1;

            // Collect candidates that match to the initial queries.
            candidates_type cands;
            for (i = 0;i < min_queries;++i) {
                candidates_type tmp;
                typename candidates_type::const_iterator itc = cands.begin();
                const value_type* p = posts[i].values;
                const value_type* last = posts[i].values + posts[i].num;

                while (itc != cands.end() || p != last) {
                    if (itc == cands.end() || (p != last && itc->value > *p)) {
                        tmp.push_back(candidate_type(*p, 1));
                        ++p;
                    } else if (p == last || (itc != cands.end() && itc->value < *p)) {
                        tmp.push_back(candidate_type(itc->value, itc->num));
                        ++itc;
                    } else {
                        tmp.push_back(candidate_type(itc->value, itc->num+1));
                        ++itc;
                        ++p;
                    }
                }
                std::swap(cands, tmp);
            }

            // No initial candidate is found.
            if (cands.empty()) {
                continue;
            }

            // Count the number of matches with remaining queries.
            for (;i < qlen;++i) {
                candidates_type tmp;
                typename candidates_type::const_iterator itc;
                const value_type* first = posts[i].values;
                const value_type* last = posts[i].values + posts[i].num;

                // For each active candidate.
                for (itc = cands.begin();itc != cands.end();++itc) {
                    int num = itc->num;
                    if (std::binary_search(first, last, itc->value)) {
                        ++num;
                    }

                    if (num + (qlen - i - 1) >= mmin) {
                        // This candidate still has the chance.
                        tmp.push_back(candidate_type(itc->value, num));
                    }
                }
                std::swap(cands, tmp);

                // Exit the loop if all candidates are pruned.
                if (cands.empty()) {
                    break;
                }
            }

            double norm = std::sqrt((double)xlen * qlen);
            for (typename candidates_type::const_iterator itc = cands.begin();itc != cands.end();++itc) {
                if (mmin <= itc->num) {
                    results.push_back(result_similarity_type(itc->value, itc->num / norm));
                }
            }
        }
    }

protected:
    index_type& open_index(const std::string& base, int length)
    {
        database_type& db = m_dbs[length-1];
        if (!db.index.is_open()) {
            std::stringstream ss;
            ss << base << '.' << length << ".cdb";
            db.image.open(ss.str().c_str(), std::ios::in);
            if (db.image.is_open()) {
                db.index.open(db.image.data(), db.image.size());
            }
        }

        return db.index;
    }
};


template <
    class string_tmpl
>
class reader_base
    : public ngramdb_reader_base<string_tmpl, uint32_t>
{
public:
    /// The type representing a character.
    typedef string_tmpl string_type;
    typedef typename string_type::value_type char_type;
    typedef std::vector<string_type> ngrams_type;
    /// 
    typedef ngramdb_reader_base<string_tmpl, uint32_t> base_type;

    struct record_type
    {
        std::string str;
        double sim;

        record_type() : sim(0) {}
        record_type(const std::string& _str, double _sim) : str(_str), sim(_sim) {}
    };

protected:
    std::vector<char_type> m_strings;

public:
    /**
     * Constructs an object.
     */
    reader_base()
    {
    }

    /**
     * Destructs an object.
     */
    virtual ~reader_base()
    {
        close();
    }

    bool open(const std::string& name)
    {
        uint32_t num_entries, max_length;

        // Open the master file.
        std::ifstream ifs(name.c_str(), std::ios_base::in | std::ios_base::binary);
        if (ifs.fail()) {
            return false;
        }
        ifs.seekg(0, std::ios_base::end);
        size_t size = (size_t)ifs.tellg();
        ifs.seekg(0, std::ios_base::beg);
        
        m_strings.resize(size);
        ifs.read(&m_strings[0], size);
        ifs.close();

        // Check the file header.
        const char* p = &m_strings[0];
        if (size < 12 || std::strncmp(p, "SSDB", 4) != 0) {
            return false;
        }
        p += 4;

        // Check the chunk size.
        if (size != read_uint32(p)) {
            return false;
        }
        p += 4;

        // Check the version.
        if (DASTRING_STREAM_VERSION != read_uint32(p)) {
            return false;
        }
        p += 4;

        // Check the byte order.
        if (BYTEORDER_CHECK != read_uint32(p)) {
            return false;
        }
        p += 4;

        num_entries = read_uint32(p);
        p += 4;
        max_length = read_uint32(p);

        base_type::open(name, (int)max_length);
        return true;
    }

    void close()
    {
        base_type::close();
    }

    template <class query_type, class insert_iterator>
    void retrieve(const query_type& query, insert_iterator ins)
    {
        typename base_type::results_type results;
        base_type::search(query, results);
        typename base_type::results_type::const_iterator it;
        const char* strings = &m_strings[0];
        for (it = results.begin();it != results.end();++it) {
            ngrams_type xgrams;
            const char_type* xstr = reinterpret_cast<const char_type*>(strings + *it);
            *ins = xstr;
        }
    }

    template <class query_type, class container_type>
    void retrieve_similarity(const query_type& query, container_type& cont)
    {
        typename base_type::results_similarity_type results;
        base_type::search_similarity(query, results);
        typename base_type::results_similarity_type::const_iterator it;
        const char* strings = &m_strings[0];
        for (it = results.begin();it != results.end();++it) {
            ngrams_type xgrams;
            const char_type* xstr = reinterpret_cast<const char_type*>(strings + it->value);
            cont.push_back(record_type(xstr, it->sim));
        }
    }

protected:
    inline uint32_t read_uint32(const char* p) const
    {
        return *reinterpret_cast<const uint32_t*>(p);
    }
};

};
