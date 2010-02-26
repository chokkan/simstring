#!/usr/bin/env python
# -*- coding:utf-8 -*-

"""
In order to use similarity 
represent character n-grams in Unicode as representation, we need to:
1) set the unicode flag to True
2) use 8-bit strings encoded in UTF-8

When a unicode flag is set to True, the 'simstring' module assumes

     
Because this source code is written in UTF-8 (see the encoding
declaration), we can use 8-bit strings (that are also encoded in
UTF-8) as queries.

"""

import simstring

# Open a SimString database for writing (with unicode mode enabled).
db = simstring.writer('sample_unicode.db', 3, True, True)

# Write a string, and close the database.
db.insert('スパゲティ')
db.close()


# Open the SimString database for reading.
db = simstring.reader('sample_unicode.db')

# Set a similarity measure and threshold.
db.measure = simstring.cosine
db.threshold = 0.6

# Use an 8-bit string in UTF-8 encoding.
print ' '.join(db.retrieve('スパゲッティ'))

# Convert a Unicode object into an 8-bit UTF-8 string.
print ' '.join(db.retrieve(u'スパゲティー'.encode('utf-8')))

