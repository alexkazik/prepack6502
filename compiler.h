
struct membuf* gen_decruncher(
	int eof_marker, int zpaddr,
	int program_base, int destination, int destination_end, int table_argtable,
	int table_abs_lo, int table_abs_hi, int table_imm,
	int table_rel, int table_zp, int table_op
);
