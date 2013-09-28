/*

		** prepack6502 **
	http://github.com/alexkazik/prepack6502
	
	Author:
		ALeX Kazik / http://alex.kazik.de/

	Licence:
		Creative Commons Attribution 3.0 Unported License
		http://creativecommons.org/licenses/by/3.0/

	History:
		v1.0 2013-09-28
			initial public release

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

const uint8_t depack_eof_zero[] = {
	/* copy the table into the zeropage */
		/* base+$00 */ 0xa2, 0x0b,       /* ldx #$0b */
		/* base+$02 */ 0xbd, 0x38, 0x00, /* lda base+$38,x */
		/* base+$05 */ 0x95, 0x00,       /* sta zptmp,x */
		/* base+$07 */ 0xca,             /* dex */
		/* base+$08 */ 0x10, 0xf8,       /* bpl base+$02 */
	/* init the output pointer */
		/* base+$0a */ 0xa0, 0x00,       /* ldy #dstaddr&$00ff */
	/* read a byte from the opcode table and output it */
		/* base+$0c */ 0xa2, 0x0a,       /* ldx #$0a */
		/* base+$0e */ 0x20, 0x26, 0x00, /* jsr base+$26 */
	/* check eof */
		/* base+$11 */ 0xaa,             /* tax */
		/* base+$12 */ 0xf0, 0x23,       /* beq base+$37 */
	/* output arguments & loop */
		/* base+$14 */ 0x20, 0x19, 0x00, /* jsr base+$19 */
		/* base+$17 */ 0xd0, 0xf3,       /* bne base+$0c */
	/* determine from which table (and how many arguments) */
		/* base+$19 */ 0xbd, 0x00, 0x00, /* lda optable,x */
	/* "impl" -> done */
		/* base+$1c */ 0x30, 0x19,       /* bmi base+$37 */
	/* check for absolute argument and skip if not absolute */
		/* base+$1e */ 0xaa,             /* tax */
		/* base+$1f */ 0xd0, 0x05,       /* bne base+$26 */
	/* output the lo byte of the absolute argument and advance to the hi byte table */
		/* base+$21 */ 0x20, 0x26, 0x00, /* jsr base+$26 */
		/* base+$24 */ 0xe8,             /* inx */
		/* base+$25 */ 0xe8,             /* inx */
	/* read a byte from a table and inc it */
		/* base+$26 */ 0xa1, 0x00,       /* lda (zptmp,x) */
		/* base+$28 */ 0xf6, 0x00,       /* inc zptmp,x */
		/* base+$2a */ 0xd0, 0x02,       /* bne base+$2e */
		/* base+$2c */ 0xf6, 0x01,       /* inc zptmp+1,x */
	/* store a byte and inc the position */
		/* base+$2e */ 0x99, 0x00, 0x00, /* sta dstaddr&$ff00,y */
		/* base+$31 */ 0xc8,             /* iny */
		/* base+$32 */ 0xd0, 0x03,       /* bne base+$37 */
		/* base+$34 */ 0xee, 0x30, 0x00, /* inc base+$30 */
		/* base+$37 */ 0x60,             /* rts */
	/* base+$38 = table of tables */
	/* base+$44 = first table */
};

