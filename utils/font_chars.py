#!/usr/bin/env python

import sys

if len(sys.argv) != 2:
    print "usage: %s <possible text>" % (sys.argv[0],)
    print ""
    print ("Builds a regular expression matching all characters from"
           " <possible text>.")
    sys.exit(1)

chars = set(sys.argv[1])
prefix = ''
suffix = ''

if ']' in chars:
    chars.remove(']')
    prefix = ']'

if '-' in chars:
    chars.remove('-')
    suffix = '-'

if '^' in chars:
    chars.remove('^')
    if suffix and not (chars or prefix):
        suffix += '^'
    else:
        suffix = '^' + suffix

print '[%s%s%s]' % (prefix, ''.join(sorted(chars)), suffix)
