prepack6502
===========

[prepack6502][] reorganizes 6502 code (and adds a
depacker) in order to make the code better to crunch.

See the examples on how many bytes are saved after using [exomizer][].

The program is only useful on 6502 code, not data (i.e. tables,
graphics). You have to split code and data.

[prepack6502]: http://github.com/alexkazik/prepack6502
[exomizer]: http://hem.bredband.net/magli143/exo/


Requirements
------------

Temporary RAM which can hold the size of the program plus about 330
bytes (will be less in some cases).

12 bytes of temporary RAM in the zeropage.

The one to three bytes behind the extracted program are trashed.

All this memory ranges should not overlap.

Once the depacker is loaded/decrunched into ram you have to
`jsr tempaddr` to depack, afterwards jump into the program.


Changelog
---------

* v1.0 2013-09-28
  - initial public release

* v1.1 2014-09-12
  - bug fix
  - using Magnus Lind's assembler from Exomizer
  - more verbose output
  - option to split the temporary addresses


Compilation
-----------

	make


Usage
-----

	Usage: prepack6502 [options]
	Options:
		-h, --help             show this help
		-i, --input <file>     specifies the input file, defaults to stdin
		-o, --output <file>    specifies the output file, defaults to stdout
		-p, --output2 <file>   if enabled the output is splitted to be better crunched
		-t, --tempaddr <addr>  specify the temporary address, defaults to $1000
		-T, --tempaddr2 <addr> specify the temporary address, defaults to off or behind part one if -p is used
		-z, --zpaddr <addr>    specify the temporary zerpage address, defaults to $f4
		-O, --tableorder <lst> specify the order of tables, defaults to "do.tlirzh"
		-M, --memmap           print memmap
		-e, --exomize[=<pte>]  compress the output(s), optionally specify the /path/to/exomizer
		-s, --stats            print stats, enables -e

The `-t`, `-T` and `-z` options accept numbers in all common formats:
2048, 0x800, $800, 04000. You may not want to use $ since it has to be
escaped.

The `-e` option exomises the generated output(s). If the file ends with
'.prg' then it's replaced with '.exo' otherwise '.exo' is appended. This
does not work with stdout. The options to exomizer are 'mem -c -l none'.

The `-s` option shows a little statistic on the crunched files and the
bytes saved. It will exomize the input (like the `-e` does to the
output) and does not work with stdin.

The `-O` option let you specify the order of the tables. Variations may
be smaller than the default one, depends on the code. The dot specifies
the split point (if two outputs are used). All chars have to be in the
list, just in any order. With `-M` the memory map is shown.


Examples
--------

Packs the file "code.prg" into "prepack.prg" using the temporary
addresses $c000-... and $20-$2b.

	prepack6502 -t 0xc000 -z 0x20 -i code.prg -o prepack.prg

	Temporary: $20-$2b and $bc8a
	Program:   $a400-$bc89, 6282 bytes, crunched 4621 bytes
	Depacker:  $c000-$d9c8, 6601 bytes, crunched 4285 bytes, saved 336 bytes

Splitting the output in two parts and exomize them separately does in
many cases also save some bytes.

	prepack6502 -t 0xc000 -z 0x20 -i code.prg -o prepack1.prg -p prepack2.prg

	Temporary: $20-$2b and $bc8a
	Program:   $a400-$bc89, 6282 bytes, crunched 4621 bytes
	Depacker:  $c000-$cc21, 3106 bytes, $cc22-$d9c8, 3495 bytes, total 6601 bytes, crunched 4273 bytes, saved 348 bytes


License
-------

[Creative Commons Attribution 3.0 Unported License](http://creativecommons.org/licenses/by/3.0/)
