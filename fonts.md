Quick font ideas:

- keep font under 4K
- 1 or 2 bpp
- 128-entry lookup table, 2B each (12b offset, 4b char width (plus constant?))
- column-first? 2B per column at height of 8px...
- this is already getting too complex.

SIMPLE FONT
===========

- 1 bpp
- lookup entries: 2B (12b offset, 4b char width (* 2))
- column first

| offset | name       | size | description |
|--------|------------|------|-------------|
| 0x0000 | Metrics    | 2    | The height of the font characters and the column length |
| 0x0002 | First char | 1    | The ASCII code of the first character in the font |
| 0x0003 | Char count | 1    | CC: The number characters in the font |
| 0x0004 | Lookup     | CC*2 | The offset and width of each character |
| CC*2+4 | Char data  | ...  | |


