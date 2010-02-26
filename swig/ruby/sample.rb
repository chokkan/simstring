#!/usr/bin/env ruby

require 'simstring'

# Create a SimString database with two person names.
db = Simstring::Writer.new('sample.db')
db.insert('Barack Hussein Obama II')
db.insert('James Gordon Brown')
db.close()

# Open the database for reading.
db = Simstring::Reader.new('sample.db')

# Use cosine similarity and threshold 0.6.
db.measure = Simstring::Cosine
db.threshold = 0.6
p(db.retrieve('Barack Obama'))      # OK.
p(db.retrieve('Gordon Brown'))      # OK.
p(db.retrieve('Obama'))             # Too dissimilar!

# Use overlap coefficient and threshold 1.0.
db.measure = Simstring::Overlap
db.threshold = 1
p(db.retrieve('Obama'))             # OK.

