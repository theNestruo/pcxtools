[![C/C++ CI](https://github.com/theNestruo/pcxtools/workflows/C/C++%20CI/badge.svg)](https://github.com/theNestruo/pcxtools/actions)
[![CodeFactor](https://www.codefactor.io/repository/github/thenestruo/pcxtools/badge)](https://www.codefactor.io/repository/github/thenestruo/pcxtools)

# PCXTOOLS: PNG2MSX, PNGSPR\[+\] and TMX2BIN

`PNG2MSX` and `PNG2SPR[+]` are free command line tools to convert PNG images to TMS9918 (MSX-1 VDP) format (i.e. CHRTBL/CLRTBL/NAMTBL-ready values, and SPRTBL/SPATBL-ready values).

`TMX2BIN` is a free command line tool to convert Tiled maps to binary.

## Index

- [Deprecation notice](#deprecation-notice)
- [Usage](#usage)
- [Options](#options)
- [Palettes](#palettes)
- [Version history](#version-history)
- [Future plans](#future-plans)
- [Author and last words](#author-and-last-words)

## Deprecation notice

~~`PCX2MSX[+]`~~ and ~~`PCX2SPR[+]`~~ have been replaced by `PNG2MSX` and `PNG2SPR[+]`, will not be further maintained, and may be removed in future versions.

## Usage

Exit code will be zero if ok and non-zero if there was an error. This allows using this programs in makefiles, command chains, etc.

### PNG2MSX, PNG2SPR, and PNG2SPR+

From command line, type:
* `PNG2MSX [options] charset.png`
* `PNG2MSX [options] charset.png [extra.png ...]`
* `PNG2MSX [options] charset.png -n [screen.png ...]`
* `PNG2SPR [options] sprites.png`
* `PNG2SPR+ [options] sprites.png`

`charset.png`, `extra.png`, `screen.png`, and `sprites.png` are the name of the input PNG file(s).

Please note that:
* Extra pixels (width or height not multiple of 8 or 16) will be ignored (this behaviour differs to the original PCX2MSX)

The output files will have the same name as the input file plus an additional extension (.chr, .clr, .nam, .spr\[.asm\] or .spat\[.asm\]). If files exist with the same name, they will be overwritten.

### TMX2BIN

From command line, type:
* `TMX2BIN [options] tmxFilename`

`tmxFilename` is the name of the input TMX file. The next restrictions apply to the input file:
+ Must be well formed (XML identifier followed by `<map>` tag).
+ All the XML file lines must be shorter than 1024b.
+ It should have at least one layer. Extra layers, if present, will be ignored.
+ The layer must have its data encoded as "csv". Other Tiled encoding methods, such as XML or Base64, are not supported.
+ Tilesets with more than 256 tiles are not supported. Values greater than 255 will cause a warning, as each value is meant to be stored in one byte.

## Options

Order of options is non important. Unknown options will be silently ignored. If there are mutually exclusive options, is undefined which one will be taked into account.

### Common options

* `-v` verbose execution
    By default, only exceptional output (warnings and error) is written. Use `-v` to see the differents parts of the process
* `-d` dry run
    Doesn't write output files. Useful to just validate input PNG files
* `-e` index color by euclidean distance (this is the default)
* `-g` index color by weighted distance

### PNG2MSX, and PNG2SPR\[+\]

* `-i` inverted
    Flips bitmap vertically.
* `-m` mirrored
    Flips bitmap horizontally.

### PNG2MSX

* `-il` ignore line on color collision
    Continue processing the file even if collisions are found. Offending lines will be have 0x00 both as pattern and color.
    Can be useful in combination with `-d` to check all the collisions at once.
* `-hl` force higher color to be foreground
* `-lh` force lower color to be foreground
* `-f<0..7>` force bit <n> to be foreground (set) on patterns
* `-b<0..7>` force bit <n> to be background (reset) on patterns
    These four options allow some control on how patterns are created, and which color is foreground and which one is background.
    Can be useful if colors are going to be set programatically (e.g.: fonts colored with FILVRM) or to improve compression results.
* `-n<00..ff>` generate NAMTBL \[starting at value <n>\]
    Generates NAMTBL.
    If various PNG files are provided, NAMTBL is generated from the additional files, mapped against blocks of the first PNG file.
    If optional index value is specified, name table will be generated to load the character definition after that index (e.g.: tileset loaded without overwriting ASCII font).
    Usually to be used in combination with `-rm` and/or `-rr`.
* `-bb<00..ff>` blank block at position <nn>
    A blank block (pattern = 0x00, color = 0x00) will be generated at specified position. Removed blocks will have this index in the name table.
    Usually to be used in combination with -n and/or rm (e.g.: to keep ASCII 0x20 " " as the blank block)
* ` -rr` remove repeated tiles
    Only the first copy of identical tiles will be kept.
    Usally to be used in combination with `-n`.
    If various PNG files are provided, blocks removed from the first file will be also removed from the additional files (e.g.: related tilesets: on/off, day/night...)
* `-rm<0..f>` remove solid tiles of <n> color
    Removes all the solid tiles composed entirely of pixels of the specified color (hexadecimal).
    If various PNG files are provided, blocks removed from the first file will be also removed from the additional files (e.g.: related tilesets: on/off, day/night...)

### PNG2SPR only

* `-8` generate 8x8px sprites
    Output is adjusted so it can be used in 8x8px sprites modes
* ` -h` generate half sprites (8x16px, 16b per sprite)
    Processing order is adjusted so multicolored sprites are grouped by half sprites (8px width, 16px height)
* `-hl` lower colors will have higher priority planes (default)
* `-lh` higher colors will have higher priority planes
    These two options allow some control on how the colors get ordered (namely, to avoid the flickering routines make flicker the brigther colors)

### PNG2SPR+ only

* `-w<16..255>` sprite width (default: 16px)
    Sprite width inside the spritesheet.
* `-h<16..255>` sprite height (default: 16px)
    Sprite height inside the spritesheet.
* `-x<0..255>` X offset (default: center)
    X offset for horizontal coordinates for SPATBL.
    Default is center of the sprite (i.e.: half width).
    Negative coordinates may appear if non-zero X offset is used.
* `-y<0..255>` Y offset (default: middle)
    Y offset for vertical coordinates for SPATBL.
    Default is center of the sprite (i.e.: half width).
    Negative coordinates may appear anytime.
* `-p<0..4> attribute padding size (default: 1b)`
    Padding size, in bytes, to be append after each sprite group.
    Default is 1 byte (enough for a marker value byte).
* `-t<00..ff>` terminator byte (default: 0xD0 (SPAT_END))
    First padding byte value.
    If default value 0xD0 (SPAT_END) is used, pattern number will be reset after each sprite group; this is recommended for large spritesheets.
* `-b` binary spat output (default: asm)
    Save SPATBL as binary file.
    Default is the more versatile ASCII assembly code file.

### TMX2BIN only

* `-t<0..255>` reorganize data as metatiles of <0..255>x<0..255> bytes
    If not provided, the output data will be the same as it is in the TMX file.
    If a metatile size is provided, the data will be scanned from left to right and from top to bottom in blocks of the specified size.
    E.g.: Given a TMX with:
    ```
    123456
    7890ab
    cdefgh
    ijklmn
    ```
    The default output will be ` 123456 7890ab cdefgh ijklmn` (spaces added for clarity).
    A metatile size of 2 will output `1278 3490 56ab cdij efkl ghmn` (6 metatiles of 2x2 bytes).
    If the width and/or height of the TMX data is not multiple of the specified size, extra data will be silently ignored.
    Thus, a metatile size of 3 will output: `123789cde 4560abfgh` (2 metatiles of 3x3 bytes, rest of data ignored).

## Palettes
For each pixel, the color index is computed by looking for the lowest either euclidean or weigthed color distance, depending of the options.

As the MSX palette is not well defined, with several emulators using slightly different palettes, and the MSX2 palette does not exactly match the MSX palette, Four different palettes are used in a best effort to cover all the use cases and get the right color for every developer:

<table>
<tr>
    <th>Index</th>
    <th>TMS9918 palette<sup>1</sup></th>
    <th>TMS9219 palette<sup>2</sup></th>
    <th>Yazioh palette<sup>3</sup></th>
    <th>TOSHIBA palette<sup>4</sup></th>
    <th>V9938 palette<sup>2</sup></th>
    <th>Name</th>
</tr><tr>
    <td>1</td>
    <td style="background:#000000;color:#fff"><samp>000000</samp></td><!-- TMS9918 -->
    <td style="background:#000000;color:#fff"><samp>000000</samp></td><!-- TMS9219 -->
    <td style="background:#000000;color:#fff"><samp>000000</samp></td><!-- Yazioh -->
    <td style="background:#000000;color:#fff"><samp>000000</samp></td><!-- TOSHIBA -->
    <td style="background:#000000;color:#fff"><samp>000000</samp></td><!-- V9938 -->
    <td>Black</td>
</tr><tr>
    <td>2</td>
    <td style="background:#0AAD1E;color:#000"><samp>0AAD1E</samp></td><!-- TMS9918 -->
    <td style="background:#23CB32;color:#000"><samp>23CB32</samp></td><!-- TMS9219 -->
    <td style="background:#3EB849;color:#000"><samp>3EB849</samp></td><!-- Yazioh -->
    <td style="background:#66CC66;color:#000"><samp>66CC66</samp></td><!-- TOSHIBA -->
    <td style="background:#24DA24;color:#000"><samp>24DA24</samp></td><!-- V9938 -->
    <td>Medium green</td>
</tr><tr>
    <td>3</td>
    <td style="background:#34C84C;color:#000"><samp>34C84C</samp></td><!-- TMS9918 -->
    <td style="background:#60DD6C;color:#000"><samp>60DD6C</samp></td><!-- TMS9219 -->
    <td style="background:#74D07D;color:#000"><samp>74D07D</samp></td><!-- Yazioh -->
    <td style="background:#88EE88;color:#000"><samp>88EE88</samp></td><!-- TOSHIBA -->
    <td style="background:#6DFF6D;color:#000"><samp>6DFF6D</samp></td><!-- V9938 -->
    <td>Light green</td>
</tr><tr>
    <td>4</td>
    <td style="background:#2B2DE3;color:#fff"><samp>2B2DE3</samp></td><!-- TMS9918 -->
    <td style="background:#544EFF;color:#fff"><samp>544EFF</samp></td><!-- TMS9219 -->
	<td style="background:#5955E0;color:#fff"><samp>5955E0</samp></td><!-- Yazioh -->
    <td style="background:#4444DD;color:#fff"><samp>4444DD</samp></td><!-- TOSHIBA -->
    <td style="background:#2424FF;color:#fff"><samp>2424FF</samp></td><!-- V9938 -->
    <td>Dark blue</td>
</tr><tr>
    <td>5</td>
    <td style="background:#514BFB;color:#fff"><samp>514BFB</samp></td><!-- TMS9918 -->
    <td style="background:#7D70FF;color:#fff"><samp>7D70FF</samp></td><!-- TMS9219 -->
	<td style="background:#8076F1;color:#fff"><samp>8076F1</samp></td><!-- Yazioh -->
    <td style="background:#7777FF;color:#fff"><samp>7777FF</samp></td><!-- TOSHIBA -->
    <td style="background:#486DFF;color:#fff"><samp>486DFF</samp></td><!-- V9938 -->
    <td>Medium blue</td>
</tr><tr>
    <td>6</td>
    <td style="background:#BD2925;color:#fff"><samp>BD2925</samp></td><!-- TMS9918 -->
    <td style="background:#D25442;color:#fff"><samp>D25442</samp></td><!-- TMS9219 -->
	<td style="background:#B95E51;color:#fff"><samp>B95E51</samp></td><!-- Yazioh -->
    <td style="background:#BB5555;color:#fff"><samp>BB5555</samp></td><!-- TOSHIBA -->
    <td style="background:#B62424;color:#fff"><samp>B62424</samp></td><!-- V9938 -->
    <td>Dark red</td>
</tr><tr>
    <td>7</td>
    <td style="background:#1EE2EF;color:#000"><samp>1EE2EF</samp></td><!-- TMS9918 -->
    <td style="background:#45E8FF;color:#000"><samp>45E8FF</samp></td><!-- TMS9219 -->
	<td style="background:#65DBEF;color:#000"><samp>65DBEF</samp></td><!-- Yazioh -->
    <td style="background:#77DDDD;color:#000"><samp>77DDDD</samp></td><!-- TOSHIBA -->
    <td style="background:#48DAFF;color:#000"><samp>48DAFF</samp></td><!-- V9938 -->
    <td>Cyan</td>
</tr><tr>
    <td>8</td>
    <td style="background:#FB2C2B;color:#fff"><samp>FB2C2B</samp></td><!-- TMS9918 -->
    <td style="background:#FA5948;color:#fff"><samp>FA5948</samp></td><!-- TMS9219 -->
	<td style="background:#DB6559;color:#fff"><samp>DB6559</samp></td><!-- Yazioh -->
    <td style="background:#DD6666;color:#fff"><samp>DD6666</samp></td><!-- TOSHIBA -->
    <td style="background:#FF2424;color:#fff"><samp>FF2424</samp></td><!-- V9938 -->
    <td>Medium red</td>
</tr><tr>
    <td>9</td>
    <td style="background:#FF5F4C;color:#fff"><samp>FF5F4C</samp></td><!-- TMS9918 -->
    <td style="background:#FF7C6C;color:#fff"><samp>FF7C6C</samp></td><!-- TMS9219 -->
	<td style="background:#FF897D;color:#fff"><samp>FF897D</samp></td><!-- Yazioh -->
    <td style="background:#FF7777;color:#fff"><samp>FF7777</samp></td><!-- TOSHIBA -->
    <td style="background:#FF6D6D;color:#fff"><samp>FF6D6D</samp></td><!-- V9938 -->
    <td>Light red</td>
</tr><tr>
    <td>10</td>
    <td style="background:#BDA22B;color:#000"><samp>BDA22B</samp></td><!-- TMS9918 -->
    <td style="background:#D3C63C;color:#000"><samp>D3C63C</samp></td><!-- TMS9219 -->
	<td style="background:#CCC35E;color:#000"><samp>CCC35E</samp></td><!-- Yazioh -->
    <td style="background:#CCCC55;color:#000"><samp>CCCC55</samp></td><!-- TOSHIBA -->
    <td style="background:#DADA24;color:#000"><samp>DADA24</samp></td><!-- V9938 -->
    <td>Dark yellow</td>
</tr><tr>
    <td>11</td>
    <td style="background:#D7B454;color:#000"><samp>D7B454</samp></td><!-- TMS9918 -->
    <td style="background:#E5D26D;color:#000"><samp>E5D26D</samp></td><!-- TMS9219 -->
	<td style="background:#DED087;color:#000"><samp>DED087</samp></td><!-- Yazioh -->
    <td style="background:#CCCC88;color:#000"><samp>CCCC88</samp></td><!-- TOSHIBA -->
    <td style="background:#DADA91;color:#000"><samp>DADA91</samp></td><!-- V9938 -->
    <td>Light yellow</td>
</tr><tr>
    <td>12</td>
    <td style="background:#0A8C18;color:#fff"><samp>0A8C18</samp></td><!-- TMS9918 -->
    <td style="background:#23B22C;color:#fff"><samp>23B22C</samp></td><!-- TMS9219 -->
	<td style="background:#3AA241;color:#fff"><samp>3AA241</samp></td><!-- Yazioh -->
    <td style="background:#55AA55;color:#fff"><samp>55AA55</samp></td><!-- TOSHIBA -->
    <td style="background:#249124;color:#fff"><samp>249124</samp></td><!-- V9938 -->
    <td>Dark green</td>
</tr><tr>
    <td>13</td>
    <td style="background:#AF329A;color:#fff"><samp>AF329A</samp></td><!-- TMS9918 -->
    <td style="background:#C85AC6;color:#fff"><samp>C85AC6</samp></td><!-- TMS9219 -->
	<td style="background:#B766B5;color:#fff"><samp>B766B5</samp></td><!-- Yazioh -->
    <td style="background:#BB55BB;color:#fff"><samp>BB55BB</samp></td><!-- TOSHIBA -->
    <td style="background:#DA48B6;color:#fff"><samp>DA48B6</samp></td><!-- V9938 -->
    <td>Magenta</td>
</tr><tr>
    <td>14</td>
    <td style="background:#B2B2B2;color:#000"><samp>B2B2B2</samp></td><!-- TMS9918 -->
    <td style="background:#CCCCCC;color:#000"><samp>CCCCCC</samp></td><!-- TMS9219 -->
	<td style="background:#CCCCCC;color:#000"><samp>CCCCCC</samp></td><!-- Yazioh -->
    <td style="background:#CCCCCC;color:#000"><samp>CCCCCC</samp></td><!-- TOSHIBA -->
    <td style="background:#B6B6B6;color:#000"><samp>B6B6B6</samp></td><!-- V9938 -->
    <td>Gray</td>
</tr><tr>
    <td>15</td>
    <td style="background:#FFFFFF;color:#000"><samp>FFFFFF</samp></td><!-- TMS9918 -->
    <td style="background:#FFFFFF;color:#000"><samp>FFFFFF</samp></td><!-- TMS9219 -->
	<td style="background:#FFFFFF;color:#000"><samp>FFFFFF</samp></td><!-- Yazioh -->
    <td style="background:#EEEEEE;color:#000"><samp>EEEEEE</samp></td><!-- TOSHIBA -->
    <td style="background:#FFFFFF;color:#000"><samp>FFFFFF</samp></td><!-- V9938 -->
    <td>White</td>
</tr>
</table>

<sup>1</sup> TI TMS9918 palette, according Wikipedia (https://en.wikipedia.org/wiki/Texas_Instruments_TMS9918#Colors)

<sup>2</sup> TI TMS9219 and V9938 palettes from hap's meisei emulator

<sup>3</sup> Palette used by pixel artist <a href="https://twitter.com/Yazioh">@Yazioh</a>, because of reasons

<sup>4</sup> TOSHIBA palette from reidrac's MSX Pixel Tools (https://github.com/reidrac/msx-pixel-tools)

Transparent pixels are indexed from the following values:

<table>
<tr>
    <th>Index</th>
    <th>RGBA value</th>
    <th>Description</th>
    <th>Name</th>
</tr><tr>
    <td>0</td>
    <td><samp>??????00</samp></td>
    <td>Actually transparent (any color with alpha channel = 0)</td>
    <td>Transparent</td>
</tr><tr>
    <td>0</td>
    <td style="background:#FF00FF;color:#fff"><samp>FF00FF</samp></td>
    <td>Fuchsia (traditional color used to denote transparency)</td>
    <td>Transparent</td>
</tr><tr>
    <td>0</td>
    <td style="background:#404040;color:#fff"><samp>404040</samp></td>
    <td>Dark gray (was used in the old PCS2MSX reference files)</td>
    <td>Transparent</td>
</tr>
</table>

Since version 3.1 `PNG2MSX` and `PNG2SPR[+]` try to autodetect the closest palette, then use the autodetected palette to match the colors, ignoring the other palettes. This behaviour attempts to minimize color clashes between different palettes (particularly _medium blue_ and _dark blue_), but can lead to unexpected results if colors from different palettes are mixed in one image.

## Version history

See [version history](changelog.MD)

## Future plans

* \[*\] Improve source code.
* `PCX2MSX[+]` Improve NAMTBL options (banks, etc.).
* `PCX2MSX[+]` Output NAMTBL as assembly.
* `TMX2BIN` metatiles with different width and height.
* `TMX2BIN` Multiple layers.

## Author and last words

Coded by [theNestruo](mailto:theNestruo@gmail.com)

* Original `PCX2MSX` was coded by Edward A. Robsy Petrus \[25/12/2004\]. `PCX2MSX` is inspired by that original version, but is not related with it in any other way.
* [Tiled](http://www.mapeditor.org/) (c) 2008-2020 Thorbjørn Lindeijer.
* [LodePNG](https://lodev.org/lodepng/) (c) 2005-2023 by Lode Vandevenne.
* Test graphics extracted from the original freeware version of [Ninja Senki](http://ninjasenki.com/) (c) 2010 Jonathan Lavigne.
* `PNG2SPR+` demo sprites extracted from [Sydney Hunter and the Caverns of Death](http://www.studiopinagames.com/sydney.html) (c) 2011 Keith Erickson / Studio Pi&ntilde;a

Greetings to: Robsy, Iban Nieto, Jon Cortázar
