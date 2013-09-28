prepack6502
===========

[prepack6502][] reorganizes 6502 code (and adds the
depacker) in order to make the code better to crunch.

See the examples on how many bytes are saved after using [exomizer][].

The program is only useful on 6502 code, not data (i.e. tables, graphics).
You have to split code and data.

[prepack6502]: http://github.com/alexkazik/prepack6502
[exomizer]: http://hem.bredband.net/magli143/exo/


Requirements
------------

Temporary RAM which can hold the size of the program plus
330 bytes (will be less in some cases).

12 bytes of temporary RAM in the zeropage.

The one to three bytes behind the extracted program are trashed.

All this memory ranges should not overlap.

At least one opcode must not be used.
Usually not a problem since nobody uses JAM and similar,
but it maybe is a problem when data is mixed into the code.

Once the depacker is loaded/decrunched into ram you
have to `jsr tempaddr` to depack, afterwards jump into
the program.


Compilation
-----------

	gcc -o prepack6502 prepack6502.c


Usage
-----

	Usage: prepack6502 [options]
	Options:
	    -h, --help             show this help
	    -i, --input <file>     specifies the input file, defaults to stdin
	    -o, --output <file>    specifies the output file, defaults to stdout
	    -p, --output2 <file>   if enabled the output is splited to be better crunched
	    -t, --tempaddr <addr>  specify the temporary address, defaults to $1000
	    -z, --zpaddr <addr>    specify the temporary zeropage address, defaults to $f4
	    -e, --exomize[=<pte>]  compress the output(s), optionally specify the /path/to/exomizer
	    -s, --stats            print stats, enables -e

The `-t` and `-z` options accept numbers in all common
formats: 2048, 0x800, $800, 04000.
You may not want to use $ since it has to be escaped.

The `-e` option exomises the generated output(s). If the file ends with
'.prg' then it's replaced with '.exo' otherwise '.exo' is appended. This
does not work with stdout. The options to exomizer are 'mem -c -l none'.

The `-s` option shows a little statistic on the crunched files and
the bytes saved. It will exomize the input (like the `-e` does to the
output) and does not work with stdin.

Examples
--------

Packs the file "code.prg" into "prepack.prg" using the
temporary addresses $c000-... and $20-$2b.

	prepack6502 -t 0xc000 -z 0x20 -i code.prg -o prepack.prg

	Temporary: $20-$2b and $bc8a
	Program:   $a400-$bc89, 6282 bytes, crunched 4621 bytes
	Depacker:  $c000-$d9c7, 6600 bytes, crunched 4282 bytes, saved 339 bytes

Splitting the output in two parts and exomize them separately
does in many cases also save some bytes.

	prepack6502 -t 0xc000 -z 0x20 -i code.prg -o prepack1.prg -p prepack2.prg

	Temporary: $20-$2b and $bc8a
	Program:   $a400-$bc89, 6282 bytes, crunched 4621 bytes
	Depacker:  $c000-$d9c7, 6600 bytes, crunched 4272 bytes, saved 349 bytes


License
-------

[Creative Commons Attribution 3.0 Unported License](http://creativecommons.org/licenses/by/3.0/)
