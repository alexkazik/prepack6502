/*

		** prepack6502 **
	http://github.com/alexkazik/prepack6502

	Author:
		ALeX Kazik / http://alex.kazik.de/

	Licence:
		Creative Commons Attribution 3.0 Unported License
		http://creativecommons.org/licenses/by/3.0/

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from
	the use of this software.

*/

#include "asm/membuf.h"
#include "asm/parse.h"

#include "depacker.h"

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>

struct membuf* gen_decruncher(
	int eof_marker, int zpaddr,
	int program_base, int destination, int destination_end, int table_argtable,
	int table_abs_lo, int table_abs_hi, int table_imm,
	int table_rel, int table_zp, int table_op
){
	struct membuf source;
	membuf_init(&source);

	struct membuf *dest;
	membuf_new(&dest);

	char buf[100];
	membuf_append(&source, buf, sprintf(buf, "EOF_MARKER = $%02x\n", eof_marker));
	membuf_append(&source, buf, sprintf(buf, "ZP_TEMP = $%02x\n", zpaddr));
	membuf_append(&source, buf, sprintf(buf, "ADDR_BASE = $%04x\n", program_base));
	membuf_append(&source, buf, sprintf(buf, "ADDR_DEST_LO = $%02x\n", destination & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_DEST_AND_FF00 = $%04x\n", destination & 0xff00));
	membuf_append(&source, buf, sprintf(buf, "ADDR_DEST_END_LO = $%02x\n", destination_end & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_DEST_END_HI = $%04x\n", destination_end >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ARGTABLE = $%04x\n", table_argtable));

	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ABS_LO_LO = $%02x\n", table_abs_lo & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ABS_LO_HI = $%02x\n", table_abs_lo >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ABS_HI_LO = $%02x\n", table_abs_hi & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ABS_HI_HI = $%02x\n", table_abs_hi >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_IMM_LO = $%02x\n", table_imm & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_IMM_HI = $%02x\n", table_imm >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_REL_LO = $%02x\n", table_rel & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_REL_HI = $%02x\n", table_rel >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ZP_LO = $%02x\n", table_zp & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_ZP_HI = $%02x\n", table_zp >> 8));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_OP_LO = $%02x\n", table_op & 0x00ff));
	membuf_append(&source, buf, sprintf(buf, "ADDR_TABLE_OP_HI = $%02x\n", table_op >> 8));

	membuf_append(&source, depacker_asm, depacker_asm_len);

	#if 0
		FILE *asmout = fopen("generated_depacker.asm", "w");
		fwrite(membuf_get(&source), membuf_memlen(&source), 1, asmout);
		fclose(asmout);
	#endif

	// Assemble; on error fail in a rude way (the parser does so too)
	if(assemble(&source, dest)){
		exit(1);
	}

	membuf_free(&source);

	return dest;
}
