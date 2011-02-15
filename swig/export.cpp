#include <string>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <errno.h>
#include <iconv.h>
#include <simstring/simstring.h>

#include "export.h"

#define UTF16   "UTF-16LE"
#define UTF32   "UTF-32LE"

#ifdef  USE_LIBICONV_GNU
#define iconv_open      libiconv_open
#define iconv_convert   libiconv_convert
#define iconv_close     libiconv_close
#endif/*USE_LIBICONV_GNU*/

#ifndef ICONV_CONST
#define ICONV_CONST
#endif/*ICONV_CONST*/

template <class source_type, class destination_type>
bool iconv_convert(iconv_t cd, const source_type& src, destination_type& dst)
{
    typedef typename source_type::value_type source_char_type;
    typedef typename destination_type::value_type destination_char_type;
    
    const char *inbuf = reinterpret_cast<const char *>(src.c_str());
    size_t inbytesleft = sizeof(source_char_type) * src.length();
    while (inbytesleft > 0) {
        char buffer[1024];
    char *p = buffer;
    size_t outbytesleft = 1024;
    int ret = iconv(cd, (ICONV_CONST char **)&inbuf, &inbytesleft, &p, &outbytesleft);
    if (ret == -1 && errno != E2BIG) {
        return false;
    }
    dst.append(
        reinterpret_cast<const destination_char_type*>(buffer),
        (1024 - outbytesleft) / sizeof(destination_char_type)
        );
    }
    return true;
}

int translate_measure(int measure)
{
    switch (measure) {
    case exact:
        return simstring::exact;
    case dice:
        return simstring::dice;
    case cosine:
        return simstring::cosine;
    case jaccard:
        return simstring::jaccard;
    case overlap:
        return simstring::overlap;
    }
    throw std::invalid_argument("Unknown similarity measure specified");
}



typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<std::string, ngram_generator_type> writer_type;
typedef simstring::writer_base<std::wstring, ngram_generator_type> uwriter_type;
typedef simstring::reader reader_type;

writer::writer(const char *filename, int n, bool be, bool unicode)
    : m_dbw(NULL), m_gen(NULL), m_unicode(unicode)
{
    ngram_generator_type *gen = new ngram_generator_type(n, be);
    if (unicode) {
        uwriter_type *dbw = new uwriter_type(*gen, filename);
        if (dbw->fail()) {
            std::string message = dbw->error();
            delete dbw;
            delete gen;
            throw std::invalid_argument(message);
        }
    m_dbw = dbw;
    m_gen = gen;

    } else {
        writer_type *dbw = new writer_type(*gen, filename);
        if (dbw->fail()) {
            std::string message = dbw->error();
            delete dbw;
            delete gen;
            throw std::invalid_argument(message);
        }
    m_dbw = dbw;
    m_gen = gen;
    }
}

writer::~writer()
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);
    ngram_generator_type* gen = reinterpret_cast<ngram_generator_type*>(m_gen);
    
    dbw->close();
    if (dbw->fail()) {
        std::string message = dbw->error();
        delete dbw;
        delete gen;
        throw std::runtime_error(message);
    }
    delete dbw;
    delete gen;

    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    ngram_generator_type* gen = reinterpret_cast<ngram_generator_type*>(m_gen);
    
    dbw->close();
    if (dbw->fail()) {
        std::string message = dbw->error();
        delete dbw;
        delete gen;
        throw std::runtime_error(message);
    }
    delete dbw;
    delete gen;
    }
}

void writer::insert(const char *string)
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);

    std::wstring str;
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    iconv_convert(cd, std::string(string), str);
    iconv_close(cd);

    dbw->insert(str);
    if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
    }

    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    dbw->insert(string);
    if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
    }
    }
}

void writer::close()
{
    if (m_unicode) {
        uwriter_type* dbw = reinterpret_cast<uwriter_type*>(m_dbw);
        dbw->close();
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }

    } else {
        writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
        dbw->close();
        if (dbw->fail()) {
            throw std::runtime_error(dbw->error());
        }
    }
}



