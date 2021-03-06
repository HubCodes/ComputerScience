#include "stdafx.h"

// Version 2: A Simple, stack-based virtual machine
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <list>
#include <algorithm>
#include <cctype>
#include <map>
#include <stack>

enum class OpKind {
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_PUSH,
	OP_POP
};

enum class NodeKind {
	NODE_OP,
	NODE_NUM
};

struct MachineOp {
	OpKind kind;
	int opData;
};

int summary(const std::list<int>& elems) {
	int result = elems.front();
	std::for_each(++std::begin(elems), std::end(elems), [&result](int i) { result += i; });
	return result;
}

int subtractSummary(const std::list<int>& elems) {
	int result = elems.front();
	std::for_each(++std::begin(elems), std::end(elems), [&result](int i) { result -= i; });
	return result;
}

int multifly(const std::list<int>& elems) {
	int result = elems.front();
	std::for_each(++std::begin(elems), std::end(elems), [&result](int i) { result *= i; });
	return result;
}

int divide(const std::list<int>& elems) {
	int result = elems.front();
	std::for_each(++std::begin(elems), std::end(elems), [&result](int i) { result /= i; });
	return result;
}

int mod(const std::list<int>& elems) {
	int result = elems.front();
	std::for_each(++std::begin(elems), std::end(elems), [&result](int i) { result %= i; });
	return result;
}

class Machine {
public:
	explicit Machine() : machineStack() { }
	Machine(const Machine& other) : machineStack(other.machineStack) { }
	Machine& operator=(const Machine& other) {
		machineStack = other.machineStack;
	}
	~Machine() { }

	int eval(const std::vector<MachineOp>& code) {
		std::list<int> operands;
		int popCount = 0;
		auto&& handleStack = [this, &operands](int popCount, int (*operate)(const std::list<int>&)) {
			for (int i = 0; i < popCount; i++) {
				operands.push_front(machineStack.top());
				machineStack.pop();
			}
			machineStack.push(operate(operands));
		};
		std::for_each(std::begin(code), std::end(code), [this, &popCount, &operands, &handleStack](const MachineOp& op) {
			switch (op.kind) {
			case OpKind::OP_PUSH:
				machineStack.push(op.opData);
				break;
			case OpKind::OP_ADD: 
				handleStack(op.opData, summary);
				break;
			case OpKind::OP_SUB:
				handleStack(op.opData, subtractSummary);
				break;
			case OpKind::OP_MUL:
				handleStack(op.opData, multifly);
				break;
			case OpKind::OP_DIV:
				handleStack(op.opData, divide);
				break;
			case OpKind::OP_MOD:
				handleStack(op.opData, mod);
				break;
			}
			if (!operands.empty())
				operands.clear();
		});
		return machineStack.top();
	}

private:
	std::stack<int> machineStack;
};

enum class TokenKind {
	KIND_OP,
	KIND_NUM,
	KIND_OPEN,
	KIND_CLOSE
};

struct Node {
	NodeKind kind;
	Node* parent;
	int numOrOp;
	std::vector<Node*> childs;
};

using token = std::tuple<TokenKind, int>;

bool is_op(char c) {
	return
		c == '+' ||
		c == '-' ||
		c == '*' ||
		c == '/' ||
		c == '%';
}

std::vector<token> lex(const std::string& stream) {
	std::vector<token> result;
	std::for_each(std::begin(stream), std::end(stream), [&result](char c) {
		if (c == ' ')
			return;
		else if (std::isdigit(c))
			result.emplace_back(TokenKind::KIND_NUM, c - '0');
		else if (is_op(c))
			result.emplace_back(TokenKind::KIND_OP, c);
		else if (c == '(')
			result.emplace_back(TokenKind::KIND_OPEN, c);
		else
			result.emplace_back(TokenKind::KIND_CLOSE, c);
	});
	return result;
}

Node* parse(std::vector<token>& stream, int& index) {
	char op;
	std::vector<int> operands;
	if (std::get<0>(stream[index]) == TokenKind::KIND_NUM) {
		int result = std::get<1>(stream[index]);
		index++;
		return new Node{
			NodeKind::NODE_NUM,
			nullptr,
			result
		};
	}
	else if (std::get<0>(stream[index]) == TokenKind::KIND_OP) {
		Node* op = new Node{
			NodeKind::NODE_OP,
			nullptr,
			0
		};
		op->numOrOp = std::get<1>(stream[index]);
		index++;
		while (std::get<0>(stream[index]) != TokenKind::KIND_CLOSE) {
			Node* child = parse(stream, index);
			child->parent = op;
			op->childs.push_back(child);
		}
		index++;
		return op;
	}
	else {
		index++;
		return parse(stream, index);
	}
}

std::vector<std::string> machineCode;

