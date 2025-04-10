"""
-------------------------
File: parse.py
Author: Marek Pazúr
Login: xpazurm00
Date: 14.03.2025
Description: This script parses the SOL25 object oriented
						programming language and produces abstract
						 syntax tree in XML form.
-------------------------
"""

import enum
import sys
import argparse
import re
from lark import Lark, LexError, ParseError, Transformer, Visitor, UnexpectedCharacters, Tree, Token
from lark.exceptions import UnexpectedInput, UnexpectedToken
import xml.etree.ElementTree as ET
import xml.dom.minidom as dom

# SOL25 grammar (EBNF)
sol25_grammar = r"""
	program: class_def*

	?class_def: "class" CID ":" CID "{" method* "}"

	?method: selector block

	selector: ID
					| SID+

	block: "[" block_par "|" block_stat* "]" block_end

	block_end:

	block_par: PID*

	block_stat: ID ":=" expr end_stat

	end_stat: "."

	expr: expr_base expr_tail

	expr_tail: expr_sel*

	expr_sel: ID
  				| SID expr_base

	expr_base: INT | STR | ID | CID | block
 						| "(" expr ")"

	CID: /[A-Z][A-Za-z0-9]*/
	ID: /[_a-z][_A-Za-z0-9]*/
	SID: /[_a-z][_A-Za-z0-9]*:/
	PID: /:[_a-z][_A-Za-z0-9]*/
	INT: /[+-]?(0|[1-9][0-9]*)/
	STR: /'([^\x00-\x1f\\']|\\[\\n'])*'/
	COMMENT: /"[^"]*"/ 


	%import common.WS
	%ignore WS
	%ignore COMMENT
"""

# Data collection
keywords = ['class', 'self', 'super', 'nil', 'true', 'false']

class_symtab = {
	"Object": {"parent": None, "methods": ['new', 'from:']},
	"Nil": {"parent": "Object", "methods": []},
	"Integer": {"parent": "Object", "methods": []},
	"String": {"parent": "Object", "methods": ['read']},
	"Block": {"parent": "Object", "methods": []},
	"True":{"parent":"Object", "methods": []},
	"False":{"parent":"Object", "methods": []}
}

