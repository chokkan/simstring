#!/usr/bin/env python

import simstring

# Create a SimString database with two person names.
db = simstring.writer('sample.db')
db.insert('Barack Hussein Obama II')
db.insert('James Gordon Brown')
db.close()


# Open the database for reading.
db = simstring.reader('sample.db')

# Use cosine similarity and threshold 0.6.
db.measure = simstring.cosine
db.threshold = 0.6
print(db.retrieve('Barack Obama'))      # OK.
print(db.retrieve('Gordon Brown'))      # OK.
print(db.retrieve('Obama'))             # Too dissimilar!

# Use overlap coefficient and threshold 1.0.
db.measure = simstring.overlap
db.threshold = 1.
print(db.retrieve('Obama'))             # OK.
