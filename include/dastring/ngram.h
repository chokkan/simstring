#ifndef __NGRAM_H__
#define __NGRAM_H__

#include <map>
#include <sstream>
#include <string>

namespace dastring
{

template <
    class string_type,
    class insert_iterator
    >
static void
generate_ngrams(
    const string_type& str,
    insert_iterator ins,
    int n
    )
{
    typedef typename string_type::value_type char_type;
    typedef std::basic_stringstream<char_type> stringstream_type;
    typedef std::map<string_type, int> ngram_stat_type;
    const char_type mark = (char_type)'_';

    // Append marks for begin/end of the string.
    string_type src;
    for (int i = 0;i < n-1;++i) src += mark;
    src += str;
    for (int i = 0;i < n-1;++i) src += mark;
    
    // Count n-grams in the string.
    ngram_stat_type stat;
    for (int i = 0;i < (int)src.length()-n+1;++i) {
        std::string ngram = src.substr(i, n);
        typename ngram_stat_type::iterator it = stat.find(ngram);
        if (it == stat.end()) {
            stat[ngram] = 1;
        } else {
            ++it->second;
        }
    }

    // Convert the n-gram stat to a vector.
    typename ngram_stat_type::const_iterator it;
    for (it = stat.begin();it != stat.end();++it) {
        *ins = it->first;
        // Append numbers if the same n-gram occurs more than once.
        for (int i = 2;i <= it->second;++i) {
            stringstream_type ss;
            ss << it->first << i;
            *ins = it->first;
        }
    }
}

struct ngram_generator
{
    int m_n;

    ngram_generator() : m_n(2)
    {
    }

    ngram_generator(int n) : m_n(n)
    {
    }

    template <class string_type, class insert_iterator>
    void operator()(const string_type& str, insert_iterator ins) const
    {
        generate_ngrams(str, ins, m_n);
    }
};

};

#endif/*__NGRAM_H__*/
