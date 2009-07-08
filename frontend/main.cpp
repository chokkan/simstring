#include <iostream>
#include <string>
#include <vector>
#include <dastring/dastring.h>

#include "optparse.h"

typedef std::string string_type;
typedef dastring::ngram_generator ngram_generator_type;
typedef dastring::writer_base<string_type, ngram_generator_type> writer_type;
typedef dastring::reader_base<string_type> reader_type;
typedef dastring::query::exact<string_type, ngram_generator_type> query_exact_type;
typedef dastring::query::cosine<string_type, ngram_generator_type> query_cosine_type;
typedef dastring::query::dice<string_type, ngram_generator_type> query_dice_type;
typedef dastring::query::jaccard<string_type, ngram_generator_type> query_jaccard_type;

class option : public optparse
{
public:
    enum {
        MODE_INTERACTIVE,
        MODE_BUILD,
        MODE_HELP,
    };

    enum {
        QT_EXACT,
        QT_DICE,
        QT_COSINE,
        QT_JACCARD,
    };

    int mode;
    std::string name;

    int ngram_size;
    int query_type;
    double threshold;
    
public:
    option() :
        mode(MODE_INTERACTIVE),
        name(""),
        ngram_size(3),
        query_type(QT_EXACT),
        threshold(1.)
    {
    }

    BEGIN_OPTION_MAP_INLINE()
        ON_OPTION(SHORTOPT('b') || LONGOPT("build"))
            mode = MODE_BUILD;

        ON_OPTION_WITH_ARG(SHORTOPT('d') || LONGOPT("database"))
            name = arg;

        ON_OPTION(SHORTOPT('h') || LONGOPT("help"))
            mode = MODE_HELP;

    END_OPTION_MAP()
};

static void usage(std::ostream& os, const char *argv0)
{
    os << "USAGE: " << argv0 << " [OPTIONS]" << std::endl;
    os << std::endl;
}

int build(option& opt)
{
    ngram_generator_type gen(opt.ngram_size);

    writer_type db(gen);
    std::istream& is = std::cin;
    std::ostream& os = std::cout;

    db.open(opt.name);

    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }
        db.insert(line);
    }

    db.close();    
    return 0;
}

static void retrieve_callback(const string_type& str)
{
    std::cout << '\t' << str << std::endl;
}

int interactive(option& opt)
{
    reader_type db;
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    db.open(opt.name);

    ngram_generator_type gen(opt.ngram_size);

    for (;;) {
        std::string line;
        std::getline(is, line);
        if (is.eof()) {
            break;
        }

        if (line.compare(0, 7, "q exact") == 0) {
            opt.query_type = option::QT_EXACT;

        } else if (line.compare(0, 7, "q dice ") == 0) {
            opt.query_type = option::QT_DICE;
            opt.threshold = std::atof(line.c_str() + 7);

        } else if (line.compare(0, 9, "q cosine ") == 0) {
            opt.query_type = option::QT_COSINE;
            opt.threshold = std::atof(line.c_str() + 9);

        } else if (line.compare(0, 10, "q jaccard ") == 0) {
            opt.query_type = option::QT_JACCARD;
            opt.threshold = std::atof(line.c_str() + 10);

        } else if (line.compare(0, 1, "h") == 0) {
            //usage_interactive(os);

        } else if (line.compare(0, 2, "s ") == 0) {
            std::string qstr = line.substr(2);

            switch (opt.query_type) {
            case option::QT_EXACT:
                db.retrieve(query_exact_type(gen, qstr), retrieve_callback);
                break;
            case option::QT_DICE:
                db.retrieve(query_dice_type(gen, qstr, opt.threshold), retrieve_callback);
                break;
            case option::QT_COSINE:
                db.retrieve(query_cosine_type(gen, qstr, opt.threshold), retrieve_callback);
                break;
            case option::QT_JACCARD:
                db.retrieve(query_jaccard_type(gen, qstr, opt.threshold), retrieve_callback);
                break;
            }

        } else {

        }

    }

    return 0;
}


int main(int argc, char *argv[])
{
    option opt;
    std::istream& is = std::cin;
    std::ostream& os = std::cout;
    std::ostream& es = std::cerr;

    // Parse the command-line options.
    try { 
        int arg_used = opt.parse(argv, argc);
    } catch (const optparse::unrecognized_option& e) {
        es << "ERROR: unrecognized option: " << e.what() << std::endl;
        return 1;
    } catch (const optparse::invalid_value& e) {
        es << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    // Branches for the processing mode.
    switch (opt.mode) {
    case option::MODE_HELP:
        return usage(os, argv[0]);
    case option::MODE_BUILD:
        return build(opt);
    case option::MODE_INTERACTIVE:
        return interactive(opt);
    default:
        return 1;
    }
}
