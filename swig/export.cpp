#include <string>
#include <stdexcept>
#include <vector>
#include <simstring/simstring.h>

#include "export.h"

typedef std::string string_type;
typedef simstring::ngram_generator ngram_generator_type;
typedef simstring::writer_base<string_type, ngram_generator_type> writer_type;
typedef simstring::reader_base<string_type> reader_type;

writer::writer(const char *filename, int n, bool be)
    : m_dbw(NULL), m_gen(NULL)
{
    ngram_generator_type *gen = new ngram_generator_type(n, be);
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

std::vector<std::string> reader::retrieve(const char *query)
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);

    // Translate the similarity measure.
    int qt = -1;
    switch (this->measure) {
    case exact:
        qt = simstring::QT_EXACT;
        break;
    case dice:
        qt = simstring::QT_DICE;
        break;
    case cosine:
        qt = simstring::QT_COSINE;
        break;
    case jaccard:
        qt = simstring::QT_JACCARD;
        break;
    case overlap:
        qt = simstring::QT_OVERLAP;
        break;
    }

    std::vector<std::string> ret;
    dbr.retrieve(query, qt, this->threshold, std::back_inserter(ret));
    return ret;
}

void reader::close()
{
    reader_type& dbr = *reinterpret_cast<reader_type*>(m_dbr);
    dbr.close();
}