reader::reader(const char *filename)
    : m_dbr(NULL), measure(cosine), threshold(0.7)
{
    reader_type *dbr = new reader_type;

    if (!dbr->open(filename)) {
        delete dbr;
        throw std::invalid_argument("Failed to open the database");
    }

    m_dbr = dbr;
}

reader::~reader()
{
    this->close();
    delete reinterpret_cast<reader_type*>(m_dbr);
}

template <class insert_iterator_type>
void retrieve_thru(
    reader_type& dbr,
    const std::string& query,
    int measure,
    double threshold,
    insert_iterator_type ins
    )
{
    switch (measure) {
    case exact:
        dbr.retrieve<simstring::measure::exact>(query, threshold, ins);
        break;
    case dice:
        dbr.retrieve<simstring::measure::dice>(query, threshold, ins);
        break;
    case cosine:
        dbr.retrieve<simstring::measure::cosine>(query, threshold, ins);
        break;
    case jaccard:
        dbr.retrieve<simstring::measure::jaccard>(query, threshold, ins);
        break;
    case overlap:
        dbr.retrieve<simstring::measure::overlap>(query, threshold, ins);
        break;
    }
}

template <class char_type, class insert_iterator_type>
void retrieve_iconv(
    reader_type& dbr,
    const std::string& query,
    const char *encoding,
    int measure,
    double threshold,
    insert_iterator_type ins
    )
{
    typedef std::basic_string<char_type> string_type;
    typedef std::vector<string_type> strings_type;

    // Translate the character encoding of the query string from UTF-8 to the target encoding.
    string_type qstr;
    iconv_t fwd = iconv_open(encoding, "UTF-8");
    iconv_convert(fwd, query, qstr);
    iconv_close(fwd);
    
    strings_type xstrs;
    switch (measure) {
    case exact:
        dbr.retrieve<simstring::measure::exact>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case dice:
        dbr.retrieve<simstring::measure::dice>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case cosine:
        dbr.retrieve<simstring::measure::cosine>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case jaccard:
        dbr.retrieve<simstring::measure::jaccard>(qstr, threshold, std::back_inserter(xstrs));
        break;
    case overlap:
        dbr.retrieve<simstring::measure::overlap>(qstr, threshold, std::back_inserter(xstrs));
        break;
    }

    // Translate back the character encoding of retrieved strings into UTF-8.
    iconv_t bwd = iconv_open("UTF-8", encoding);
    for (typename strings_type::const_iterator it = xstrs.begin();it != xstrs.end();++it) {
        std::string dst;
    iconv_convert(bwd, *it, dst);
    *ins = dst;
    }
    iconv_close(bwd);
}

std::vector<std::string> reader::retrieve(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    std::vector<std::string> ret;

    switch (dbr.char_size()) {
    case 1:
        retrieve_thru(dbr, query, this->measure, this->threshold, std::back_inserter(ret));
        break;
    case 2:
        retrieve_iconv<uint16_t>(dbr, query, UTF16, this->measure, this->threshold, std::back_inserter(ret));
        break;
    case 4:
        retrieve_iconv<uint32_t>(dbr, query, UTF32, this->measure, this->threshold, std::back_inserter(ret));
        break;
    }

    return ret;
}

bool reader::check(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    
    if (dbr.char_size() == 1) {
        std::string qstr = query;
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    } else if (dbr.char_size() == 2) {
        std::basic_string<uint16_t> qstr;
        iconv_t fwd = iconv_open(UTF16, "UTF-8");
        iconv_convert(fwd, std::string(query), qstr);
        iconv_close(fwd);
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    } else if (dbr.char_size() == 4) {
        std::basic_string<uint32_t> qstr;
        iconv_t fwd = iconv_open(UTF32, "UTF-8");
        iconv_convert(fwd, std::string(query), qstr);
        iconv_close(fwd);
        return dbr.check(qstr, translate_measure(this->measure), this->threshold);
    }
    
    return false;
}

void reader::close()
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    dbr.close();
}