class XmlTransformer(Transformer):
	"""
	This class is used to create a transformer object to transform a parse tree
	into an abstract syntax tree in XML elements format.
	"""

	def __init__(self, comment):
		self.comment = comment

	def program(self, items):
		"""Creates program (root element) and sets description, if available."""
		program_node = ET.Element("program", language="SOL25")

		if self.comment:
			program_node.set('description', self.comment)

		# Add class elements (program node children)
		for item in items:
			program_node.append(item)

		return program_node

	def class_def(self, items):
		"""Creates class element with superclass as a 'parent' attribute."""
		class_id, father, *methods = items
		class_node = ET.Element("class", name=class_id, parent=father)

		# Add method elements (class node children)
		for method in methods:
			class_node.append(method)

		return class_node

	def method(self, items):
		"""
		Creates method element with its selector as a parameter.
		
		Sets block subnode arity parameter according to the colon count
		of the method selector.
		"""
		selector, block = items

		# Join the selector identifiers together
		selector = "".join(str(child) for child in selector.children)

		# Count the number of colons in the selector id, to set the block arity
		arity = str(selector.count(':'))

		method_node = ET.Element('method', selector=selector)

		# Add block element subnodes
		method_node.append(block)

		block.set('arity', arity)

		return method_node

	def block(self, items):
		"""
		Creates block element with arity attribute.
		Appends block parameter and statement elements in order.
		"""
		params, *statements = items

		block_node = ET.Element('block', arity=str(len(params)))

		for order, param in enumerate(params, start=1):
			block_node.append(ET.Element('parameter', order=str(order) ,name=param))

		for order, stat in enumerate(statements, start=1):
			if stat:
				stat.set('order', str(order))
				block_node.append(stat)

		return block_node

	def block_par(self, items):
		"""Creates and returns a list containing parameter IDs."""
		block_params = []

		for param in items:
			if isinstance(param, Token) and param.type == 'PID':
				block_params.append(param.value[1:])

		return block_params

	def block_stat(self, items):
		"""
		Creates a statement element with receiver ID subnode.
		Appends expression subnode.
		"""
		var, expr, end = items
		statement_node = ET.Element('assign')

		var_node = ET.Element('var', name=var)

		statement_node.append(var_node)
		statement_node.append(expr)

		return statement_node

	def block_end(self, items):
		"""Marks the end of a block."""
		return

	def expr(self, items):
		"""Creates an expression node, while skipping brackets."""
		expr_base, expr_sel = items

		# Skip expression brackets
		if len(expr_base) > 0 and expr_base.tag == 'expr' and expr_base[0].tag == 'expr':	
			return expr_base[0]

		expression_node = ET.Element('expr')
		
		# Expression has selector
		if expr_sel is not None:
			expression_node.append(expr_sel)

			# Wrap the base in expression element, if primitive type (not processed expr). 
			if expr_base.tag != 'expr':
				expr_wrap = ET.Element('expr')
				expr_wrap.append(expr_base)
				expr_sel.insert(0, expr_wrap)
			else:
				expr_sel.insert(0, expr_base)

		# No selector and expression base is already processed
		elif expr_base.tag == 'expr':
			return expr_base

		# No selector and expression base is primitive type, gets wrapped in expr tag
		else:
			expression_node.append(expr_base)

		return expression_node

	def expr_base(self, items):
		"""
		Processes primitive expression base type and returns 
		primitive type element.
		"""
		base, = items

		if isinstance(base, Token):
			t = base.type
			v = base.value
			if t == 'INT':
				base = 	ET.Element('literal', attrib={'class': 'Integer', 'value': v})
			if t == 'STR':
				base = ET.Element('literal', attrib={'class': 'String', 'value': v.strip("'")})

			if t == 'CID':
				base = ET.Element('literal', attrib={'class': 'class', 'value': v})

			if t == 'ID':
				if v == 'nil':
					base = ET.Element('literal', attrib={'class': 'Nil', 'value': 'nil'})
				elif v == 'true':
					base = ET.Element('literal', attrib={'class': 'True', 'value': 'true'})
				elif v == 'false':
					base = ET.Element('literal', attrib={'class': 'False', 'value': 'false'})
				else:
					base = ET.Element('var', name=v)

		return base

	def expr_tail(self, items):
		"""If selector exists, create send element with argument subnode elements."""
		if not items:
			return None

		# Get selector id
		selector = "".join(sel[0] for sel in items)

		send_node = ET.Element('send', selector=selector)

		# If selector has arguments
		if ':' in selector:
			args = [arg[1] for arg in items]

			# Append arg elements in order
			for order, arg in enumerate(args, start=1):
				arg_node = ET.Element('arg', order=str(order))

				# If argument expr base type is primitive, wrap it in expr element
				if arg.tag != 'expr':
					expr_wrap = ET.Element('expr')
					expr_wrap.append(arg)
					arg_node.append(expr_wrap)
				else:
					arg_node.append(arg)
				
				send_node.append(arg_node)

		return send_node

	def expr_sel(self, items):
		"""Returns expression selector node content."""
		return items

def is_keyword(token):
	"""Checks if the given token is a reserved id (keyword)."""
	if token in keywords:
		print(f"Syntax error (22): {token} is reserved identifier", file=sys.stderr)
		sys.exit(22)

def check_inheritance(cid):
	"""
	Traverses through class parents (super classes), to check
	if circular inheritance problem occurs.
	"""
	parent_list = []
	blame_cid = cid

	while cid is not None:
		parent = class_symtab[cid]["parent"]

		if parent in parent_list:
			print(f"Semantic error (35): Circular inheritance in class '{blame_cid}'", file=sys.stderr)
			sys.exit(35)
		else:
			parent_list.append(parent)
			cid = parent

def is_class_method_defined(cid, method):
	"""
	Checks whether a class method is defined for the given class
	or its superclasses.
	"""
	while cid is not None:
		if method in class_symtab[cid]["methods"]:
			return True
		else:
			cid = class_symtab[cid]["parent"]

	return False

