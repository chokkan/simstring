/*
 *      SimString queries.
 *
 * Copyright (c) 2009,2010 Naoaki Okazaki
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the authors nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* $Id$ */


#ifndef __SIMSTRING_QUERY_H__
#define __SIMSTRING_QUERY_H__

#include <iterator>
#include <set>
#include <vector>
#include <limits.h>

#include "ngram.h"

namespace simstring { namespace query {



/**
 * The base class for various query types.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class query_base
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The type of an n-gram set.
    typedef std::vector<string_type> ngrams_type;
    /// The type of a const iterator for n-gram elements.
    typedef typename ngrams_type::const_iterator const_iterator;

protected:
    const ngram_generator_type& m_gen;
    double m_th;
    string_type m_qstr;
    ngrams_type m_qgram;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    query_base(const ngram_generator_type& gen)
        : m_gen(gen), m_th(1.)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     *  @param  th          The threshold.
     */
    query_base(
        const ngram_generator_type& gen,
        const string_type& query,
        double th = 1.
        )
        : m_gen(gen)
    {
        this->set(query, th);
    }

    /**
     * Constructs a query from another instance.
     *  @param  other       The other instance.
     */
    query_base(const query_base& other) :
        m_gen(other.m_gen), m_th(other.m_th),
        m_qstr(other.m_qstr), m_qgram(other.m_qgram)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~query_base()
    {
    }

    /**
     * Tests if the query is empty.
     *  @return bool        \c true if the query is empty, \c false otherwise.
     */
    bool empty() const
    {
        return m_qgram.empty();
    }

    /**
     * Returns the query string.
     *  @return const string_type&  The query string.
     */
    const string_type& str() const
    {
        return m_qstr;
    }

    /**
     * Returns the size of the query n-grams.
     *  @return int         The size of the query n-grams.
     */
    int size() const
    {
        return (int)m_qgram.size();
    }

    /**
     * Returns a random-access iterator to the first element
     *  of the query n-grams.
     *  @return const_iterator  A random-access iterator (for read-only)
     *                          addressing the first element in the query
     *                          n-grams or to the location succeeding an
     *                          empty element.
     */
    const_iterator begin() const
    {
        return m_qgram.begin();
    }

    /**
     * Returns a random-access iterator pointing just beyond the last element
     *  of the query n-grams.
     *  @return const_iterator  A random-access iterator (for read-only)
     *                          addressing the end of the element.
     */
    const_iterator end() const
    {
        return m_qgram.end();
    }

    /**
     * Set a query string.
     *  @param  query           A query string.
     *  @param  th              The threshold for approximate string matching.
     */
    inline void set(const string_type& query, double th = 1.)
    {
        m_qgram.clear();
        m_qstr = query;
        m_gen(query, std::back_inserter(m_qgram));
        m_th = th;
    }

    /**
     * Set a query from an n-gram set.
     *  @param  first           The iterator addressing to the first element
     *                          of the n-gram set.
     *  @param  last            The iterator addressing to the last element
     *                          of the n-gram set.
     *  @param  th              The threshold for approximate string matching.
     */
    template <class iterator_type>
    inline void set(iterator_type first, iterator_type last, double th = 1.)
    {
        m_qgram.clear();
        m_qgram.insert(first, last);
        m_th = th;
    }
};



/**
 * The base class for queries with exact matching.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class exact :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The base class.
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    exact(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     */
    exact(const ngram_generator_type& gen, const string_type& query)
        : base_type(gen, query)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~exact()
    {
    }

    /**
     * Returns the minimum size for retrieved strings.
     *  @return int         The minimum size.
     */
    inline int min_size() const
    {
        return this->size();
    }

    /**
     * Returns the maximum size for retrieved strings.
     *  @return int         The maximum size.
     */
    inline int max_size() const
    {
        return this->size();
    }

    /**
     * Returns the minimum number of overlaps required for retrieved strings.
     *  @return int         The minimum number of n-gram overlaps.
     */
    inline int min_match(int size) const
    {
        return this->size();
    }
};



