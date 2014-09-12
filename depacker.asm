;-------------------------------------------------------------------------------
; prepack6502 decruncher
;
;	http://github.com/alexkazik/prepack6502
;
;	Author:
;		ALeX Kazik / http://alex.kazik.de/
;
;	Licence:
;		Creative Commons Attribution 3.0 Unported License
;		http://creativecommons.org/licenses/by/3.0/
;
;	History:
;		v1.1 2014-09-xx
;			initial use of an assembler
;
; NOTE: This code is in the format of Magnus Lind's assembler from Exomizer.
; Does not directly compile on DASM etc.
;-------------------------------------------------------------------------------

	.ORG (ADDR_BASE)

	; copy the table into the zeropage
	ldx # 2*6 - 1
copyloop:
	lda table_of_tables, x
	sta <ZP_TEMP, x
	dex
	bpl copyloop

	; init the output pointer
	ldy # ADDR_DEST_LO

	; main loop
mainloop:

	; check eof (longer+slower way)
	.IF (EOF_MARKER > 255)
		cpy # ADDR_DEST_END_LO
		bne noeof
		lda smc_out+2
		cmp # ADDR_DEST_END_HI
		beq go_rts
noeof:
	.ENDIF

	; read a byte from the opcode table and output it
	ldx # $0a
	jsr read_and_output

	; check eof (shorter way, only possible if not all opcodes are used)
	tax
	.IF (EOF_MARKER < 256)
		.IF (EOF_MARKER != 0)
			cpx # EOF_MARKER
		.ENDIF
		beq go_rts
	.ENDIF

	; output arguments & loop
	jsr copy
	bne mainloop ; branches always!



	; copy the arguments of an opcode (opcode in x)
copy:
	; determine from which table (and how many arguments)
	lda ADDR_TABLE_ARGTABLE, x
	; implied assressing mode -> done
	bmi go_rts
	; check for assressing mode and skip if not absolute (output one argument byte)
	tax
	bne read_and_output
	; output the lo byte of the absolute argument
	jsr read_and_output
	; advance to the hi byte table
	inx
	inx
	; output the hi byte

	; read one byte from a table (x) and output it
read_and_output:
	; read a byte from a table and inc it
	lda (ZP_TEMP, x)
	inc <ZP_TEMP+0, x
	bne no_inc
	inc <ZP_TEMP+1, x
no_inc:

	; store a byte and inc the position
smc_out:
	sta ADDR_DEST_AND_FF00, y
	iny
	bne go_rts
	inc smc_out+2
go_rts:
	rts

	; the start point for all tables (will be copied)
table_of_tables:
	.BYTE (ADDR_TABLE_ABS_LO_LO, ADDR_TABLE_ABS_LO_HI)
	.BYTE (ADDR_TABLE_ABS_HI_LO, ADDR_TABLE_ABS_HI_HI)
	.BYTE (ADDR_TABLE_IMM_LO, ADDR_TABLE_IMM_HI)
	.BYTE (ADDR_TABLE_REL_LO, ADDR_TABLE_REL_HI)
	.BYTE (ADDR_TABLE_ZP_LO, ADDR_TABLE_ZP_HI)
	.BYTE (ADDR_TABLE_OP_LO, ADDR_TABLE_OP_HI)