const uint8_t depack_eof_non_zero[] = {
	/* copy the table into the zeropage */
		/* base+$00 */ 0xa2, 0x0b,       /* ldx #$0b */
		/* base+$02 */ 0xbd, 0x3a, 0x00, /* lda base+$3a,x */
		/* base+$05 */ 0x95, 0x00,       /* sta zptmp,x */
		/* base+$07 */ 0xca,             /* dex */
		/* base+$08 */ 0x10, 0xf8,       /* bpl base+$02 */
	/* init the output pointer */
		/* base+$0a */ 0xa0, 0x00,       /* ldy #dstaddr&$00ff */
	/* read a byte from the opcode table and output it */
		/* base+$0c */ 0xa2, 0x0a,       /* ldx #$0a */
		/* base+$0e */ 0x20, 0x28, 0x00, /* jsr base+$28 */
	/* check eof */
		/* base+$11 */ 0xaa,             /* tax */
		/* base+$12 */ 0xc9, 0x00,       /* cmp #eof_marker */ /* <-- this line is added */
		/* base+$14 */ 0xf0, 0x23,       /* beq base+$39 */
	/* output arguments & loop */
		/* base+$16 */ 0x20, 0x1b, 0x00, /* jsr base+$1b */
		/* base+$19 */ 0xd0, 0xf1,       /* bne base+$0c */
	/* determine from which table (and how many arguments) */
		/* base+$1b */ 0xbd, 0x00, 0x00, /* lda optable,x */
	/* "impl" -> done */
		/* base+$1e */ 0x30, 0x19,       /* bmi base+$39 */
	/* check for absolute argument and skip if not absolute */
		/* base+$20 */ 0xaa,             /* tax */
		/* base+$21 */ 0xd0, 0x05,       /* bne base+$28 */
	/* output the lo byte of the absolute argument and advance to the hi byte table */
		/* base+$23 */ 0x20, 0x28, 0x00, /* jsr base+$28 */
		/* base+$26 */ 0xe8,             /* inx */
		/* base+$27 */ 0xe8,             /* inx */
	/* read a byte from a table and inc it */
		/* base+$28 */ 0xa1, 0x00,       /* lda (zptmp,x) */
		/* base+$2a */ 0xf6, 0x00,       /* inc zptmp,x */
		/* base+$2c */ 0xd0, 0x02,       /* bne base+$30 */
		/* base+$2e */ 0xf6, 0x01,       /* inc zptmp+1,x */
	/* store a byte and inc the position */
		/* base+$30 */ 0x99, 0x00, 0x00, /* sta dstaddr&$ff00,y */
		/* base+$33 */ 0xc8,             /* iny */
		/* base+$34 */ 0xd0, 0x03,       /* bne base+$39 */
		/* base+$36 */ 0xee, 0x32, 0x00, /* inc base+$32 */
		/* base+$39 */ 0x60,             /* rts */
	/* base+$3a = table of tables */
	/* base+$46 = first table */
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
		// close stdout
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
	fprintf(stderr, "    -z, --zpaddr <addr>    specify the temporary zerpage address, defaults to $f4\n");
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
		{ "zpaddr",		required_argument,	NULL,	'z'},
#ifndef DISABLE_EXOMIZER
		{ "exomize",	optional_argument,	NULL,	'e'},
		{ "stats",		no_argument,		NULL,	's'},
#endif
		{ NULL,			0,					NULL,	0 }
	};

