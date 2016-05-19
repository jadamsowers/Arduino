#/usr/local/bin/python
# coding: utf-8


notes = [130.81, 146.83, 164.81, 174.61, 196.00, 220.00, 246.94, 261.63]
#          C3      D3      E3      F3      G3      A4      B4      C4

# 1000000 µs in a second * 50% duty cycle
lengths = [ int(1000000.0 / 2 / (notes[x])) for x in range (0, len(notes)) ]
print "const long v[] = {", str(lengths)[1:-1], "};"
