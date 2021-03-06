#!/usr/bin/env python

"""
 Copyright 2004 John Levon <levon@movementarian.org>

 Broken in many different ways, but it hobbles along

 TODO:

  check for file header
  not smart enough to recognise "//" in string literals
  'function (...)'

"""

version = "0.1"

import getopt, string, re, sys

class struct:
	pass

opt = struct()

opt.check_length = False
opt.tags_file = False
opt.count = False

total_errors = 0


ident_char = "[A-Za-z0-9:_]"
identifier = "%s+" % ident_char
cpp_comment = r"//.*\n"
comment_start = r"(//|/\*)"
comment_end = r"\*/"
rop = "[&*]"
argument = "(\s%s\s)+%s*(\s%s\s)*" % (identifier, rop, identifier)
arguments = "(\s%s\s,*\s)"
lfe = "list_for_each[_a-z]*\s\(%s\)" % arguments
control_keyword = "[^%s](if|while|do|for|%s)" % (ident_char, lfe)
camel_ident = "([a-z]+[0-9:_]*[A-Z]|[A-Z]+[0-9:_]*[a-z])"
binop = "(==|!=|\+=|\*=|-=|/=|<<=|>>=|<<|>>|\|\||&&)"

cpp_file = re.compile(".*\.cpp")
spurious_std = re.compile(".*std::")
using = re.compile("^using ")
open_indent = re.compile(r".*\{\s*$")
space_prefix = re.compile(r"^[\t]*[ ][\t ]*[^\t ].*\n")
cpp_io_continue = re.compile(r"\s*<<")
string_literal_continue = re.compile(r"\s*\"")
end_logical_or = re.compile(r".*\|\|\s*$")
end_logical_and = re.compile(r".*\&\&\s*$")
end_comma = re.compile(r".*,\s*$")
end_semicolon = re.compile(r".*;\s*$")
start_logical_or = re.compile(r"\s*\|\|")
start_logical_and = re.compile(r"\s*\&\&")
start_conditional = re.compile(r"\s*\?")
start_conditional2 = re.compile(r"\s*:")
camel_case = re.compile(r"([^\"]+%s|^%s)" % (camel_ident, camel_ident))
cpp_comment_line = re.compile(".*%s" % cpp_comment)
comment_block_start = re.compile("\s*%s" % comment_start)
comment_block_end = re.compile(".*%s" % comment_end)
trailing_comment = re.compile(".*[^ \t]+?\s*%s" % comment_start)
endif = re.compile(".*#endif")
elseif = re.compile(".*#else(if)*")
doxygen_comment = re.compile(r"(\s*///|.*/\*\*<|.*///<)")
namespace_close = re.compile(r".*(%s\snamespace|namespace\s%s)" % (identifier, identifier))
http = re.compile(".*http://.*")
return_statement = re.compile(".*return ")
operator_decl = re.compile(".*operator..*\(")
bin_operator_decl = re.compile(".*operator%s" % binop)
pointer_missing_space = re.compile(".*%s\s+[*]%s" % (identifier, identifier))
reference_missing_space = re.compile(".*%s\s+[&]%s" % (identifier, identifier))
binop_missing_space_before = re.compile(".*[^ \t\n]+%s" % binop)
binop_missing_space_after = re.compile(".*%s[^ ;\t\n]+" % binop)
spurious_one_line_block = re.compile("\s*[ ]|[\t]+catch .*{.*|[\t]+try {.*")
const_inline_func = re.compile(".*const {")

simple_regexps = [
	( re.compile(r".*%s\(" % control_keyword), "missing space after control keyword"),
	( re.compile(".* ,"), "extra space before comma"),
	( re.compile(r".*,[^ \n'\"].*"), "comma not followed by whitespace"),
	( re.compile(".*! "), "extra space after negation"),
	( re.compile(r".*(?<!delete) \["), "extra space before square bracket"),
	( re.compile(r".*\( [^;]"), "extra space after open bracket"),
	( re.compile(r"[^;]* \)"), "extra space before close bracket"),
	( re.compile(r".*\){"), "missing space between bracket and brace"),
	( re.compile(".*const\s%s\s%s" % (identifier, rop)), "'const' modifier in wrong place"),
	( re.compile("\s*else.*{"), "else clause on following line"),
]