class Semantics(Visitor):
	"""
	This class is used to create an object that visits individual nodes
	of the parse tree and performs the necessary semantic checks on them.
	"""

	def __init__(self):
		self.current_cid = ""
		self.current_method = ""
		self.current_id = ""
		self.id_stack = []
		self.symtab_stack = []
		self.symtab = {}

	def program(self, tree):
		"""
		Travereses through the list of classes and adds their entry
		to the class symtab.
		Checks the redefinition of current class.
		"""
		classes_list = tree.children

		for classes in classes_list:
			cid, parent = classes.children[:2]

			if cid.value in class_symtab:
				print(f"Semantic error (35): Redefinition of class '{cid.value}'", file=sys.stderr)
				sys.exit(35)
			
			class_symtab[cid.value] = {
				"parent": parent.value,
				"methods": [],
				"instance_methods": []
			}

	def class_def(self, tree):
		"""Checks cyclic inheritance and superclass definition."""	
		classes = tree.children
		cid, parent = classes[:2]
		self.current_cid = cid.value

		if parent.value not in class_symtab:
			print(f"Semantic error (32): Undefined class '{parent.value}'", file=sys.stderr)
			sys.exit(32)

		check_inheritance(cid.value)
	
	def method(self, tree):
		"""
		Checks instance method arity and keyword collision.
		Adds instance method to class symtab.
		"""
		selector, block = tree.children
		parameters = block.children[0]

		method = "".join(str(sel.value) for sel in selector.children)

		is_keyword(method)

		self.current_method = method

		class_symtab[self.current_cid]["instance_methods"].append(method)

		if self.current_method.count(':') != len(parameters.children):
			print(f"Semantic error (33): Invalid block arity {self.current_method} - {parameters.children}", file=sys.stderr)
			sys.exit(33)

	def block(self, tree):
		"""
		Pushes actual symtab to stack and creates new one on block entry.
		Pushes current statement ID to stack on block entry.
		"""
		# Stacks actions
		if self.symtab:
			self.symtab_stack.append(self.symtab)

		if self.current_id:
			self.id_stack.append(self.current_id)

		self.symtab = {
			"self": {"type": "var", "def": True},
			"super": {"type": "var", "def": True}
		}

	def block_end(self, tree):
		"""Pops symtab and statement id from their stacks"""
		# Stack action
		if self.symtab_stack:
			self.symtab = self.symtab_stack.pop()

		if self.id_stack:
			self.current_id = self.id_stack.pop()

	def block_par(self, tree):
		"""
		Creates PID entry to block symtab, checks keyword collision
		and PID redefinition.
		"""
		parameters = tree.children

		for param in parameters:
			pid = param.value.lstrip(':')

			if pid in keywords:
				print(f"Syntax error (22): {pid} is reserved identifier", file=sys.stderr)
				sys.exit(22)
			
			if pid in self.symtab:
				# param id already defined
				if self.symtab[pid]["type"] == 'param':
					print(f"Semantic error (35): Parameter redefinition", file=sys.stderr)
					sys.exit(35)	

			self.symtab[pid] = {"type": "param", "def": True}

	def block_stat(self, tree):
		"""
		Checks parameter and variable colission, adds var id entry to symtab.
		Checks keyword collision.
		"""
		var_def = tree.children[0].value

		self.current_id = var_def

		is_keyword(var_def)

		if var_def in self.symtab:
			# variable and param collision
			if self.symtab[var_def]["type"] == 'param':
				print(f"Semantic error (34): Variable parameter collision", file=sys.stderr)
				sys.exit(34)	
		else:
			self.symtab[var_def] = {"type": "var", "def": False}

		return

	def end_stat(self, tree):
		"""Marks currently processed var ID as initialised at the end of a block"""
		if self.current_id and self.current_id in self.symtab:
			self.symtab[self.current_id]["def"] = True

	def expr(self, tree):
		"""Checks class definition and class method definition in expressions"""
		base, tail = tree.children

		if isinstance(base.children[0], Token) and base.children[0].type == 'CID':
			cid = base.children[0]

			if cid not in class_symtab:
				print(f"Semantic error (32): Undefined class '{cid}'", file=sys.stderr)
				sys.exit(32)

			selectors = tail.children
			
			method = "".join(str(sel.children[0].value) for sel in selectors)

			if method:
				if is_class_method_defined(cid, method) == False:
					print(f"Semantic error (32): Undefined class method", file=sys.stderr)
					sys.exit(32)

	def expr_base(self, tree):
		"""
		Checks the definition and initialisation of variable used in expression.
		Checks the definition of a class id used in expression.
		"""
		base = tree.children[0]
		
		if isinstance(base, Tree) and base.data == 'expr':
			return

		if isinstance(base, Token):
			if base.type == 'ID' and base.value not in keywords[1:]:
				if base.value not in self.symtab:
					print(f"Semantic error (32): Undefined variable '{base.value}'", file=sys.stderr)
					sys.exit(32)

				if not self.symtab[base.value]["def"]:
					print(f"Semantic error (32): Uninitialised variable '{base.value}'", file=sys.stderr)
					sys.exit(32)

			if base.type == 'CID' and base.value not in class_symtab:
					print(f"Semantic error (32): Undefined class '{base.value}'", file=sys.stderr)
					sys.exit(32)

	def expr_sel(self, tree):
		"""Checks selector ID collision with a keyword"""
		selector = tree.children[0].value

		is_keyword(selector)

