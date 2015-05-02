    PCX2MSX
    PCX2MSX+
    PCX2SPR
    PCX2SPR+
---------------
PCX2MSX[+] and PCX2SPR[+] are free command line tools
to convert PCX images to TMS9918 (MSX-1 VDP) format
(i.e. CHRTBL/CLRTBL/NAMTBL and SPRTBL/SPATBL-ready values).

Original PCX2MSX was coded by Edward A. Robsy Petrus [25/12/2004].
(PCX2MSX is inspired by that original version,
but is not related with it in any other way)


    Usage
-------------
From command line, type:
    PCX2MSX [options] pcxFilename
or
    PCX2MSX+ [options] pcxFilename [pcxFilename...]
or
    PCX2SPR [options] pcxFilename
or
    PCX2SPR+ [options] pcxFilename

pcxFilename is the name of the input PCX file.
The next restrictions apply to the input file:
+ Must be well formed (0x0A signature, RLE encoded).
+ Must have 8bpp color depth.
Please note that:
+ Pixels with color index greater than 15 will be ignored
+ Extra pixels (width or height not multiple of 8 or 16) will be ignored
  (this behaviour differs to the original PCX2MSX)

The output files will have the same name as pcxFilename
plus an additional extension (.chr, .clr, .nam, .spr or .spat[.asm]).
If files exist with the same name, they will be overwritten.

Exit code will be zero if ok and non-zero if there was an error.
This allows using this programs in makefiles, command chains, etc.


    Options
---------------
Order of options is non important.
Unknown options will be silently ignored.
If there are mutually exclusive options,
is undefined which one will be taked into account.


These options are common to PCX2MSX, PCX2MSX+ and PCX2SPR:
-v  verbose execution
    By default, only exceptional output (warnings and error) is written.
    Use -v to see the differents parts of the process

-d  dry run. Doesn't write output files.
    Useful to just validate input PCX files

-i  inverted. Flips bitmap vertically.

-m  mirrored. Flips bitmap horizontally.


The next options are specific to PCX2MSX:
-il  ignore line on color collision
    Continue processing the file even if collisions are found.
    Offending lines will be have 0x00 both as pattern and color.
    Can be useful in combination with -d to check all the collisions at once.

-f<0..7> force bit <n> to be foreground (set) on patterns
-b<0..7> force bit <n> to be background (reset) on patterns
-hl force higher color to be foreground
-lh force lower color to be foreground
    This four options allow some control on how patterns are created,
    and which color is foreground and which one is background.
    Can be useful if colors are going to be set programatically
    (e.g.: fonts colored with FILVRM)

The next options are specific to PCX2MSX+:
-n<00..ff> generate NAMTBL [starting at value <n>]
    Generate NAMTBL.
    If various PCX files are provided, NAMTBL is generated
    from the additional files, mapped against blocks of the first PCX file.
    If optional index value is specified, name table will be generated
    to load the character definition after that index
    (e.g.: tileset loaded without overwriting ASCII font)
    Usually to be used in combination with -rm and/or -rr

-bb<00..ff> blank block at position <nn>
    A blank block (pattern = 0x00, color = 0x00) will be generated
    at specified position. Removed blocks will have this index in the name table.
    Usually to be used in combination with -n and/or rm
    (e.g.: to keep ASCII 0x20 " " as the blank block)

-rm<0..f> remove solid tiles of <n> color
    Removes all the solid tiles composed entirely
    of pixels of the specified color (hexadecimal).
    If various PCX files are provided, blocks removed from the first file
    will be also removed from the additional files
    (e.g.: related tilesets: on/off, day/night...)

-rr remove repeated tiles
    Only the first copy of identical tiles will be kept.
    Usally to be used in combination with -n.
    If various PCX files are provided, blocks removed from the first file
    will be also removed from the additional files
    (e.g.: related tilesets: on/off, day/night...)


The next options are specific to PCX2SPR:
-8  generate 8x8px sprites
    Output is adjusted so it can be used in 8x8px sprites modes

-h  generate half sprites (8x16px, 16b per sprite)
    Processing order is adjusted so multicolored sprites
    are grouped by half sprites (8px width, 16px height)


The next options are specific to PCX2SPR+:
-w<16..255> sprite width (default: 16px)
    Sprite width inside the spritesheet.
    
-h<16..255> sprite height (default: 16px)
    Sprite height inside the spritesheet.
    
-x<0..255> X offset (default: center)
    X offset for horizontal coordinates for SPATBL.
    Default is center of the sprite (i.e.: half width).
    Negative coordinates may appear if non-zero X offset is used.
    
-y<0..255> Y offset (default: middle)
    Y offset for vertical coordinates for SPATBL.
    Default is center of the sprite (i.e.: half width).
    Negative coordinates may appear anytime.

-p<0..4> attribute padding size (default: 1b)
    Padding size, in bytes, to be append after each sprite group.
    Default is 1 byte (enough for a marker value byte).
    
-t<00..ff> terminator byte (default: 0xD0 (SPAT_END))
    First padding byte value. If default value 0xD0 (SPAT_END) is used,
    pattern number will be reset after each sprite group;
    this is recommended for large spritesheets.
    
-b  binary spat output (default: asm)
    Save SPATBL as binary file.
    Default is the more versatile ASCII assembly code file.


    Version history
-----------------------
22/05/2014  v1.99c algorithm completely rewritten (PCX2SPR+)
13/04/2013  v1.99b suboptimal solutions fixed (PCX2SPR+)
28/03/2013  v1.99 first version with PCX2SPR+
21/12/2013  v1.0  PCX2MSX forked to PCX2MSX+, having
                  added offset to NAMTBL options
                  added blank block option
                  multiple PCX file management
09/10/2013  v0.99 removed -ps option (now, palette detection is automatic)
                  added NAMTBL options in PCX2MSX
15/06/2013  v0.9  first merged version of PCX2MSX and PCX2SPR


    Future plans
--------------------
Improve source code.
Improve NAMTBL options (banks, etc.).
Output NAMTBL as assembly.


    Author and last words
-----------------------------
Coded by theNestruo (theNestruo@gmail.com)

Greetings to: Robsy, Iban Nieto, Jon Cortázar
