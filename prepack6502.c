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

/*
	Uncomment this line to disable the exomizer support
	#define DISABLE_EXOMIZER
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#ifndef DISABLE_EXOMIZER
#include <sys/stat.h>
#include <libgen.h>
#include <unistd.h>
#endif

#include "asm/membuf.h"
#include "compiler.h"

#define MAX_PRG_SIZE 32768

#define MODEabs    0x00
#define MODEimm    0x04
#define MODErel    0x06
#define MODEzp     0x08
#define MODEimpl   0x80
#define MODEjam   0x180

const int opcode_to_mode[256] = {
	MODEimpl, MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEabs,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEimpl, MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEimpl, MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEimm,  MODEzp,   MODEimm,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEjam,  MODEjam,  MODEabs,  MODEjam,  MODEjam,
	MODEimm,  MODEzp,   MODEimm,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEjam,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEjam,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEimm,  MODEzp,   MODEimm,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODEimm,  MODEzp,   MODEimm,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEimm,  MODEimpl, MODEimm,  MODEabs,  MODEabs,  MODEabs,  MODEabs,
	MODErel,  MODEzp,   MODEjam,  MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEzp,   MODEimpl, MODEabs,  MODEimpl, MODEabs,  MODEabs,  MODEabs,  MODEabs,  MODEabs
};

#ifndef DISABLE_EXOMIZER
int exomize(char *exomizer_cmd, const char *filename){
	char *dest_filename = malloc(strlen(filename)+5);
	strcpy(dest_filename, filename);
	if(strlen(dest_filename) > 4 && strcmp(dest_filename+strlen(dest_filename)-4, ".prg") == 0){
		// filename ends with .prg -> replace it
		strcpy(dest_filename+strlen(dest_filename)-4, ".exo");
	}else{
		// append .exo to the filename
		strcpy(dest_filename+strlen(dest_filename), ".exo");
	}

	int pid = fork();

	if(pid == 0){
		// this is the child process
		// close stdin+out
		fclose(stdin);
		fclose(stdout);
		// start the exomizer
		execlp(exomizer_cmd, basename(exomizer_cmd), "mem", "-c", "-l", "none", "-o", dest_filename, filename, NULL);
		// this only occurs when exec fails
		exit(126);
	}

	// this is the parent process
	int status;
    waitpid(pid, &status, 0);
    if(! WIFEXITED(status)) {
		fprintf(stderr, "Error: The child was killed or segfaulted or something\n");
		exit(1);
    }

    status = WEXITSTATUS(status);

	if(status != 0){
		fprintf(stderr, "Error: Executing exomizer failed\n");
		exit(1);
	}

	struct stat buf;
	if(stat(dest_filename, &buf)){
		fprintf(stderr, "Error: unable to stat file \"%s\": %s\n", dest_filename, strerror(errno));
		exit(1);
	}

	free(dest_filename);

	return buf.st_size;
}
#endif

void usage(const char *progname){
	fprintf(stderr, "Usage: %s [options]\n", progname);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "    -h, --help             show this help\n");
	fprintf(stderr, "    -i, --input <file>     specifies the input file, defaults to stdin\n");
	fprintf(stderr, "    -o, --output <file>    specifies the output file, defaults to stdout\n");
	fprintf(stderr, "    -p, --output2 <file>   if enabled the output is splitted to be better crunched\n");
	fprintf(stderr, "    -t, --tempaddr <addr>  specify the temporary address, defaults to $1000\n");
	fprintf(stderr, "    -T, --tempaddr2 <addr> specify the temporary address, defaults to off or behind part one if -p is used\n");
	fprintf(stderr, "    -z, --zpaddr <addr>    specify the temporary zerpage address, defaults to $f4\n");
	fprintf(stderr, "    -O, --tableorder <lst> specify the order of tables, defaults to \"do.tlirzh\"\n");
	fprintf(stderr, "    -M, --memmap           print memmap\n");
#ifndef DISABLE_EXOMIZER
	fprintf(stderr, "    -e, --exomize[=<pte>]  compress the output(s), optionally specify the /path/to/exomizer\n");
	fprintf(stderr, "    -s, --stats            print stats, enables -e\n");
#endif
}

int main(int argc, char **argv){

	int i;

	/* options descriptor */
	static const struct option longopts[] = {
		{ "help",		no_argument,		NULL,	'h'},
		{ "input",		required_argument,	NULL,	'i'},
		{ "output",		required_argument,	NULL,	'o'},
		{ "output2",	required_argument,	NULL,	'p'},
		{ "tempaddr",	required_argument,	NULL,	't'},
		{ "tempaddr2",	required_argument,	NULL,	'T'},
		{ "zpaddr",		required_argument,	NULL,	'z'},
		{ "tableorder",	required_argument,	NULL,	'O'},
		{ "memmap",		no_argument,		NULL,	'M'},
#ifndef DISABLE_EXOMIZER
		{ "exomize",	optional_argument,	NULL,	'e'},
		{ "stats",		no_argument,		NULL,	's'},
#endif
		{ NULL,			0,					NULL,	0 }
	};

