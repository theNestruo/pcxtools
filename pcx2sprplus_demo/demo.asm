; -----------------------------------------------------------------------------
; PCX2SPR+ demo (by theNestruo)
; MSX cartridge / 8kB/16kB ROM / 8kB RAM
; Syntax: asMSX (v0.16 WIP, Eduardo A. Robsy Petrus)
; -----------------------------------------------------------------------------

	USE_CONTROL_SPRITES	equ 1
	
; BIOS entry points
	.bios

; ROM addresses (BIOS constants)
	VDP_DR	equ $0006 ; Base port address for VDP data read
	VDP_DW	equ $0007 ; Base port address for VDP data write
	MSXID1	equ $002b ; Frecuency, date format, charset
	MSXID3	equ $002d ; MSX version number

; RAM addresses (system vars)
	CLIKSW	equ $f3db ; Keyboard click sound
	RG1SAV	equ $f3e0 ; Content of VDP(1) register (R#1)
	FORCLR	equ $f3e9 ; Foreground colour
	BAKCLR	equ $f3ea ; Background colour
	BDRCLR	equ $f3eb ; Border colour
	HIMEM	equ $fc4a ; High free RAM address available (init stack with)

; VRAM addresses
	CHRTBL	equ $0000 ; Pattern table
	NAMTBL	equ $1800 ; Name table
	CLRTBL	equ $2000 ; Color table
	SPRATR	equ $1B00 ; Sprite attributes table
	SPRTBL	equ $3800 ; Sprite pattern table

; Special values
	SPAT_END	equ $d0 ; SPRATR end
	SPAT_OB		equ $d1 ; Sprite out of bounds

;
; =============================================================================
;	ROM, 8kB/16kB
; =============================================================================
;

	.page	1
	.rom
	.start	INIT
	
INIT:
; reset interrupt mode & stack pointer
	di
	im	1
	ld	sp, [HIMEM]
	ei
; color 15,13,13
	ld	a, 15
	ld	[FORCLR], a
	ld	a, 1
	ld	[BAKCLR], a
	ld	[BDRCLR], a
; screen 2
	call	INIGRP
	call	DISSCR
; screen ,2
	ld	hl, RG1SAV
	set	1, [hl]
; screen ,,0
	xor	a
	ld	[CLIKSW], a
	
	call	ENASCR
MAIN_LOOP:
; init control sprites
	ld	hl, CONTROL_SPRTBL_DATA
	ld	de, SPRTBL
	ld	bc, CONTROL_SPRTBL_DATA_SIZE
	call	LDIRVM
; init
	ld	hl, SPRTBL_DATA
	ld	[sprtbl_ram_cursor], hl
	ld	hl, SPRATR_DATA
	ld	[spratr_ram_cursor], hl
	ld	a, SPRITE_COUNT
	ld	[sprite_count], a
	
BLIT_GROUP:
	ld	hl, SPRTBL + CONTROL_SPRTBL_DATA_SIZE ; skips control sprites
	ld	[sprtbl_vram_cursor], hl
	ld	hl, SPRATR
	ld	[spratr_vram_cursor], hl
	
BLIT_ONE:
; SPRATR
	ld	de, [spratr_ram_cursor]
	ld	hl, [spratr_vram_cursor]
; group finished
	ld	a, [de]
	cp	SPAT_END
	jp	z, GROUP_END
; y
	ld	a, [de]
	add	192 / 2
	call	WRTVRM
	inc	de
	inc	hl
; x
	ld	a, [de]
	add	256 / 4
	call	WRTVRM
	inc	de
	inc	hl
; pattern
	ld	a, [de]
	add	16 ; skips control sprites
	call	WRTVRM
	inc	de
	inc	hl
; color
	ld	a, [de]
	call	WRTVRM
	inc	de
	inc	hl
; update cursors
	ld	[spratr_ram_cursor], de
	ld	[spratr_vram_cursor], hl

; SPRTBL
	ld	hl, [sprtbl_ram_cursor]
	ld	de, [sprtbl_vram_cursor]
	ld	bc, 32
	call	LDIRVM
; update cursors
	ld	bc, 32
	ld	hl, [sprtbl_ram_cursor]
	add	hl, bc
	ld	[sprtbl_ram_cursor], hl
	ld	hl, [sprtbl_vram_cursor]
	add	hl, bc
	ld	[sprtbl_vram_cursor], hl
	
; update counter
	ld	hl, sprite_count
	dec	[hl]
; next sprite	
	jr	BLIT_ONE
	
GROUP_END:
IFDEF USE_CONTROL_SPRITES
; blit control sprites
	push	de
	ex	de, hl
	ld	hl, CONTROL_SPRATR_DATA
	ld	bc, CONTROL_SPRATR_DATA_SIZE
	call	LDIRVM
	pop	de
	
ELSE
; blit the SPAT_END
	call	WRTVRM
	
ENDIF

	inc	de
	ld	[spratr_ram_cursor], de
; pause
	ld	b, 20
PAUSE_LOOP:
	halt
	djnz	PAUSE_LOOP

; more groups?
	ld	a, [sprite_count]
	or	a
	jp	nz, BLIT_GROUP ; yes
; no
	jp	MAIN_LOOP
	
; =============================================================================
CONTROL_SPRATR_DATA:
	.db	72 -1,	148,	$00,	5
	.db	72 -1,	168,	$04,	6
	.db	72 -1,	192,	$08,	7
	.db	72 -1,	212,	$0c,	8
	.db	88 -1,	152,	$00,	5
	.db	88 -1,	172,	$04,	6
	.db	88 -1,	196,	$08,	7
	.db	88 -1,	216,	$0c,	8
	.db	104 -1,	156,	$00,	5
	.db	104 -1,	176,	$04,	6
	.db	104 -1,	200,	$08,	7
	.db	104 -1,	220,	$0c,	8
	.db	SPAT_END
	CONTROL_SPRATR_DATA_SIZE equ $ - CONTROL_SPRATR_DATA
	
CONTROL_SPRTBL_DATA:
	.db	$FF, $FF, $C0, $CF, $CF, $CF, $CF, $CF
	.db	$C0, $C7, $FF, $FF, $C7, $E0, $FF, $FF
	.db	$FF, $FF, $0F, $CF, $FF, $FF, $FF, $FF
	.db	$0F, $87, $C3, $C3, $87, $0F, $FF, $FF
	.db	$FF, $FF, $E0, $C7, $CF, $CF, $CF, $CF
	.db	$C0, $C7, $CF, $CF, $C7, $E0, $FF, $FF
	.db	$FF, $FF, $0F, $CF, $FF, $FF, $FF, $FF
	.db	$0F, $87, $C3, $C3, $87, $0F, $FF, $FF
	.db	$FF, $FF, $C0, $CF, $FF, $FF, $FF, $FF
	.db	$FF, $FE, $FC, $FC, $FC, $FC, $FF, $FF
	.db	$FF, $FF, $03, $C3, $C3, $C3, $C3, $83
	.db	$07, $0F, $1F, $3F, $3F, $3F, $FF, $FF
	.db	$FF, $FF, $E0, $C7, $CF, $CF, $CF, $C7
	.db	$E0, $C7, $CF, $CF, $C7, $E0, $FF, $FF
	.db	$FF, $FF, $0F, $87, $C3, $C3, $C3, $87
	.db	$0F, $87, $C3, $C3, $87, $0F, $FF, $FF
	CONTROL_SPRTBL_DATA_SIZE equ $ - CONTROL_SPRTBL_DATA

SPRATR_DATA:
	.include "pcx2sprplus_demo\demo.pcx.spat.asm"

SPRTBL_DATA:
	.incbin	"pcx2sprplus_demo\demo.pcx.spr"
	SPRITE_COUNT equ ($ - SPRTBL_DATA) / 32

; =============================================================================

;
; =============================================================================
;	RAM
; =============================================================================
;

	.org	$e000
	
spratr_ram_cursor:
	.word
sprtbl_ram_cursor:
	.word
spratr_vram_cursor:
	.word
sprtbl_vram_cursor:
	.word
sprite_count:
	.byte


; EOF
