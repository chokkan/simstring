use strict;
use warnings;
use Test::More tests => 8;
use simstring;

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

my $db = simstring::writer->new('sample.db');
ok( $db, 'writer->new' );
$db->insert('Barack Hussein Obama II');
$db->insert('James Gordon Brown');
$db->close;

$db = simstring::reader->new('sample.db');
ok( $db, 'reader->new' );
$db->swig_measure_set($simstring::cosine);
ok( $db->swig_measure_get == 2, 'measure_get: cosine' );
$db->swig_threshold_set(0.6);
ok( $db->swig_threshold_get == 0.6, 'threshold_get: 0.6' );

ok( ( scalar @{ $db->retrieve('Barack Obama') } == 1
      && scalar @{ $db->retrieve('Gordon Brown') } == 1
      && scalar @{ $db->retrieve('Obama') } == 0), 'retrieve (cosine 0.6)' );

$db->swig_measure_set($simstring::overlap);
ok( $db->swig_measure_get == 4, 'measure_get: overlap' );
$db->swig_threshold_set(1);
ok( $db->swig_threshold_get == 1, 'threshold_get: 1' );

ok( scalar @{ $db->retrieve('Obama') } == 1, 'retrieve (overlap 1)' );