std::vector<MachineOp> compile() {
	std::vector<MachineOp> result;
	std::for_each(std::begin(machineCode), std::end(machineCode), [&result](const std::string& code) {
		std::string opcode = code.substr(0, 3);
		MachineOp instr;
		auto&& arg = std::find_if(std::begin(code), std::end(code), [](char c) {
			return !!std::isdigit(c);
		});
		instr.opData = std::stoi(std::string(arg, std::end(code)));
		if (opcode == "ADD") {
			instr.kind = OpKind::OP_ADD;
		}
		else if (opcode == "SUB") {
			instr.kind = OpKind::OP_SUB;
		}
		else if (opcode == "MUL") {
			instr.kind = OpKind::OP_MUL;
		}
		else if (opcode == "DIV") {
			instr.kind = OpKind::OP_DIV;
		}
		else if (opcode == "MOD") {
			instr.kind = OpKind::OP_MOD;
		}
		else {
			instr.kind = OpKind::OP_PUSH;
		}
		result.push_back(instr);
	});
	return result;
}

void gen(Node* root) {
	static std::map<int, std::string> opmap = {
		{ '+', "ADD" },
		{ '-', "SUB" },
		{ '*', "MUL" },
		{ '/', "DIV" },
		{ '%', "MOD" }
	};
	if (root->kind == NodeKind::NODE_OP) {
		int args = 0;
		if (!root->childs.empty()) {
			std::for_each(std::begin(root->childs), std::end(root->childs), [&args](Node* op) {
				gen(op);
				args++;
			});
		}
		machineCode.push_back(opmap[root->numOrOp] + ' ' + std::to_string(args));
	}
	else {	// maybe numbers...
		machineCode.push_back("PUSH " + std::to_string(root->numOrOp));
	}
}

void deleteNode(Node* root) {
	if (!root->childs.empty()) {
		std::for_each(std::begin(root->childs), std::end(root->childs), [](Node* child) {
			deleteNode(child);
		});
	}
	delete root;
}

int main(void) {
	int index = 0;
	std::string code;
	Machine machine;
	std::cout << "Simple interpreter Version 1.0 release 1\n"
		"Copyright (C) 2018 HubCodes, All rights reserved.\n";

	for (;;) {
		std::cout << "repl> ";
		std::getline(std::cin, code);
		if (code.empty()) 
			continue;
		auto&& lexed = lex(code);
		auto&& parsed = parse(lexed, index);
		gen(parsed);
		auto&& compiled = compile();
		std::cout << machine.eval(compiled) << '\n';
		index = 0;
		deleteNode(parsed);
		machineCode.clear();
	}
	return 0;
}

/*

// Version 1: Just used call stack, and simple lexer, read-eval-print loop
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <cctype>
#include <map>

enum kinds {
	KIND_OP,
	KIND_NUM,
	KIND_OPEN,
	KIND_CLOSE
};

using token = std::tuple<int, int>;

bool is_op(char c) {
	return
		c == '+' ||
		c == '-' ||
		c == '*' ||
		c == '/' ||
		c == '%';
}

int sum(const std::vector<int>& elems) {
	int result = elems[0];
	std::for_each(std::begin(elems) + 1, std::end(elems), [&result](int i) { result += i; });
	return result;
}

int minus(const std::vector<int>& elems) {
	int result = elems[0];
	std::for_each(std::begin(elems) + 1, std::end(elems), [&result](int i) { result -= i; });
	return result;
}

int multifly(const std::vector<int>& elems) {
	int result = elems[0];
	std::for_each(std::begin(elems) + 1, std::end(elems), [&result](int i) { result *= i; });
	return result;
}

int divide(const std::vector<int>& elems) {
	int result = elems[0];
	std::for_each(std::begin(elems) + 1, std::end(elems), [&result](int i) { result /= i; });
	return result;
}

int mod(const std::vector<int>& elems) {
	int result = elems[0];
	std::for_each(std::begin(elems) + 1, std::end(elems), [&result](int i) { result %= i; });
	return result;
}

std::map<int, int(*) (const std::vector<int>&)> op_impl = {
	{ '+', sum },
	{ '-', minus },
	{ '*', multifly },
	{ '/', divide },
	{ '%', mod }
};

std::vector<token> lex(const std::string& stream) {
	std::vector<token> result;
	std::for_each(std::begin(stream), std::end(stream), [&result](char c) {
		if (c == ' ')
			return;
		else if (std::isdigit(c))
			result.emplace_back(KIND_NUM, c - '0');
		else if (is_op(c))
			result.emplace_back(KIND_OP, c);
		else if (c == '(')
			result.emplace_back(KIND_OPEN, c);
		else
			result.emplace_back(KIND_CLOSE, c);
	});
	return result;
}

int eval(std::vector<token>& stream, int& index) {
	char op;
	std::vector<int> operands;
	if (std::get<0>(stream[index]) == KIND_NUM) {
		auto&& result = std::get<1>(stream[index]);
		index++;
		return result;
	}
	else if (std::get<0>(stream[index]) == KIND_OP) {
		op = std::get<1>(stream[index]);
		index++;
		while (std::get<0>(stream[index]) != KIND_CLOSE)
			operands.push_back(eval(stream, index));
		index++;
		return op_impl[op](operands);
	}
	else {
		index++;
		return eval(stream, index);
	}
}

int main(void) {
	int index = 0;
	std::string code;
	for (;;) {
		std::cout << "repl> ";
		std::getline(std::cin, code);
		auto&& lexed = lex(code);
		std::cout << eval(lexed, index) << '\n';
		index = 0;
	}
	return 0;
}
*/