#include <string>
#include <stdexcept>
#include <vector>
#include <simstring/simstring.h>

#include "export.h"

typedef std::string string_type;
typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<string_type, ngram_generator_type> writer_type;
typedef simstring::reader_base<string_type> reader_type;

writer::writer(const char *filename, int n)
    : m_dbw(NULL), m_gen(NULL)
{
    ngram_generator_type *gen = new ngram_generator_type(n);
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

writer::~writer()
{
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

void writer::insert(const char *string)
{
    writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    dbw->insert(string);
    if (dbw->fail()) {
        throw std::runtime_error(dbw->error());
    }
}

void writer::close()
{
    writer_type* dbw = reinterpret_cast<writer_type*>(m_dbw);
    dbw->close();
    if (dbw->fail()) {
        throw std::runtime_error(dbw->error());
    }
}

reader::reader(const char *filename, int n)
{
    ngram_generator_type *gen = new ngram_generator_type(n);
    reader_type *dbr = new reader_type;

    if (!dbr->open(filename)) {
        delete dbr;
        delete gen;
        throw std::invalid_argument("Failed to open the database");
    }

    m_dbr = dbr;
    m_gen = gen;
}

reader::~reader()
{
    this->close();
    delete reinterpret_cast<reader_type*>(m_dbr);
    delete reinterpret_cast<ngram_generator_type*>(m_gen);
}

std::vector<std::string> reader::retrieve(const char *query)
{
    typedef simstring::query::exact<string_type, ngram_generator_type> query_exact_type;
    typedef simstring::query::cosine<string_type, ngram_generator_type> query_cosine_type;
    typedef simstring::query::dice<string_type, ngram_generator_type> query_dice_type;
    typedef simstring::query::jaccard<string_type, ngram_generator_type> query_jaccard_type;
    typedef simstring::query::overlap<string_type, ngram_generator_type> query_overlap_type;

    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    ngram_generator_type& gen = *reinterpret_cast<ngram_generator_type*>(m_gen);

    std::vector<std::string> ret;
    switch (this->measure) {
    case exact:
        dbr.retrieve(
            query_exact_type(gen, query),
            std::back_inserter(ret)
            );
        break;
    case dice:
        dbr.retrieve(
            query_dice_type(gen, query, this->threshold),
            std::back_inserter(ret)
            );
        break;
    case cosine: 
        dbr.retrieve(
            query_cosine_type(gen, query, this->threshold),
            std::back_inserter(ret)
            );
        break;
    case jaccard:
        dbr.retrieve(
            query_jaccard_type(gen, query, this->threshold),
            std::back_inserter(ret)
            );
        break;
    case overlap:
        dbr.retrieve(
            query_overlap_type(gen, query, this->threshold),
            std::back_inserter(ret)
            );
        break;
    }

    return ret;
}

void reader::close()
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    dbr.close();
}