#ifndef DISABLE_EXOMIZER
#define getopt_opts "i:o:p:t:T:z:O:Me::s"
#else
#define getopt_opts "i:o:p:t:T:z:O:M"
#endif

	char *filename_input = NULL;
	char *filename_output1 = NULL;
	char *filename_output2 = NULL;
	int tempaddr = 0x1000;
	int tempaddr2 = 0;
	int zpaddr = 0x0100-12;
	int do_exomize = 0;
	char *exomizer_cmd = "exomizer";
	int do_stats = 0;
	int do_split = 0;
	int show_memmap = 0;

	char *table_order = "do.tlirzh";
#define TABLE_ORDER_LENGTH 9

	/*
	** read and check arguments
	*/

	int ch;
	while((ch = getopt_long(argc, argv, getopt_opts, longopts, NULL)) != -1){
		switch(ch){
		case 'i':
			filename_input = optarg;
			break;
		case 'o':
			filename_output1 = optarg;
			break;
		case 'p':
			filename_output2 = optarg;
			break;
		case 't':
			if(optarg[0] == '$'){
				tempaddr = strtol(optarg+1, NULL, 16);
			}else{
				tempaddr = strtol(optarg, NULL, 0);
			}
			if(tempaddr < 0x0200 || tempaddr > 0xff00){
				fprintf(stderr, "Error: tempaddr is an invalid address\n");
				exit(1);
			}
			break;
		case 'T':
			if(optarg[0] == '$'){
				tempaddr2 = strtol(optarg+1, NULL, 16);
			}else{
				tempaddr2 = strtol(optarg, NULL, 0);
			}
			if(tempaddr2 < 0x0200 || tempaddr2 > 0xff00){
				fprintf(stderr, "Error: tempaddr2 is an invalid address\n");
				exit(1);
			}
			break;
		case 'z':
			if(optarg[0] == '$'){
				zpaddr = strtol(optarg+1, NULL, 16);
			}else{
				zpaddr = strtol(optarg, NULL, 0);
			}
			if(zpaddr < 0x0000 || zpaddr > 0x0100-12){
				fprintf(stderr, "Error: zpaddr is an invalid address (valid: $00-$f4)\n");
				exit(1);
			}
			break;
		case 'O':
			{
				int tcnt[TABLE_ORDER_LENGTH];
				for(int i=0; i<TABLE_ORDER_LENGTH; i++){
					tcnt[i] = 0;
				}
				for(int i=0; i<strlen(optarg); i++){
					char tmp[2];
					tmp[0] = optarg[i];
					tmp[1] = 0;
					char* pos = strcasestr(table_order, tmp);
					if(pos == NULL){
						fprintf(stderr, "Error: tableorder contains an unknown table id \"%c\"\n", optarg[i]);
						exit(1);
					}else{
						tcnt[pos - table_order]++;
					}
				}
				for(int i=0; i<TABLE_ORDER_LENGTH; i++){
					if(tcnt[i] == 0){
						fprintf(stderr, "Error: tableorder does not contain table id \"%c\"\n", table_order[i]);
						exit(1);
					}else if(tcnt[i] > 1){
						fprintf(stderr, "Error: tableorder does contain table id \"%c\" more than once\n", table_order[i]);
						exit(1);
					}
				}
				if(optarg[0] == '.'){
					fprintf(stderr, "Error: tableorder can't have the separator as first element\n");
					exit(1);
				}
				if(optarg[TABLE_ORDER_LENGTH-1] == '.'){
					fprintf(stderr, "Error: tableorder can't have the separator as first element\n");
					exit(1);
				}
				table_order = optarg;
			}
			break;
		case 'M':
			show_memmap = 1;
			break;
		case 'h':
			usage(argv[0]);
			return 0;
			break;
#ifndef DISABLE_EXOMIZER
		case 'e':
			do_exomize = 1;
			if(optarg != NULL){
				exomizer_cmd = optarg;
			}
			break;
		case 's':
			do_stats = 1;
			do_exomize = 1;
			break;
#endif
		default:
			usage(argv[0]);
			exit(1);
			break;
		}
	}

	if(argc != optind){
		usage(argv[0]);
		exit(1);
	}

	if(do_exomize && filename_output1 == NULL){
		fprintf(stderr, "Error: can't exomize stdout\n");
		exit(1);
	}

	if(do_stats && filename_input == NULL){
		fprintf(stderr, "Error: can't exomize stdin\n");
		exit(1);
	}

	if(filename_output2 != NULL){
		do_split = 1;
	}

	/*
	** open the input
	*/

	FILE *file_input;

	if(filename_input != NULL){
		file_input = fopen(filename_input, "rb");
		if(file_input == NULL){
			fprintf(stderr, "Error: unable to open file \"%s\": %s\n", filename_input, strerror(errno));
			exit(1);
		}
	}else{
		file_input = stdin;
		filename_input = "stdin";
	}

	/*
	** read the program
	*/

	uint8_t program_data[MAX_PRG_SIZE];
	int program_addr, program_len;

	if(2 != fread(program_data, 1, 2, file_input)){
		if(ferror(file_input)){
			fprintf(stderr, "Error: reading file \"%s\": %s\n", filename_input, strerror(ferror(file_input)));
			exit(1);
		}else{
			fprintf(stderr, "Error: reading file \"%s\": file too short\n", filename_input);
			exit(1);
		}
	}
	program_addr = program_data[0] + (program_data[1] << 8);

	program_len = fread(program_data, 1, MAX_PRG_SIZE, file_input);
	if(ferror(file_input)){
		fprintf(stderr, "Error: reading file \"%s\": %s\n", filename_input, strerror(ferror(file_input)));
		exit(1);
	}else if(program_len >= MAX_PRG_SIZE - 100){
		fprintf(stderr, "Error: file \"%s\" is too long\n", filename_input);
		exit(1);
	}

	if(program_addr + program_len > 0x10000){
		fprintf(stderr, "Error: file \"%s\" exceeds $ffff\n", filename_input);
		exit(1);
	}

	/*
	** split program and count used opcodes
	*/

	int used_opcodes[256];
	int count_op = 0, count_abs = 0, count_imm = 0, count_rel = 0, count_zp = 0;
	uint8_t data_op[MAX_PRG_SIZE], data_abs_lo[MAX_PRG_SIZE], data_abs_hi[MAX_PRG_SIZE];
	uint8_t data_imm[MAX_PRG_SIZE], data_rel[MAX_PRG_SIZE], data_zp[MAX_PRG_SIZE];

	for(i=0; i<256; i++){
		used_opcodes[i] = 0;
	}

	for(i=0; i<program_len; i++){
		// we've seen this opcode
		used_opcodes[program_data[i]] = 1;
		// add the opcode to the output
		data_op[count_op++] = program_data[i];
		// split arguments
		switch(opcode_to_mode[program_data[i]]){
		case MODEimpl:
			// argument is implied -> nothing to do
			break;
		case MODEabs:
			// argument is absolue address = two bytes
			data_abs_lo[count_abs] = program_data[++i];
			data_abs_hi[count_abs++] = program_data[++i];
			break;
		case MODEimm:
			// argument is immediate = one byte
			data_imm[count_imm++] = program_data[++i];
			break;
		case MODErel:
			// argument is relative address = one byte
			data_rel[count_rel++] = program_data[++i];
			break;
		case MODEzp:
			// argument is zeropage address = one byte
			data_zp[count_zp++] = program_data[++i];
			break;
		case MODEjam:
			// warn JAM/instable
			fprintf(stderr, "Use of the instable opcode $%02x at $%04x\n", program_data[i], program_addr+i);
			break;
		}
	}

	/*
	** find an EOF marker
	*/

	int eof_marker;

	for(eof_marker=0; eof_marker<256 && used_opcodes[eof_marker]!=0; eof_marker++);

	if(eof_marker == 256){
		// the pointer will be checked every op
		fprintf(stderr, "Warning: all opcodes are used, will make decruncher slower\n");
	}else{
		// use the marker -> place it
		data_op[count_op++] = eof_marker;
	}

	int real_program_len = count_op + 2*count_abs + count_imm + count_rel + count_zp;

	/*
	** trim and generate the argtable
	*/

	int count_argtable, argtable_start;

	for(count_argtable=256; used_opcodes[count_argtable-1] == 0; count_argtable--);
	for(argtable_start=0; used_opcodes[argtable_start] == 0; argtable_start++, count_argtable--);

	uint8_t data_argtable[256];
	for(i=0; i<256; i++){
		data_argtable[i] = used_opcodes[i] != 0 ? (opcode_to_mode[i] & 0xff) : 0x00;
	}

	/*
	** generate the decruncher, temporarily
	*/

	struct membuf* compiled_program;
	compiled_program = gen_decruncher(
		eof_marker, zpaddr, // the important stuff
		0x1000, 0xffff, 0xffff, 0xffff, // dummies for program base, destination/-end and argtable
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff // dummies for the 6 tables
	);
	int count_depack = membuf_memlen(compiled_program);
	membuf_delete(&compiled_program);

	/*
	** calculate table offsets and place them in the code
	*/

	int table_depack;
	int table_argtable;
	int table_abs_lo;
	int table_abs_hi;
	int table_imm;
	int table_rel;
	int table_zp;
	int table_op;

	int part1_start = tempaddr;
	int part1_end = 0;
	int part2_start = tempaddr;
	int part2_end = 0;

	if(show_memmap){
		fprintf(stderr, "\nMemory Map\n----------\n");
		fprintf(stderr, " %04x-%04x %5d Temporary\n", zpaddr, zpaddr+11, 12);
		if(program_addr < part1_start){
			fprintf(stderr, " %04x-%04x %5d Decrunched Program\n", program_addr, program_addr+program_len-1, program_len);
			fprintf(stderr, " %04x-%04x %5d Temporary\n", program_addr+program_len, program_addr+real_program_len-1, real_program_len-program_len);
		}
	}

	int cur_address = tempaddr;
	for(int i=0; i<TABLE_ORDER_LENGTH; i++){
		switch(table_order[i]){
		case 'd':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Depacker\n", cur_address, cur_address+count_depack-1, count_depack);
			}
			table_depack = cur_address;
			cur_address += count_depack;
			break;
		case 't':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Argument Table\n", cur_address, cur_address+count_argtable-1, count_argtable);
			}
			table_argtable = cur_address;
			cur_address += count_argtable;
			break;
		case 'l':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for abs, lo\n", cur_address, cur_address+count_abs-1, count_abs);
			}
			table_abs_lo = cur_address;
			cur_address += count_abs;
			break;
		case 'h':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for abs, hi\n", cur_address, cur_address+count_abs-1, count_abs);
			}
			table_abs_hi = cur_address;
			cur_address += count_abs;
			break;
		case 'i':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for imm\n", cur_address, cur_address+count_imm-1, count_imm);
			}
			table_imm = cur_address;
			cur_address += count_imm;
			break;
		case 'r':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for rel\n", cur_address, cur_address+count_rel-1, count_rel);
			}
			table_rel = cur_address;
			cur_address += count_rel;
			break;
		case 'z':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for zp\n", cur_address, cur_address+count_zp-1, count_zp);
			}
			table_zp = cur_address;
			cur_address += count_zp;
			break;
		case 'o':
			if(show_memmap){
				fprintf(stderr, " %04x-%04x %5d Table for opcode\n", cur_address, cur_address+count_op-1, count_op);
			}
			table_op = cur_address;
			cur_address += count_op;
			break;
		case '.':
			if(do_split){
				part1_end = cur_address;
				if(tempaddr2 != 0){
					cur_address = part2_start = tempaddr2;
				}else{
					part2_start = cur_address;
				}

				if(show_memmap && program_addr > part1_start && program_addr < part2_start){
					fprintf(stderr, " %04x-%04x %5d Decrunched Program\n", program_addr, program_addr+program_len-1, program_len);
					fprintf(stderr, " %04x-%04x %5d Temporary\n", program_addr+program_len, program_addr+real_program_len-1, real_program_len-program_len);
				}
			}
			break;
		}
	}
	if(do_split){
		part2_end = cur_address;
	}else{
		part1_end = cur_address;
	}

	if(show_memmap){
		if(program_addr > part2_start){
			fprintf(stderr, " %04x-%04x %5d Decrunched Program\n", program_addr, program_addr+program_len-1, program_len);
			fprintf(stderr, " %04x-%04x %5d Temporary\n", program_addr+program_len, program_addr+real_program_len-1, real_program_len-program_len);
		}
		fprintf(stderr, "\n");
	}

	/*
	** compile the depacker
	*/

	compiled_program = gen_decruncher(
		eof_marker, zpaddr,
		table_depack, program_addr, program_addr+real_program_len, table_argtable-argtable_start,
		table_abs_lo, table_abs_hi, table_imm, table_rel, table_zp, table_op
	);
	if(count_depack != membuf_memlen(compiled_program)){
		fprintf(stderr, "Error: compile error\n");
		exit(1);
	}

	/*
	** write data
	*/

	FILE *file_output1, *file_output2;
	int close_output1 = 0, close_output2 = 0;

	if(filename_output1 != NULL){
		file_output1 = fopen(filename_output1, "wb");
		if(file_output1 == NULL){
			fprintf(stderr, "Error: unable to open file \"%s\": %s\n", filename_output1, strerror(errno));
			exit(1);
		}
		close_output1 = 1;
	}else{
		file_output1 = stdout;
		filename_output1 = "stdout";
	}

	// load address (reuse the source program buffer)
	program_data[0] = part1_start & 0xff;
	program_data[1] = part1_start >> 8;
	if(2 != fwrite(program_data, 1, 2, file_output1)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output1, strerror(errno));
		exit(1);
	}

	if(do_split){
		file_output2 = fopen(filename_output2, "wb");
		if(file_output2 == NULL){
			fprintf(stderr, "Error: unable to open file \"%s\": %s\n", filename_output2, strerror(errno));
			exit(1);
		}
		close_output2 = 1;

		// load address (reuse the source program buffer)
		program_data[0] = part2_start & 0xff;
		program_data[1] = part2_start >> 8;
		if(2 != fwrite(program_data, 1, 2, file_output2)){
			fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
			exit(1);
		}
	}

	FILE* file_current = file_output1;
	char* filename_current = filename_output1;

	for(int i=0; i<TABLE_ORDER_LENGTH; i++){
		switch(table_order[i]){
		case 'd':
			// depacker
			if(count_depack != fwrite(membuf_get(compiled_program), 1, count_depack, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 't':
			// table: argtable
			if(count_argtable != fwrite(data_argtable+argtable_start, 1, count_argtable, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'l':
			// table: abs_lo
			if(count_abs != fwrite(data_abs_lo, 1, count_abs, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'h':
			// table: abs_hi
			if(count_abs != fwrite(data_abs_hi, 1, count_abs, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'i':
			// table: imm
			if(count_imm != fwrite(data_imm, 1, count_imm, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'r':
			// table: rel
			if(count_rel != fwrite(data_rel, 1, count_rel, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'z':
			// table: zp
			if(count_zp != fwrite(data_zp, 1, count_zp, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case 'o':
			// table: op
			if(count_op != fwrite(data_op, 1, count_op, file_current)){
				fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_current, strerror(errno));
				exit(1);
			}
			break;
		case '.':
			if(do_split){
				file_current = file_output2;
				filename_current = filename_output2;
			}
			break;
		}
	}

	// close real files
	if(close_output1){
		fclose(file_output1);
	}
	if(close_output2){
		fclose(file_output2);
	}

	// free compiled program
	membuf_delete(&compiled_program);

	/*
	** print stats
	*/

	if(program_addr+program_len == program_addr+real_program_len){
		fprintf(stderr, "Temporary: $%02x-$%02x\n", zpaddr, zpaddr+11);
	}else if(program_addr+program_len+1 == program_addr+real_program_len){
		fprintf(stderr, "Temporary: $%02x-$%02x and $%04x\n", zpaddr, zpaddr+11, program_addr+program_len);
	}else{
		fprintf(stderr, "Temporary: $%02x-$%02x and $%04x-$%04x\n", zpaddr, zpaddr+11, program_addr+program_len, program_addr+real_program_len-1);
	}

#ifndef DISABLE_EXOMIZER
	int exo_input_size;
	if(do_exomize && do_stats){
		exo_input_size = exomize(exomizer_cmd, filename_input);

		fprintf(stderr, "Program:   $%04x-$%04x, %d bytes, crunched %d bytes\n", program_addr, program_addr+program_len-1, program_len, exo_input_size);
	}else{
#endif
		fprintf(stderr, "Program:   $%04x-$%04x, %d bytes\n", program_addr, program_addr+program_len-1, program_len);
#ifndef DISABLE_EXOMIZER
	}
#endif

#ifndef DISABLE_EXOMIZER
	if(do_exomize){
		int size = exomize(exomizer_cmd, filename_output1);
		if(do_split){
			size += exomize(exomizer_cmd, filename_output2);
		}

		if(do_split){
			if(do_stats){
				fprintf(stderr,
					"Depacker:  $%04x-$%04x, %d bytes, $%04x-$%04x, %d bytes, total %d bytes, crunched %d bytes, saved %d bytes\n",
					part1_start,
					part1_end-1,
					part1_end-part1_start,
					part2_start,
					part2_end-1,
					part2_end-part2_start,
					(part1_end-part1_start) + (part2_end-part2_start),
					size,
					exo_input_size-size
				);
			}else{
				fprintf(stderr,
					"Depacker:  $%04x-$%04x, %d bytes, $%04x-$%04x, %d bytes, crunched %d bytes\n",
					part1_start,
					part1_end-1,
					part1_end-part1_start,
					part2_start,
					part2_end-1,
					part2_end-part2_start,
					size
				);
			}
		}else{
			if(do_stats){
				fprintf(stderr,
					"Depacker:  $%04x-$%04x, %d bytes, crunched %d bytes, saved %d bytes\n",
					part1_start,
					part1_end-1,
					part1_end-part1_start,
					size,
					exo_input_size-size
				);
			}else{
				fprintf(stderr,
					"Depacker:  $%04x-$%04x, %d bytes, crunched %d bytes\n",
					part1_start,
					part1_end-1,
					part1_end-part1_start,
					size
				);
			}
		}
	}else{
#endif
		if(do_split){
				fprintf(stderr,
					"Depacker:  $%04x-$%04x, %d bytes, $%04x-$%04x, %d bytes, total %d bytes\n",
					part1_start,
					part1_end-1,
					part1_end-part1_start,
					part2_start,
					part2_end-1,
					part2_end-part2_start,
					(part1_end-part1_start) + (part2_end-part2_start)
				);
		}else{
			fprintf(stderr,
				"Depacker: $%04x-$%04x, %d bytes\n",
				part1_start,
				part1_end-1,
				part1_end-part1_start
			);
		}
#ifndef DISABLE_EXOMIZER
	}
#endif

	if(
		(part1_start <= program_addr && part1_end > program_addr) ||
		(part1_start >= program_addr && program_addr+real_program_len > part1_start)
	){
		fprintf(stderr, "Error: the program and temporary spaces are overlaping\n");
		exit(1);
	}

	if(do_split){
		if(
			(program_addr <= part2_start && program_addr+real_program_len > part2_start) ||
			(program_addr >= part2_start && part2_end > program_addr)
		){
			fprintf(stderr, "Error: the program and temporary spaces are overlaping\n");
			exit(1);
		}
	}

	if(table_depack != part1_start){
		fprintf(stderr, "Notice: The depacker is not at the start of the program but at $%04x instead\n", table_depack);
	}

	if(part1_end > 0x10000){
		fprintf(stderr, "Error: temporary space exceeds $ffff\n");
		exit(1);
	}

	if(part2_end > 0x10000){
		fprintf(stderr, "Error: second temporary space exceeds $ffff\n");
		exit(1);
	}

	if(
		do_split &&
		(
			(part1_start <= part2_start && part1_end > part2_start) ||
			(part1_start >= part2_start && part2_end > part1_start)
		)
	){
		fprintf(stderr, "Error: the two temporary spaces are overlaping\n");
		exit(1);
	}

	return 0;

}
