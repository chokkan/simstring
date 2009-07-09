#include <stdint.h>
#include <algorithm>
#include <cmath>
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

namespace dastring
{

/**
 * A writer for an N-gram database.
 *  @param  char_tmpl       The type of a character.
 *  @param  value_tmpl      The value type, which must be a basic type.
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

public:
    /**
     * Constructs an object.
     *  @param  ngram_length    The length of n-grams.
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

    bool empty()
    {
        return m_indices.empty();
    }

    void clear()
    {
        m_indices.clear();
    }

    int max_length() const
    {
        return (int)m_indices.size();
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

        // Resize the index array for the number of the n-grams because we
        // builds an index for each n-gram number.
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
    bool store(const std::string& base) const
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
    bool store(const string_type& name, const index_type& index) const
    {
        // Open the database file with binary mode.
        std::ofstream ofs(name.c_str(), std::ios::binary);
        if (ofs.fail()) {
            return false;
        }

        // Open a CDBM writer.
        cdbpp::builder dbw(ofs);
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
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef uint32_t value_type;
    /// The type representing a character.
    typedef typename string_type::value_type char_type;
    /// 
    typedef ngramdb_writer_base<string_tmpl, uint32_t, ngram_generator_tmpl> base_type;

protected:
    std::string m_name;
    std::ofstream m_ofs;
    int m_num_entries;

public:
    /**
     * Constructs an object.
     */
    writer_base(const ngram_generator_type& gen)
        : base_type(gen), m_num_entries(0)
    {
    }

    /**
     * Destructs an object.
     */
    virtual ~writer_base()
    {
        close();
    }

    void open(const std::string& name)
    {
        m_name = name;
        m_num_entries = 0;
        m_ofs.open(name.c_str(), std::ios::binary);
        this->write_header(m_ofs);
    }

    void close()
    {
        if (!m_name.empty()) {
            this->store(m_name);
        }
        if (m_ofs.is_open()) {
            this->write_header(m_ofs);
        }
        m_name.clear();
    }

    /**
     * Inserts a string to the database.
     *  @param  key         The string to be inserted.
     */
    bool insert(const string_type& key)
    {
        value_type off = (value_type)(std::streamoff)m_ofs.tellp();
        m_ofs.write(key.c_str(), sizeof(char_type) * (key.length()+1));
        ++m_num_entries;
        return base_type::insert(key, off);
    }

protected:
    void write_header(std::ofstream& ofs)
    {
        uint32_t num_entries = m_num_entries;
        uint32_t max_length = (uint32_t)this->max_length();

        ofs.seekp(0);
        m_ofs.write("SSDB", 4);
        m_ofs.write(reinterpret_cast<char*>(&num_entries), sizeof(num_entries));
        m_ofs.write(reinterpret_cast<char*>(&max_length), sizeof(max_length));
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
        int xmin = query.min_length();
        int xmax = query.max_length();
        if (m_max_length < xmax) xmax = m_max_length;

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

protected:
    memory_mapped_file m_strings;

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
        m_strings.open(name, std::ios::in);

        // Check the file header.
        const char* p = m_strings.const_data();
        size_t size = m_strings.size();
        if (size < 12 || std::strncmp(p, "SSDB", 4) != 0) {
            return false;
        }
        p += 4;
        num_entries = *reinterpret_cast<const uint32_t*>(p);
        p += 4;
        max_length = *reinterpret_cast<const uint32_t*>(p);

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
        const char* strings = m_strings.const_data();
        for (it = results.begin();it != results.end();++it) {
            ngrams_type xgrams;
            const char_type* xstr = reinterpret_cast<const char_type*>(strings + *it);
            *ins = xstr;
        }
    }
};

};