/**
 * The base class for queries with cosine similarity measure.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class cosine :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The base class.
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    cosine(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     */
    cosine(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~cosine()
    {
    }

    /**
     * Returns the minimum size for retrieved strings.
     *  @return int         The minimum size.
     */
    inline int min_size() const
    {
        return (int)std::ceil(this->m_th * this->m_th * this->size());
    }

    /**
     * Returns the maximum size for retrieved strings.
     *  @return int         The maximum size.
     */
    inline int max_size() const
    {
        return (int)std::floor(this->size() / (this->m_th * this->m_th));
    }

    /**
     * Returns the minimum number of overlaps required for retrieved strings.
     *  @return int         The minimum number of n-gram overlaps.
     */
    inline int min_match(int size) const
    {
        return (int)std::ceil(this->m_th * std::sqrt((double)this->size() * size));
    }
};



/**
 * The base class for queries with dice similarity measure.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class dice :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The base class.
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    dice(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     */
    dice(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~dice()
    {
    }

    /**
     * Returns the minimum size for retrieved strings.
     *  @return int         The minimum size.
     */
    inline int min_size() const
    {
        return (int)std::ceil(this->m_th * this->size() / (2. - this->m_th));
    }

    /**
     * Returns the maximum size for retrieved strings.
     *  @return int         The maximum size.
     */
    inline int max_size() const
    {
        return (int)std::floor((2. - this->m_th) * this->size() / this->m_th);
    }

    /**
     * Returns the minimum number of overlaps required for retrieved strings.
     *  @return int         The minimum number of n-gram overlaps.
     */
    inline int min_match(int size) const
    {
        return (int)std::ceil(0.5 * this->m_th * (this->size() + size));
    }
};



/**
 * The base class for queries with jaccard similarity measure.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class jaccard :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The base class.
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    jaccard(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     */
    jaccard(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~jaccard()
    {
    }

    /**
     * Returns the minimum size for retrieved strings.
     *  @return int         The minimum size.
     */
    inline int min_size() const
    {
        return (int)std::ceil(this->m_th * this->size());
    }

    /**
     * Returns the maximum size for retrieved strings.
     *  @return int         The maximum size.
     */
    inline int max_size() const
    {
        return (int)std::floor(this->size() / this->m_th);
    }

    /**
     * Returns the minimum number of overlaps required for retrieved strings.
     *  @return int         The minimum number of n-gram overlaps.
     */
    inline int min_match(int size) const
    {
        return (int)std::ceil(this->m_th * (this->size() + size) / (1 + this->m_th));
    }
};



/**
 * The base class for queries with overlap similarity measure.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <
    class string_tmpl,
    class ngram_generator_tmpl = simstring::ngram_generator
>
class overlap :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    /// The type of a string.
    typedef string_tmpl string_type;
    /// The type of an n-gram generator.
    typedef ngram_generator_tmpl ngram_generator_type;
    /// The base class.
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     */
    overlap(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    /**
     * Constructs a query.
     *  @param  gen         The n-gram generator.
     *  @param  query       The query string.
     */
    overlap(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    /**
     * Destructs an instance.
     */
    virtual ~overlap()
    {
    }

    /**
     * Returns the minimum size for retrieved strings.
     *  @return int         The minimum size.
     */
    inline int min_size() const
    {
        return 1;
    }

    /**
     * Returns the maximum size for retrieved strings.
     *  @return int         The maximum size.
     */
    inline int max_size() const
    {
        return (int)INT_MAX;
    }

    /**
     * Returns the minimum number of overlaps required for retrieved strings.
     *  @return int         The minimum number of n-gram overlaps.
     */
    inline int min_match(int size) const
    {
        return (int)std::ceil(this->m_th * std::min(this->size(), size));
    }
};



}; };

#endif/*__SIMSTRING_QUERY_H__*/