camel_ok = [
	"__NR_",
	"poptContext",
	"op_poptGetContext",
	"poptGetContext",
	"poptGetNextOpt",
	"poptOption",
	"poptPrintHelp",
	"poptFreeContext",
	"poptStrerror",
	"poptBadOption",
	"poptGetArg",
	"QListViewItem",
	"setText",
	"setChecked",
	"isChecked",
	"isSelected",
	"firstChild",
	"nextSibling",
	"setEnabled",
	"QTimerEvent",
	"QCloseEvent",
	"QMessageBox::warning",
	"setNum",
	"removePage",
	"QPixmap",
	"QIntValidator",
	"setValidator",
	"setCurrentItem",
	"QString",
	"setRange",
	"minimumSizeHint",
	"QDialog::accept",
	"setUpdatesEnabled",
	"setSelected",
	"setPixmap",
	"startTimer",
	"timerEvent",
	"setExclusive",
	"setSorting",
	"toUInt",
	"sizeHint",
	"setMinimumSize",
	"QCheckBox",
	"QApplication",
	"setMainWidget",
	"QFileDialog::getExistingDirectory",
	"QFileDialog::getOpenFileName",
	"isNull",
	"CPU_PPC64_POWER5p",
]

false_positives = [
"#define KERNEL_OFFSET 0xC0000000",
"	To value;",
"template <typename To, typename From>",
"To op_lexical_cast(From const & src)",
"string const begin_comment(\"/* \");",
"	if ((pmc0 & ~0x1UL) != 0UL) {",
"		if (sscanf(line, \"cycle frequency [Hz] : %lu\", &uval) == 1) {",
"	{ \"//\", \"/\" },",
"	{ \"//usr\", \"/usr\" },",
"	{ \"///\", \"/\" },",
"	{ \"/usr///\", \"/usr\" },",
"	// \"////usr\" must return \"/\"",
"template <typename Input, typename Output>",
"	Input input;",
"	Output output;",
"template <typename Input, typename Output, typename Result>",
"static void check_result(char const * fct_name, Input const & input,",
"		  Output const & output, Result const & result)",
"static void check_result(char const * fct_name, Input const & input1,",
"	Input input2, Output const & output, Result const & result)",
"#define APIC_TDCR			0x3E0",
"#define APIC_TDR_DIV_1			0xB",
"#define APIC_DEFAULT_PHYS_BASE		0xfee00000",
"#define APIC_SPIV			0xF0",
"#define MSR_IA32_APICBASE 0x1B",
"#define GET_APIC_VERSION(x)	((x)&0xFF)",
"#define GET_APIC_MAXLVT(x)	(((x)>>16)&0xFF)",
"#define APIC_INTEGRATED(x)	((x)&0xF0)",
"#define CCCR_RESERVED_BITS 0x38030FFF",
" *		// do something",
"	if (sample.vma & ~0xffffffffLLU)",
"	address = *(unsigned short *)phys_to_virt(0x40E);",
"		smp_scan_config(0xF0000,0x10000)) {",
"	return sum & 0xFF;",
"		      << \" valid range is [\" << OP_MIN_BUF_SIZE << \", \"",
"		      << \" valid range is [\" << OP_MIN_NOTE_TABLE_SIZE << \", \"",
"		       \"ftp://ftp.compaq.com/pub/products/alphaCPUdocs/alpha_arch_ref.pdf\\n\");",
"			    << cfg.count << \" must be in [ \"",
"			     << \" allowed range: [0-100]\" << endl;",
"	catch (Catch const &) {",
"		throw Throw(\"\");",
"template <typename Throw, typename Catch>",
"	{ \"//////\", \"/\" },",
"	{ \"///usr\", \"/\" },",
"	{ \"///usr/dir\", \"///usr\" },",
"	{ \"///usr\", \"usr\" },",
"	{ \"///usr/dir\", \"dir\" },",
"	{ \"///usr//dir\", \"dir\" },",
"	{ \".//.//\" \"file_manip_tests.o\", \"file_manip_tests.o\" },",
"template <typename Throw, typename Catch>",
"		     \"verbose output\", \"all,debug,bfd,level1,sfile,stats,xml\"),",
"	sscanf(arg, \"%lx,%lx\", &kernel_start, &kernel_end);",
"		event->user = copy_ulong(&c, ',');",
"	glob_filter f1(\"foo,*bar\", \"foobar\");",
"		     \"comma separated list\", \"cpu,tid,tgid,unitmask,all\"),",
"		     \"comma separated list\", \"cpu,lib,tid,tgid,unitmask,all\"),",
"		       \"for Software Development (Document 245320-003),\\n\"",
"		       \"for Software Optimization (Document 245473-003),\\n\"",
"		sep += \"library,\";",
"		sep += \"kernel,\";",
"		sep += \"cpu,\";",
"		sep += \"thread,\";",
"	sscanf(arg, \"%llx,%llx\", &vmlinux_image.start, &vmlinux_image.end);",
"	{ \"verbose\", \'V\', POPT_ARG_STRING, &verbose, 0, \"be verbose in log file\", \"all,sfile,arcs,samples,module,misc\", },",
"		\"abcdefghijklmnopqrstuvwxyz0123456789_:=-+%,./\")",
"	c1.set(\"2,3\");",
"	c2.set(\"6,all\");",
"	c2.set(\"all,6\");",
"	path_filter f1(\"foo,*bar\", \"foobar\");",
"	path_filter f6(\" foo,bar\", \"bar \");",
"	string_filter f4(\"ok,ok2,\", \"\");",
"	string_filter f5(\"ok,ok2\", \"no,no2\");",
"		     \"sort by\", \"sample,image,app-name,symbol,debug,vma\"),",
"	right_rule.add_pattern(\".*=[ ]*\\\"(.*)\\\"\", \"\\\\1\");",
"	left_rule.add_pattern(\"[ ]*\\\"(.*)\\\"[ ]*=.*\", \"\\\\1\");",
"	var_value_rule.add_pattern(\".*=[ ]*\\\"(.*)\\\"\", \"\\\\1\");",
"	var_name_rule.add_pattern(\"^\\\\$([_a-zA-Z][_a-zA-Z0-9]*)[ ]*=.*\", \"\\\\1\");",
"	synth_syms.reset(new asymbol * [cur_symcount + start]);",
"	                                + \" [self]\");",
"#elif (defined(__mips__) && (_MIPS_SIM == _MIPS_SIM_ABI32)) /*_MIPSEL */",
"	sscanf(arg, \"%llx,%llx\", &xen_image.start, &xen_image.end);",
"		      << \" valid range is [0 (use default), buffer size/2] \"",
"		      << \" valid range is [\" << OP_MIN_CPU_BUF_SIZE << \", \"",
"		      << \" valid range is [\" << OP_MIN_CPU_BUF_SIZE << \", \"",
"		    ordered_samples.find(node[pos].key);",
"		      << \" valid range is [\" << 0 << \", \"",
"	void fill(size_type size, T const & value) {",
]

