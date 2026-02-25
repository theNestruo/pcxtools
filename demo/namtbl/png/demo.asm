; -----------------------------------------------------------------------------
; PNG2MSX demo (by theNestruo)
; MSX cartridge / 8kB/16kB ROM / 8kB RAM
; Syntax: asMSX (v0.16 WIP, Eduardo A. Robsy Petrus)
; -----------------------------------------------------------------------------

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

; horizontal
	ld	hl, HORIZONTAL_CHRTBL
	call	LDIRVM_CHRTBL
	ld	hl, HORIZONTAL_CLRTBL
	call	LDIRVM_CLRTBL
	ld	hl, HORIZONTAL_NAMTBL
	call	LDIRVM_NAMTBL

; pause
	call	 PAUSE

; vertical
	ld	hl, VERTICAL_CHRTBL
	call	LDIRVM_CHRTBL
	ld	hl, VERTICAL_CLRTBL
	call	LDIRVM_CLRTBL
	ld	hl, VERTICAL_NAMTBL
	call	LDIRVM_NAMTBL

; pause
	call	PAUSE

	jp	MAIN_LOOP

LDIRVM_CHRTBL:
	ld	de, CHRTBL + $0000
	call	LDIRVM_ONE_BANK
	ld	de, CHRTBL + $0800
	call	LDIRVM_ONE_BANK
	ld	de, CHRTBL + $1000
	jp	LDIRVM_ONE_BANK

LDIRVM_CLRTBL:
	ld	de, CLRTBL + $0000
	call	LDIRVM_ONE_BANK
	ld	de, CLRTBL + $0800
	call	LDIRVM_ONE_BANK
	ld	de, CLRTBL + $1000

LDIRVM_ONE_BANK:
	push	hl
	ld	bc, $0800
	call	LDIRVM
	pop	hl
	ret

LDIRVM_NAMTBL:
	ld	de, NAMTBL
	ld	bc, $0300
	jp	LDIRVM

PAUSE:
	ld	b, 20
PAUSE_LOOP:
	halt
	djnz	PAUSE_LOOP
	ret

; =============================================================================
HORIZONTAL_CHRTBL:
	.incbin "demo/namtbl/png/screen-th.png.chr"
HORIZONTAL_CLRTBL:
	.incbin "demo/namtbl/png/screen-th.png.clr"
HORIZONTAL_NAMTBL:
	.incbin "demo/namtbl/png/screen-th.png.nam"

VERTICAL_CHRTBL:
	.incbin "demo/namtbl/png/screen-tv.png.chr"
VERTICAL_CLRTBL:
	.incbin "demo/namtbl/png/screen-tv.png.clr"
VERTICAL_NAMTBL:
	.incbin "demo/namtbl/png/screen-tv.png.nam"
; =============================================================================

;
; =============================================================================
;	RAM
; =============================================================================
;

	.org	$e000

; EOF