def semantic_analysis(tree):
	"""
	This function, by topdown traversing the parse tree, performs
	the following semantic checks:

	[1] Checking the existence of the Main class and its instance method run
	[2] Checking the definition and initialization of a variable, the definition
	 of a formal parameter, class, and class method
	[3] Checking the arity of the block assigned to the instance method selector
	[4] Collision of a variable with a parameter (redefinition)
	[5] Circular inheritance, parameter redefinition, etc.
	"""
	Semantics().visit_topdown(tree)

	if 'Main' not in class_symtab or 'run' not in class_symtab['Main']['instance_methods']:
		print(f"Semantic error (31): Missing class Main or instance method run", file=sys.stderr)
		sys.exit(31)

def args_parse():
	""" Program input arguments parsing and processing. """

	# Cant have more than one argument
	if len(sys.argv) > 2:
		print(f"error (10): Argument parse", file=sys.stderr)
		sys.exit(10)

	parser = argparse.ArgumentParser("parse.py", exit_on_error=False, add_help=False)

	parser.add_argument("--help", "-h", action="store_true", help="Help message")

	args,unknown = parser.parse_known_args()

	if unknown:
		print(f"error (10): Argument parse", file=sys.stderr)
		sys.exit(10)

	if args.help:
		help = """
		Usage: python3.11 parse.py [OPTION]

		This script parses the SOL25 object-oriented programming language
		and produces abstract syntax tree in XML format.

		Options:
		-h, --help Print this help message and exit.

		If no option is specified, the parse script will read from the standard input
		and parse the SOL25 source code, generating the corresponding AST in XML
		format to the standard output.

		Any other option exits with an error code 10.
		"""
		print(help)
		sys.exit(0)

def main():
	# Parse arguments
	args_parse()

	# Read from standard input
	source_file = sys.stdin.read()

	# Search for the first comment
	match = re.search(r'"([^"]*)"', source_file)
	comment = r""

	if match:
		comment = match.group(1)

	# Lark lexical and syntax analysis 
	parser = Lark(sol25_grammar, start='program', parser="lalr")

	try:
		tokens = list(parser.lex(source_file))
	except LexError as e:
		print(f"Lexical error (21): {e}", file=sys.stderr)
		sys.exit(21)
	except UnexpectedInput as e:
		print(f"Lexical error (21): {e}", file=sys.stderr)
		sys.exit(21)

	# Processing the parse tree
	try:
		# Parse tree
		tree = parser.parse(source_file)

		#print(tree.pretty())

		# Semantic analysis
		semantic_analysis(tree)
		
		# Parse tree to AST in XML format
		ast = XmlTransformer(comment).transform(tree)

		xml_str = ET.tostring(ast, encoding='UTF-8')
		
		print(dom.parseString(xml_str).toprettyxml(encoding='UTF-8').decode('UTF-8'))

	except ParseError as e:
		print(f"Parse error (22): {e}", file=sys.stderr)
		sys.exit(22)

if __name__ == "__main__":
	main()