#!/usr/bin/env perl

use warnings;
use strict;
use simstring;

# Create a SimString database with two person names.
my $db = simstring::writer->new('sample.db');
$db->insert('Barack Hussein Obama II');
$db->insert('James Gordon Brown');
$db->close;

# Open the database for reading.
$db = simstring::reader->new('sample.db');

# Use cosine similarity and threshold 0.6.
$db->swig_measure_set($simstring::cosine);
$db->swig_threshold_set(0.6);
print @{ $db->retrieve('Barack Obama') }, "\n";      # OK.
print @{ $db->retrieve('Gordon Brown') }, "\n";      # OK.
print @{ $db->retrieve('Obama') }, "\n";             # Too dissimilar!

# Use overlap coefficient and threshold 1.0.
$db->swig_measure_set($simstring::overlap);
$db->swig_threshold_set(1);
print @{ $db->retrieve('Obama') }, "\n";             # OK.

__END__
