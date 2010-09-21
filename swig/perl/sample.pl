#!/usr/bin/env perl

use warnings;
use strict;
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
$db->insert('Barack Hussein Obama II');
$db->insert('James Gordon Brown');
$db->close;

$db = simstring::reader->new('sample.db');
$db->swig_measure_set($simstring::cosine);
$db->swig_threshold_set(0.6);

print @{ $db->retrieve('Barack Obama') }, "\n";      # OK.
print @{ $db->retrieve('Gordon Brown') }, "\n";      # OK.
print @{ $db->retrieve('Obama') }, "\n";             # Too dissimilar!

$db->swig_measure_set($simstring::overlap);
$db->swig_threshold_set(1);

print @{ $db->retrieve('Obama') }, "\n";             # OK.

__END__
