use strict;
use warnings;
use Test::More tests => 5;
use simstring;
use utf8;

# *insert = *simstringc::writer_insert;
# *close = *simstringc::writer_close;

# *retrieve = *simstringc::reader_retrieve;
# *close = *simstringc::reader_close;
# *swig_measure_get = *simstringc::reader_measure_get;
# *swig_measure_set = *simstringc::reader_measure_set;
# *swig_threshold_get = *simstringc::reader_threshold_get;
# *swig_threshold_set = *simstringc::reader_threshold_set;

# *exact = *simstringc::exact;     0
# *dice = *simstringc::dice;       1
# *cosine = *simstringc::cosine;   2
# *jaccard = *simstringc::jaccard; 3
# *overlap = *simstringc::overlap; 4

my $db = simstring::writer->new('sample_unicode.db', 3, undef, 1);
ok( $db, 'writer->new' );
$db->insert('スパゲティ');
$db->close;

$db = simstring::reader->new('sample_unicode.db');
ok( $db, 'reader->new' );
$db->swig_measure_set($simstring::cosine);
ok( $db->swig_measure_get == 2, 'measure_get: cosine' );
$db->swig_threshold_set(0.6);
ok( $db->swig_threshold_get == 0.6, 'threshold_get: 0.6' );

ok( scalar @{ $db->retrieve('スパゲティー') } == 1, 'retrieve (cosine 0.6)' );