#ifndef DISABLE_EXOMIZER
#define getopt_opts "i:o:p:t:z:e::s"
#else
#define getopt_opts "i:o:p:t:z:"
#endif
	
	char *filename_input = NULL;
	char *filename_output1 = NULL;
	char *filename_output2 = NULL;
	int tempaddr = 0x1000;
	int zpaddr = 0x0100-12;
	int do_exomize = 0;
	char *exomizer_cmd = "exomizer";
	int do_stats = 0;

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
		case 'z':
			if(optarg[0] == '$'){
				zpaddr = strtol(optarg+1, NULL, 16);
			}else{
				zpaddr = strtol(optarg, NULL, 0);
			}
			if(zpaddr < 0x0000 || zpaddr > 0x0100-12){
				fprintf(stderr, "Error: zpaddr is an invalid address\n");
				exit(1);
			}
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
	}else if(program_len == MAX_PRG_SIZE){
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
	int count_prg = 0, count_abs = 0, count_imm = 0, count_rel = 0, count_zp = 0;
	uint8_t data_prg[MAX_PRG_SIZE], data_abs_lo[MAX_PRG_SIZE], data_abs_hi[MAX_PRG_SIZE];
	uint8_t data_imm[MAX_PRG_SIZE], data_rel[MAX_PRG_SIZE], data_zp[MAX_PRG_SIZE];
	
	for(i=0; i<256; i++){
		used_opcodes[i] = 0;
	}
	
	for(i=0; i<program_len; i++){
		// we've seen this opcode
		used_opcodes[program_data[i]]++;
		// add the opcode to the output
		data_prg[count_prg++] = program_data[i];
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
	
	for(eof_marker=0; eof_marker<256 && used_opcodes[eof_marker]>0; eof_marker++);
	
	if(eof_marker == 256){
		fprintf(stderr, "Error: all opcodes are used\n");
		exit(1);
	}

	/*
	** trim and generate the optable
	*/

	int optable_len, optable_start;
	
	for(optable_len=256; used_opcodes[optable_len-1] == 0; optable_len--);
	for(optable_start=0; used_opcodes[optable_start] == 0; optable_start++, optable_len--);

	uint8_t data_optable[256];
	for(i=0; i<256; i++){
		data_optable[i] = used_opcodes[i] ? (opcode_to_mode[i] & 0xff) : 0x00;
	}

	/*
	** generate the decruncher
	*/

	uint8_t data_depack[0x47];
	int count_depack;
	int eof_offset;
	
	if(eof_marker == 0){
		for(i=0; i<0x38; i++){
			data_depack[i] = depack_eof_zero[i];
		}
		count_depack = 0x44;
		eof_offset = -2; // we "save" the eof cmp operation
	}else{
		for(i=0; i<0x3a; i++){
			data_depack[i] = depack_eof_non_zero[i];
		}
		count_depack = 0x46;
		eof_offset = 0; // regular
		// set the eof-marker
		data_depack[0x13] = eof_marker;
	}

	/*
	** fix the temporary addresses within the code
	*/

	{
		int addr;
		
		addr = tempaddr + data_depack[0x03+0];
		data_depack[0x03+0] = addr & 0xff;
		data_depack[0x03+1] = addr >> 8;
	
		addr = tempaddr + data_depack[0x0f+0];
		data_depack[0x0f+0] = addr & 0xff;
		data_depack[0x0f+1] = addr >> 8;
	
		addr = tempaddr + data_depack[0x17+eof_offset+0];
		data_depack[0x17+eof_offset+0] = addr & 0xff;
		data_depack[0x17+eof_offset+1] = addr >> 8;
	
		addr = tempaddr + data_depack[0x24+eof_offset+0];
		data_depack[0x24+eof_offset+0] = addr & 0xff;
		data_depack[0x24+eof_offset+1] = addr >> 8;
	
		addr = tempaddr + data_depack[0x37+eof_offset+0];
		data_depack[0x37+eof_offset+0] = addr & 0xff;
		data_depack[0x37+eof_offset+1] = addr >> 8;
	}

	/*
	** fix the zeropage address
	*/

	data_depack[0x06] += zpaddr;
	data_depack[0x29+eof_offset] += zpaddr;
	data_depack[0x2b+eof_offset] += zpaddr;
	data_depack[0x2f+eof_offset] += zpaddr;

	/*
	** set the destination address
	*/
	
	data_depack[0x0b] = program_addr & 0xff;
	data_depack[0x32+eof_offset] = program_addr >> 8;
	
	/*
	** calculate table offsets and place them in the code
	*/

	int table_prg = tempaddr + count_depack;
	int table_optable = table_prg + count_prg;
	int table_abs_lo = table_optable + optable_len;
	int table_imm = table_abs_lo + count_abs;
	int table_rel = table_imm + count_imm;
	int table_zp = table_rel + count_rel;
	int table_abs_hi = table_zp + count_zp;
	int table_eof = table_abs_hi + count_abs;

	data_depack[0x1c + eof_offset] = (table_optable-optable_start) & 0xff;
	data_depack[0x1d + eof_offset] = (table_optable-optable_start) >> 8;

	data_depack[count_depack-12] = table_abs_lo & 0xff;
	data_depack[count_depack-11] = table_abs_lo >> 8;
	data_depack[count_depack-10] = table_abs_hi & 0xff;
	data_depack[count_depack- 9] = table_abs_hi >> 8;
	data_depack[count_depack- 8] = table_imm & 0xff;
	data_depack[count_depack- 7] = table_imm >> 8;
	data_depack[count_depack- 6] = table_rel & 0xff;
	data_depack[count_depack- 5] = table_rel >> 8;
	data_depack[count_depack- 4] = table_zp & 0xff;
	data_depack[count_depack- 3] = table_zp >> 8;
	data_depack[count_depack- 2] = table_prg & 0xff;
	data_depack[count_depack- 1] = table_prg >> 8;

	if(table_eof > 0x10000){
		fprintf(stderr, "Error: temporary space exceeds $ffff\n");
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

	if(filename_output2 != NULL){
		file_output2 = fopen(filename_output2, "wb");
		if(file_output2 == NULL){
			fprintf(stderr, "Error: unable to open file \"%s\": %s\n", filename_output2, strerror(errno));
			exit(1);
		}
		close_output2 = 1;
	}else{
		file_output2 = file_output1;
		filename_output2 = filename_output1;
	}
	
	// load address
	program_data[0] = tempaddr & 0xff;
	program_data[1] = tempaddr >> 8;
	if(2 != fwrite(program_data, 1, 2, file_output1)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output1, strerror(errno));
		exit(1);
	}
	// depacker
	if(count_depack != fwrite(data_depack, 1, count_depack, file_output1)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output1, strerror(errno));
		exit(1);
	}
	// table: prg
	if(count_prg != fwrite(data_prg, 1, count_prg, file_output1)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output1, strerror(errno));
		exit(1);
	}
	// load address
	if(filename_output1 != filename_output2){
		program_data[0] = table_optable & 0xff;
		program_data[1] = table_optable >> 8;
		if(2 != fwrite(program_data, 1, 2, file_output2)){
			fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output1, strerror(errno));
			exit(1);
		}
	}
	// table: optable
	if(optable_len != fwrite(data_optable+optable_start, 1, optable_len, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}
	// table: abs_lo
	if(count_abs != fwrite(data_abs_lo, 1, count_abs, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}
	// table: imm
	if(count_imm != fwrite(data_imm, 1, count_imm, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}
	// table: rel
	if(count_rel != fwrite(data_rel, 1, count_rel, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}
	// table: zp
	if(count_zp != fwrite(data_zp, 1, count_zp, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}
	// table: abs_hi
	if(count_abs != fwrite(data_abs_hi, 1, count_abs, file_output2)){
		fprintf(stderr, "Error: error writing file \"%s\": %s\n", filename_output2, strerror(errno));
		exit(1);
	}

	// close real files
	if(close_output1){
		fclose(file_output1);
	}
	if(close_output2){
		fclose(file_output2);
	}

	/*
	** print stats
	*/

	if(program_addr+program_len == program_addr+(table_eof-(table_prg+optable_len))){
		fprintf(stderr, "Temporary: $%02x-$%02x and $%04x\n", zpaddr, zpaddr+11, program_addr+program_len);
	}else{
		fprintf(stderr, "Temporary: $%02x-$%02x and $%04x-$%04x\n", zpaddr, zpaddr+11, program_addr+program_len, program_addr+(table_eof-(table_prg+optable_len)));
	}
	
#ifndef DISABLE_EXOMIZER
	int exo_input_size;
	if(do_stats){
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
		if(filename_output1 != filename_output2){
			size += exomize(exomizer_cmd, filename_output2);
		}

		if(do_stats){
			fprintf(stderr, "Depacker:  $%04x-$%04x, %d bytes, crunched %d bytes, saved %d bytes\n", tempaddr, table_eof-1, table_eof-tempaddr, size, exo_input_size-size);
		}else{
			fprintf(stderr, "Depacker:  $%04x-$%04x, %d bytes, crunched %d bytes\n", tempaddr, table_eof-1, table_eof-tempaddr, size);
		}
	}else{
#endif
		fprintf(stderr, "Depacker: $%04x-$%04x, %d bytes\n", tempaddr, table_eof-1, table_eof-tempaddr);
#ifndef DISABLE_EXOMIZER
	}
#endif
	
	return 0;
}
