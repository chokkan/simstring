/*
 *      SimString queries.
 *
 * Copyright (c) 2009, Naoaki Okazaki
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

namespace simstring { namespace query {



/**
 * The base class for queries.
 *  @param  string_tmpl             The string class.
 *  @param  ngram_generator_tmpl    The n-gram generator class.
 */
template <class string_tmpl, class ngram_generator_tmpl>
class query_base
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef std::vector<string_type> ngrams_type;
    typedef typename ngrams_type::const_iterator const_iterator;

protected:
    double m_th;
    string_type m_qstr;
    ngrams_type m_qgram;
    const ngram_generator_type& m_gen;

public:
    query_base(const ngram_generator_type& gen)
        : m_gen(gen), m_th(1.)
    {
    }

    query_base(const ngram_generator_type& gen, const string_type& query, double th = 1.)
        : m_gen(gen)
    {
        this->set(query, th);
    }

    query_base(const query_base& rho) :
        m_qstr(rho.m_qstr), m_qgram(rho.m_qgram),
        m_gen(rho.m_gen), m_th(rho.m_th)
    {
    }

    virtual ~query_base()
    {
    }

    bool empty() const
    {
        return m_qgram.empty();
    }

    const string_type& str() const
    {
        return m_qstr;
    }

    int length() const
    {
        return (int)m_qgram.size();
    }

    const_iterator begin() const
    {
        return m_qgram.begin();
    }

    const_iterator end() const
    {
        return m_qgram.end();
    }

    inline void set(const string_type& query, double th = 1.)
    {
        m_qgram.clear();
        m_qstr = query;
        m_gen(query, std::back_inserter(m_qgram));
        m_th = th;
    }

    template <class iterator_type>
    inline void set(iterator_type first, iterator_type last, double th = 1.)
    {
        m_qgram.clear();
        m_qgram.insert(first, last);
        m_th = th;
    }
};



template <class string_tmpl, class ngram_generator_tmpl>
class exact :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    exact(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    exact(const ngram_generator_type& gen, const string_type& query)
        : base_type(gen, query)
    {
    }

    exact(const exact& rho)
        : base_type(rho)
    {
    }

    virtual ~exact()
    {
    }

    inline int min_length() const
    {
        return this->length();
    }

    inline int max_length() const
    {
        return this->length();
    }

    inline int min_match(int length) const
    {
        return this->length();
    }
};



template <class string_tmpl, class ngram_generator_tmpl>
class cosine :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    cosine(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    cosine(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    virtual ~cosine()
    {
    }

    inline int min_length() const
    {
        return (int)std::ceil(this->m_th * this->m_th * this->length());
    }

    inline int max_length() const
    {
        return (int)std::floor(this->length() / (this->m_th * this->m_th));
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(this->m_th * std::sqrt((double)this->length() * length));
    }
};



template <class string_tmpl, class ngram_generator_tmpl>
class dice :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    dice(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    dice(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    virtual ~dice()
    {
    }


    inline int min_length() const
    {
        return (int)std::ceil(this->m_th * this->length() / (2. - this->m_th));
    }

    inline int max_length() const
    {
        return (int)std::floor((2. - this->m_th) * this->length() / this->m_th);
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(0.5 * this->m_th * (this->length() + length));
    }
};



template <class string_tmpl, class ngram_generator_tmpl>
class jaccard :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    jaccard(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    jaccard(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    virtual ~jaccard()
    {
    }

    inline int min_length() const
    {
        return (int)std::ceil(this->m_th * this->length());
    }

    inline int max_length() const
    {
        return (int)std::floor(this->length() / this->m_th);
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(this->m_th * (this->length() + length) / (1 + this->m_th));
    }
};



template <class string_tmpl, class ngram_generator_tmpl>
class overlap :
    public query_base<string_tmpl, ngram_generator_tmpl>
{
public:
    typedef string_tmpl string_type;
    typedef ngram_generator_tmpl ngram_generator_type;
    typedef query_base<string_tmpl, ngram_generator_tmpl> base_type;

public:
    overlap(const ngram_generator_type& gen)
        : base_type(gen)
    {
    }

    overlap(const ngram_generator_type& gen, const string_type& query, double th)
        : base_type(gen, query, th)
    {
    }

    virtual ~overlap()
    {
    }

    inline int min_length() const
    {
        return 1;
    }

    inline int max_length() const
    {
        return (int)INT_MAX;
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(this->m_th * std::min(this->length(), length));
    }
};



}; };

#endif/*__SIMSTRING_QUERY_H__*/