def err(file, nr, line, message):
	global total_errors

	for false in false_positives:
		if false == line.splitlines()[0]:
			return

	# http:// looks like "trailing comment"
	if http.match(line):
		return

	if opt.count:
		total_errors = total_errors + 1
		return

	sys.stderr.write("%s:%d: %s\n" % (file, nr, message))
		
def check_camel(file, nr, line):
	match = camel_case.match(line)

	if match == None:
		return

	# dodgy
	for word in camel_ok:
		if line.find(word) != -1:
			return

	err(file, nr, line, "camelCase")

def check_indentation(file, nr, line, prev_line):
	if open_indent.match(prev_line):
		err(file, nr, line, "logical indentation with spaces")
		return

	# hacky as you like

	if cpp_io_continue.match(line) or string_literal_continue.match(line):
		return
	if end_logical_or.match(prev_line) or end_logical_and.match(prev_line):
		return
	if start_logical_or.match(line) or start_logical_and.match(line):
		return
	if end_comma.match(prev_line) or end_semicolon.match(prev_line):
		return
	if start_conditional.match(line) or start_conditional2.match(line):
		return

	open = prev_line.count('(')
	close = prev_line.count(')')

	if open <= close:
		err(file, nr, line, "possible logical indentation with spaces")

def check_trailing_comment(file, nr, line):
	if endif.match(line) or elseif.match(line):
		return
	if doxygen_comment.match(line):
		return
	if namespace_close.match(line):
		return

	err(file, nr, line, "trailing comment")

# false positives are difficult to avoid for inline function defined in class
def check_brace(file, nr, lines):
	if nr >= 3:
		cur = lines[nr-1].rstrip()
		last = lines[nr-3].rstrip()
		if '{' in last and '}' in last:
			return
		if cur == '}' or '{' in cur or not cur.endswith('}'):
			return
		if last == '{' or not last.endswith('{'):
			return
		if spurious_one_line_block.match(last):
			return
		if len(lines[nr-2].expandtabs()) >= 80:
			return
		if operator_decl.match(last):
			return
		if const_inline_func.match(last):
			return
		for false in false_positives:
			if false == last:
				return
		err(file, nr, lines[nr-1], "unnecessary brace around one line block")

