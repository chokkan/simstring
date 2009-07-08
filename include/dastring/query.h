#ifndef __SSDB_QUERY_H__
#define __SSDB_QUERY_H__

#include <iterator>
#include <set>
#include <vector>

namespace dastring { namespace query {



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
        this->set(query, th)
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
        : base_type(gen)
    {
        this->set(query);
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
        : base_type(gen)
    {
        this->set(query, th);
    }

    virtual ~cosine()
    {
    }

    inline void set(const string_type& query, double th)
    {
        base_type::set(query);
        m_th = th;
    }

    template <class iterator_type>
    inline void set(iterator_type first, iterator_type last, double th)
    {
        base_type::set(first, last);
        m_th = th;
    }

    inline int min_length() const
    {
        return (int)std::ceil(m_th * m_th * this->length());
    }

    inline int max_length() const
    {
        return (int)std::floor(this->length() / (m_th * m_th));
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(m_th * std::sqrt((double)this->length() * length));
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
        : base_type(gen)
    {
        this->set(query, th);
    }

    virtual ~dice()
    {
    }

    inline void set(const string_type& query, double th)
    {
        base_type::set(query);
        m_th = th;
    }

    template <class iterator_type>
    inline void set(iterator_type first, iterator_type last, double th)
    {
        base_type::set(first, last);
        m_th = th;
    }

    inline int min_length() const
    {
        return (int)std::ceil(m_th * this->length() / (2. - m_th));
    }

    inline int max_length() const
    {
        return (int)std::floor((2. - m_th) * this->length() / m_th);
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(0.5 * m_th * (this->length() + length));
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
        : base_type(gen)
    {
        this->set(query, th);
    }

    virtual ~jaccard()
    {
    }

    inline void set(const string_type& query, double th)
    {
        base_type::set(query);
        m_th = th;
    }

    template <class iterator_type>
    inline void set(iterator_type first, iterator_type last, double th)
    {
        base_type::set(first, last);
        m_th = th;
    }

    inline int min_length() const
    {
        return (int)std::ceil(m_th * this->length());
    }

    inline int max_length() const
    {
        return (int)std::floor(this->length() / m_th);
    }

    inline int min_match(int length) const
    {
        return (int)std::ceil(m_th * (this->length() + length) / (1 + m_th));
    }
};


}; };

#endif/*__SSDB_QUERY_H__*/