in_comment = False

def check_line(file, nr, line, prev_line, lines):

	global in_comment

	if trailing_comment.match(line):
		check_trailing_comment(file, nr, line)

	if cpp_comment_line.match(line):
		return

	if comment_block_start.match(line):
		if comment_block_end.match(line):
			return
		in_comment = True
		return

	if comment_block_end.match(line):
		in_comment = False
		return

	if in_comment:
		return

	if cpp_file.match(file) and spurious_std.match(line) and not using.match(line):
		err(file, nr, line, "warning: spurious std:: qualification")

	check_camel(file, nr, line)
 
	if space_prefix.match(line):
		check_indentation(file, nr, line, prev_line)

	if pointer_missing_space.match(line) and not return_statement.match(line):
		err(file, nr, line, "warning: possible missing space after pointer declaration")

	if reference_missing_space.match(line) and not return_statement.match(line):
		err(file, nr, line, "warning: possible missing space after reference declaration")

	if binop_missing_space_before.match(line) and not bin_operator_decl.match(line):
		err(file, nr, line, "warning: missing space before binary operator")

	if binop_missing_space_after.match(line) and not bin_operator_decl.match(line):
		err(file, nr, line, "warning: missing space after binary operator")

	if opt.check_length and len(line) > 80:
		err(file, nr, line, "warning: line is of length %d" % len(line))

	check_brace(file, nr, lines)

	for regexp, error in simple_regexps:
		if regexp.match(line):
			err(file, nr, line, error)

def check_file(filename):
	file = open(filename, 'r')
	lines = file.readlines()
	# enumerate() is not in Python 2.2
	nr = 1
	in_comment = False
	for line in lines:
		prev_line = ""
		if nr - 1 > 0:
			prev_line = lines[nr - 2]

		check_line(filename, nr, line, prev_line, lines)
		nr = nr + 1

def do_files(files):
	for file in files:
		check_file(file)
	if opt.count:
		print "Possible errors: %d" % total_errors

def usage():
	print "check_style [-v/--version] [-h/--help] [-c/--count] [-t/--from-tags-file <file>] [-l/--check-length] <files>"

def parse_options(argv):
	_options =  ["help"]
	try:
		opts, args = getopt.getopt(argv[1:], "chlt:v", _options)
	except getopt.error:
		usage()
		sys.exit(2)

	print "check_style %s: remember this script is only a guide!" % version

	for o, a in opts:
		if o in ("-h", "--help"):
			usage()
			sys.exit()
		if o in ("-v", "--version"):
			print "check_style version %s" % version
			sys.exit()
		if o in ("-l", "--check-length"):
			opt.check_length = True
		if o in ("-t", "--from-tags-file"):
			opt.tags_file = a
		if o in ("-c", "--count"):
			opt.count = True

	if opt.tags_file:
		files = []
		file = open(opt.tags_file, 'r')
		lines = file.readlines()
		for line in lines:
			if line[0] == '!':
				continue;

			dummy, file, dummy = string.split(line, '\t', 2)

			if string.find(file, "Makefile") != -1:
				continue
			if string.find(file, ".sh") != -1:
				continue
			if string.find(file, ".S") != -1:
				continue
			if string.find(file, "module/") != -1:
				continue

			if file == "config.h":
				continue
			if file == "utils/opcontrol":
				continue
			if file == "gui/ui/oprof_start.base.moc.cpp":
				continue
			if file == "gui/oprof_start.moc.cpp":
				continue
			if file == "gui/ui/oprof_start.base.h":
				continue
			if file == "gui/ui/oprof_start.base.cpp":
				continue
			if file == "daemon/liblegacy/p_module.h":
				continue
			if file == "daemon/opd_perfmon.h":
				continue
			if file == "check_style.py":
				continue
			if file == "configure":
				continue

			files.append(file)

		files = dict([(x,None) for x in files]).keys()
 
		files.sort()
		return files

	if len(args) == 0:
		sys.stderr.write("No files specified.\n");
		usage()
		sys.exit(2)

	return args

def main(argv):
	do_files(parse_options(argv))

if __name__ == "__main__":
	main(sys.argv)
