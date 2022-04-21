#include <array>
#include <vector>
#include <string.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "SPL/Utils.hpp"
#include "SPL/Arrays.hpp"
#include "SPL/Integral.hpp"

struct Token{
	union{
		std::array<char, 16> name;
		double value;
		size_t number;
		std::array<size_t, 2> numbers;
	};

	enum class Type : uint32_t{
		End, Value, LocalVariable, Variable, LocalVariableDefinition, VariableDefinition, FunctionDefinition, ArgsRef,
		ParenL, ParenR, BracketL, BracketR, BraceL, BraceR, Comma, Allocation, Call,
		Condition, Not, Or, And, Assign, NotEquals, Lesser, Greater, LesserEq, GreaterEq,
		Add, Subtract, Divide, Multiply, Power, Abs,
		UPlus, UMinus, Factorial,
		Pi, Euler, Gravity,
		Sqrt, Cbrt, Exp, Exp2, Ln, Log, Log2, Log10, Gamma, Erf,
		Mod, Floor, Ceil, Round, Sign, Step, Ramp,
		Sin, Cos, Tan, Sinh, Cosh, Tanh, Asin, Acos, Atan, Atan2, Asinh, Acosh, Atanh,
		Sum, Product, Integral
	} type;


	Token() noexcept {}
	Token(const Type initType) noexcept : type{initType} {}
	Token(const Type initType, const char *s) noexcept : type{initType}{ strncpy(std::begin(this->name), s, 15); }
	Token(const Type initType, double d) noexcept : value{d}, type{initType} {}
	Token(const Type initType, size_t n) noexcept : number{n}, type{initType} {}
};


struct FuncRunInfo{
	std::vector<Token> code;
	size_t varsCount;
};

struct FuncCallInfo{
	std::array<char, 16> name;
	uint32_t argsCount;
};

struct FuncParseInfo{
	std::vector<FuncRunInfo> funcs;
	std::vector<FuncCallInfo> funcNames;
	std::vector<std::array<char, 16>> varNames;
	size_t funcIndex = 0;
};



struct RunInfo{
	size_t nodeIndex = 0;
	size_t baseIndex = 0;
	size_t funcIndex = 0;
	std::vector<double> variables;
};



 
void raiseError(const char *s){ printf("ERROR: %s\n", s); exit(1); }
void raiseError(const char *s0, const char *s1, const char *s2){ printf("ERROR: %s%s%s\n", s0, s1, s2); exit(1); }
bool isNotNameChar(const char x){ return (x<'a' || x>'z') && (x<'A' || x>'Z') && x!='_' && (x<'0' || x>'9'); }

std::vector<Token> makeTokens(const char *input);
std::vector<FuncRunInfo> parseTokens(const Token *input);
double recCalculate(const std::vector<FuncRunInfo> &functions, RunInfo &runInfo);

size_t parse0(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parse1(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parse2(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parse3(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parsePostfix(const Token **tokenI, FuncParseInfo *info, size_t rootNodeIndex);
size_t parsePastComma(const Token **tokenI, FuncParseInfo *info, size_t exprSize);
size_t parse4(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parse5(const Token **tokenI, FuncParseInfo *parseInfo);
size_t parse6(const Token **tokenI, FuncParseInfo *parseInfo);

void parseFunction(const Token **tokenI, FuncParseInfo *parseInfo);




int main(int argc, char **argv){
	
	std::vector<Token> tokens;
	if (argc == 1) return 1;

	if (!strncmp(argv[1], "-f", 2)){

		const bool printFile = argv[1][2]=='p' ? true :
			[x=argv[1][2]](){ if (x){ puts("ERROR: wrong fomat argument"); exit(1); } else return false; }();


		FILE *inputFile = fopen(argv[2], "r");
		if (!inputFile){
			puts("ERROR: this file does not exist");
			return 1;
		}
		fseek(inputFile, 0, SEEK_END);
   	size_t fileSize = ftell(inputFile);

   	char *str = (char *)malloc((fileSize + 1)*sizeof(char));

   	rewind(inputFile);
   	fread(str, sizeof(char), fileSize, inputFile);
		
		if (printFile) printf("%s ", str);
		fclose(inputFile);
		
		str[fileSize] = '\0';
		tokens = makeTokens(str);
	} else tokens = makeTokens(argv[1]);

	std::vector<FuncRunInfo> functions = parseTokens(&*std::begin(tokens));
	RunInfo runInfo;
	runInfo.variables.resize(functions.front().varsCount);

	printf("= %lf\n", recCalculate(functions, runInfo));

	return 0;
}


















std::vector<Token> makeTokens(const char *input){
	std::vector<Token> tokens;
	tokens.reserve(32);
	while (*input){
		if ((*input>='0' && *input<='9') || *input=='.'){
			tokens.push_back(Token{Token::Type::Value, strtod(input, &const_cast<char *&>(input))});
		} else{
			switch (*input){
			case '#': ++input;
				while (*input!='\n' && *input!='\0') ++input;
				break;
			case '+': ++input;
				tokens.push_back(Token{Token::Type::Add});
				break;
			case '-': ++input;
				tokens.push_back(Token{Token::Type::Subtract});
				break;
			case '*': ++input;
				tokens.push_back(Token{Token::Type::Multiply});
				break;
			case '/': ++input;
				tokens.push_back(Token{Token::Type::Divide});
				break;
			case '^': ++input;
				tokens.push_back(Token{Token::Type::Power});
				break;
			case '|': ++input;
				if (input[0] == '|'){
					++input;
					tokens.push_back(Token{Token::Type::Or});
					break;
				}
				tokens.push_back(Token{Token::Type::Abs});
				break;
			case '&': ++input;
				if (input[0] == '|'){
					++input;
					tokens.push_back(Token{Token::Type::And});
					break;
				}
				raiseError("unrecognized token");
				break;
			case '(': ++input;
				tokens.push_back(Token{Token::Type::ParenL});
				break;
			case ')': ++input;
				tokens.push_back(Token{Token::Type::ParenR});
				break;
			case '[': ++input;
				tokens.push_back(Token{Token::Type::BracketL});
				break;
			case ']': ++input;
				tokens.push_back(Token{Token::Type::BracketR});
				break;
			case '{': ++input;
				tokens.push_back(Token{Token::Type::BraceL});
				break;
			case '}': ++input;
				tokens.push_back(Token{Token::Type::BraceR});
				break;
			case '~': ++input;
				tokens.push_back(Token{Token::Type::Not});
				break;
			case '!': ++input;
				if (input[0] == '='){
					++input;
					tokens.push_back(Token{Token::Type::NotEquals});
					break;
				}
				tokens.push_back(Token{Token::Type::Factorial});
				break;
			case '<': ++input;
				if (input[0] == '='){
					++input;
					tokens.push_back(Token{Token::Type::LesserEq});
					break;
				}
				tokens.push_back(Token{Token::Type::Lesser});
				break;
			case '>': ++input;
				if (input[0] == '='){
					++input;
					tokens.push_back(Token{Token::Type::GreaterEq});
					break;
				}
				tokens.push_back(Token{Token::Type::Greater});
				break;
			case '=': ++input;
				tokens.push_back(Token{Token::Type::Assign});
				break;
			case 'a':
				if (input[1]=='c' && input[2]=='o' && input[3]=='s'){
					if(input[4]=='h' && isNotNameChar(input[5])){
						tokens.push_back(Token{Token::Type::Acosh});
						input += 5; break;
					} else if (isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Acos});
						input += 4; break;
					}
				} else if (input[1]=='s' && input[2]=='i' && input[3]=='n'){
					if(input[4]=='h' && isNotNameChar(input[5])){
						tokens.push_back(Token{Token::Type::Asinh});
						input += 5; break;
					} else if (isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Asin});
						input += 4; break;
					}
				} else if (input[1]=='t' && input[2]=='a' && input[3]=='n'){
					if (input[4]=='h' && isNotNameChar(input[5])){
						tokens.push_back(Token{Token::Type::Atanh});
						input += 5; break;
					} else if (input[4]=='2' && isNotNameChar(input[5])){
						tokens.push_back(Token{Token::Type::Atan2});
						input += 5; break;
					} else if (isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Atan});
						input += 4; break;
					}
				} goto Default;
			case 'c':
				if (input[1]=='e' && input[2]=='i' && input[3]=='l' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Ceil});
					input += 4; break;
				} else if (input[1]=='b' && input[2]=='r' && input[3]=='t' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Cbrt});
					input += 4; break;
				} else if (input[1]=='o' && input[2]=='s'){
					if (input[3]=='h' && isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Cosh});
						input += 4; break;
					} else if (isNotNameChar(input[3])){
						tokens.push_back(Token{Token::Type::Cos});
						input += 3; break;
					}
				} goto Default;
			case 'e':
				if (input[1]=='x' && input[2]=='p'){
					if (input[3]=='2' && isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Exp2});
						input += 4; break;
					} else if (isNotNameChar(input[3])){	
						tokens.push_back(Token{Token::Type::Exp});
						input += 3; break;
					}
				} else if (input[1]=='r' && input[2]=='f' && isNotNameChar(input[3])){
					tokens.push_back(Token{Token::Type::Erf});
					input += 3; break;
				} else if (isNotNameChar(input[1])){
					tokens.push_back(Token{Token::Type::Euler});
					input += 1; break;
				} goto Default;
			case 'f':
				if (input[1]=='l' && input[2]=='o' && input[3]=='o' && input[4]=='r' && isNotNameChar(input[5])){
					tokens.push_back(Token{Token::Type::Floor});
					input += 5; break;
				} goto Default;
			case 'g':
				if ((input[1]=='a' && input[2]=='m') && (input[2]=='m' && input[4]=='a' && isNotNameChar(input[5]))){
					tokens.push_back(Token{Token::Type::Gamma});
					input += 5; break;
				} else if (isNotNameChar(input[1])){
					tokens.push_back(Token{Token::Type::Gravity});
					input += 1; break;
				} goto Default;
			case 'i':
				if (input[1]=='n' && input[2]=='t' && input[3]=='['){
					tokens.push_back(Token{Token::Type::Integral});
					input += 4; break;
				} goto Default;
			case 'l':
				if (input[1]=='n' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Ln});
					input += 2; break;
				} else if (input[1]=='o' && input[2]=='g'){
					if(input[3]=='2' && isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Log2});
						input += 4; break;
					} else if (input[3]=='1' && input[4]=='0' && isNotNameChar(input[5])){
						tokens.push_back(Token{Token::Type::Log10});
						input += 5; break;
					} else if (isNotNameChar(input[3])){
						tokens.push_back(Token{Token::Type::Log});
						input += 3; break;
					}
				} goto Default;
				break;
			case 'm':
				if (input[1]=='o' && input[2]=='d'){
					tokens.push_back(Token{Token::Type::Mod});
					input += 3; break;
				} goto Default;
			case 'p':
				if (input[1]=='i' && isNotNameChar(input[2])){
					tokens.push_back(Token{Token::Type::Pi});
					input += 2; break;
				} else if (input[1]=='r' && input[2]=='o' && input[3]=='d' && input[4]=='['){
					tokens.push_back(Token{Token::Type::Product});
					input += 5; break;
				} goto Default;
			case 'r':
				if (input[1]=='o' && input[2]=='u' && input[3]=='n' && input[4]=='d' && isNotNameChar(input[5])){
					tokens.push_back(Token{Token::Type::Round});
					input += 5; break;
				} else if (input[1]=='a' && input[2]=='m' && input[3]=='p' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Ramp});
					input += 3; break;
				} goto Default;
			case 's':
				if (input[1]=='u' && input[2]=='m' && input[3]=='['){
					tokens.push_back(Token{Token::Type::Sum});
					input += 4; break;
				} else if (input[1]=='q' && input[2]=='r' && input[3]=='t' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Sqrt});
					input += 4; break;
				} else if (input[1]=='t' && input[2]=='e' && input[3]=='p' && isNotNameChar(input[4])){
					tokens.push_back(Token{Token::Type::Step});
					input += 4; break;
				} else if (input[1]=='i'){
					if (input[2]=='n'){
						if (input[3]=='h' && isNotNameChar(input[4])){
							tokens.push_back(Token{Token::Type::Sinh});
							input += 4; break;
						} else if (isNotNameChar(input[3])){
							tokens.push_back(Token{Token::Type::Sin});
							input += 3; break;
						}
					} else if (input[2]=='g' && input[3]=='n' && isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Sign});
						input += 4; break;
					}
				} goto Default;
			case 't':
				if(input[1]=='a' && input[2]=='n'){
					if (input[3]=='h' && isNotNameChar(input[4])){
						tokens.push_back(Token{Token::Type::Tanh});
						input += 4; break;
					}
					else if (isNotNameChar(input[3])){
						tokens.push_back(Token{Token::Type::Tan});
						input += 3; break;
					}
				} goto Default;
			case ',': ++input;
				tokens.push_back(Token{Token::Type::Comma});
				break;
			case '\n':
			case '\t':
			case ' ': ++input;
				break;
			default:
			Default:
				if ((*input>='a' && *input<='z') || (*input>='A' && *input<='Z') || *input=='_'){
					tokens.push_back(Token{Token::Type::Variable});
					tokens.back().name[0] = *input;
					++input;
					int i = 1;
					for (; i<15 && !isNotNameChar(*input); ++i, ++input)
						tokens.back().name[i] = *input;
					tokens.back().name[i] = 0;
				} else
					raiseError("unrecognized token");
				break;
			}
		}
	}
	tokens.push_back(Token{Token::Type::End});
	return tokens;
}




size_t parse0(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;
	size_t currNodeIndex = std::size(*nodes);

	switch (iter->type){
	case Token::Type::Value:
		nodes->emplace_back(Token::Type::Value, iter->value);
		++iter;
		return parsePostfix(tokenI, info, currNodeIndex);
	case Token::Type::Variable:{
			const char *varName = std::begin(iter->name);
			for (size_t i=std::size(info->varNames); i; --i)
				if (!strcmp(std::begin(info->varNames[i-1]), varName)){
					++iter;
					if (i > info->funcs.front().varsCount)
						nodes->emplace_back(Token::Type::LocalVariable, i-info->funcs.front().varsCount-1);
					else
						nodes->emplace_back(Token::Type::Variable, i-1);
					return parsePostfix(tokenI, info, std::size(*nodes)-1);
				}
			if (iter[1].type == Token::Type::ParenL){
				for (size_t i=0; i!=std::size(info->funcNames); ++i)
					if (!strcmp(std::begin(info->funcNames[i].name), varName)){
						iter += 2;
						size_t argsCount = info->funcNames[i].argsCount;
						
						nodes->emplace_back(Token::Type::Call);
						nodes->back().numbers[0] = i + 1;
						nodes->back().numbers[1] = argsCount;
						size_t exprSize = 1;

						size_t argCounter = 0;
						if (iter->type != Token::Type::ParenR){
							exprSize += parse3(tokenI, info);
							++argCounter;
							
							while (iter->type != Token::Type::ParenR){
								iter += iter->type == Token::Type::Comma;
								exprSize += parse3(tokenI, info);
								++argCounter;
							}
						} ++iter;
						if (argCounter != argsCount) raiseError("wrong number of arguments to function: ", varName, "");
						
						return parsePostfix(tokenI, info, std::size(*nodes)-exprSize);
					}
			}
			raiseError("variable \'", std::begin(iter->name), "\' was not declared");
		}
	case Token::Type::ParenL: ++iter;{
		parse3(tokenI, info);
		if (iter->type != Token::Type::ParenR)
			raiseError("pharenthesis were opened, but not closed");
		++iter;
		return parsePostfix(tokenI, info, currNodeIndex);
	}
	case Token::Type::Abs: ++iter;{
		nodes->emplace_back(Token::Type::Abs);
		parse3(tokenI, info);
		if (iter->type != Token::Type::Abs)
			raiseError("absolute value symbol was opened, but not closed");
		++iter;
		return parsePostfix(tokenI, info, currNodeIndex);
	}
	case Token::Type::Add: ++iter;
		return parse0(tokenI, info);
	case Token::Type::Subtract: ++iter;
		nodes->emplace_back(Token::Type::UMinus);
		return parse0(tokenI, info);
	case Token::Type::Pi:
	case Token::Type::Euler:
		nodes->emplace_back(Token::Type::Value, iter->type==Token::Type::Pi ? M_PI : M_E);
		++iter;
		return parsePostfix(tokenI, info, currNodeIndex);
	case Token::Type::Acos:
	case Token::Type::Acosh:
	case Token::Type::Asin:
	case Token::Type::Asinh:
	case Token::Type::Atan:
	case Token::Type::Atanh:
	case Token::Type::Cbrt:
	case Token::Type::Ceil:
	case Token::Type::Cos:
	case Token::Type::Cosh:
	case Token::Type::Erf:
	case Token::Type::Exp:
	case Token::Type::Exp2:
	case Token::Type::Floor:
	case Token::Type::Gamma:
	case Token::Type::Ln:
	case Token::Type::Log10:
	case Token::Type::Log2:
	case Token::Type::Round:
	case Token::Type::Sqrt:
	case Token::Type::Step:
	case Token::Type::Sin:
	case Token::Type::Sinh:
	case Token::Type::Sign:
	case Token::Type::Tan:
	case Token::Type::Tanh:
		nodes->emplace_back(iter->type);
		++iter;
		if (iter->type == Token::Type::ParenL){
			++iter;
			parse3(tokenI, info);
			if (iter->type != Token::Type::ParenR)
				raiseError("pharenthesis were opened, but not closed");
			++iter;
			return parsePostfix(tokenI, info, currNodeIndex);
		}
		return 1 + parse0(tokenI, info);
	case Token::Type::Atan2:
	case Token::Type::Log:
	case Token::Type::Mod:
		nodes->emplace_back(iter->type);
		++iter;
		if (iter->type == Token::Type::ParenL){
			++iter;
			parse3(tokenI, info);
			iter += iter->type == Token::Type::Comma;
			parse3(tokenI, info);
			if (iter->type != Token::Type::ParenR)
				raiseError("pharenthesis were opened, but not closed");
			++iter;
			return parsePostfix(tokenI, info, currNodeIndex);
		}
		{
			size_t s = 1 + parse0(tokenI, info);
			return s + parse0(tokenI, info);
		}

	case Token::Type::Sum:
	case Token::Type::Product:
	case Token::Type::Integral:{
		const Token::Type operationType = iter->type;
		++iter;
		constexpr const char *options[3] = {"sum", "product", "integral"};
		if (iter->type != Token::Type::Variable)
			raiseError("expected variable definition inside ", options[(uint32_t)operationType-(uint32_t)Token::Type::Sum], " expression");
		
		const char *indexVarName = std::begin(iter->name);
		if (iter[1].type != Token::Type::Assign)
			raiseError("non initialized variable inside ", options[(uint32_t)operationType-(uint32_t)Token::Type::Sum], "expressin");

		iter += 2 + (iter->type == Token::Type::Comma);

		nodes->emplace_back(operationType, info->funcs[info->funcIndex].varsCount);
		++info->funcs[info->funcIndex].varsCount;
		size_t exprSize = 1 + parse3(tokenI, info);
		exprSize += parse3(tokenI, info);

		if (iter->type != Token::Type::BracketR)
			raiseError("missing closing square bracket inside", options[(uint32_t)operationType-(uint32_t)Token::Type::Sum], "expression");
		++iter;

		info->varNames.emplace_back();
		strcpy(std::begin(info->varNames.back()), indexVarName);
		exprSize += parse3(tokenI, info);
		info->varNames.back()[0] = '\0';
		return exprSize;
	}
	default:
		raiseError("wrong syntax");
	}
}


size_t parse1(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprSize = parse0(tokenI, info);
	
	if (iter->type == Token::Type::Power){
		++iter;
		nodes->emplace_back();
		std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
		new(&*nodes->end()-exprSize-1) Token{Token::Type::Power};
		exprSize += 1 + parse1(tokenI, info);
	}
	return exprSize;
}

size_t parse2(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprSize = parse1(tokenI, info);
	{
		Token::Type tokType = iter->type;
		while (tokType==Token::Type::Multiply || tokType==Token::Type::Divide){
			++iter;
			nodes->emplace_back();
			std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
			new(&*nodes->end()-exprSize-1) Token{tokType};
			exprSize += 1 + parse1(tokenI, info);
			tokType = iter->type;
		}
	} return exprSize;
}

size_t parse3(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprStart = std::size(*nodes); 
	size_t exprSize = parse2(tokenI, info);
	{
		Token::Type tokType = iter->type;
		if (tokType == Token::Type::Abs){
			++iter;
			size_t exprEnd = exprStart + exprSize;
			nodes->emplace_back();
			exprSize += parse6(tokenI, info);
			std::rotate(std::begin(*nodes)+exprStart, std::begin(*nodes), std::end(*nodes));
			return exprSize;
		}
		while (tokType==Token::Type::Add || tokType==Token::Type::Subtract){
			++iter;
			nodes->emplace_back();
			std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
			new(&*nodes->end()-exprSize-1) Token{tokType};
			exprSize += 1 + parse2(tokenI, info);
			tokType = iter->type;
		}
	} return exprSize;
}

size_t parsePostfix(const Token **tokenI, FuncParseInfo *info, size_t rootIndex){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;
	while (iter->type == Token::Type::Factorial){
		++iter;
		nodes->emplace_back();
		std::move_backward(std::begin(*nodes)+rootIndex, std::end(*nodes)-1, std::end(*nodes));
		new(&*std::begin(*nodes)+rootIndex) Token{Token::Type::Factorial};
	}
	return std::size(*nodes) - rootIndex;
}

size_t parsePastComma(const Token **tokenI, FuncParseInfo *info, size_t exprSize){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;
	{
		Token::Type tokType = iter->type;
		while (tokType==Token::Type::Add || tokType==Token::Type::Subtract){
			++iter;
			nodes->emplace_back();
			std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
			new(&*nodes->end()-exprSize-1) Token{tokType};
			exprSize += 1 + parse2(tokenI, info);
			tokType = iter->type;
		}
	} return exprSize;
}

size_t parse6(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprSize = parse5(tokenI, info);
	
	while (iter->type == Token::Type::Or){
		++iter;
		nodes->emplace_back();
		std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
		new(&*nodes->end()-exprSize-1) Token{Token::Type::Or};
		exprSize += 1 + parse5(tokenI, info);
	}
	return exprSize;
}

size_t parse5(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprSize = parse4(tokenI, info);
	
	while (iter->type == Token::Type::And){
		++iter;
		nodes->emplace_back();
		std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
		new(&*nodes->end()-exprSize-1) Token{Token::Type::And};
		exprSize += 1 + parse4(tokenI, info);
	}
	return exprSize;
}

size_t parse4(const Token **tokenI, FuncParseInfo *info){
	const Token *&iter = *tokenI;
	std::vector<Token> *nodes = &info->funcs[info->funcIndex].code;

	size_t exprSize = parse3(tokenI, info);
	{
		Token::Type tokType = iter->type;
		while (tokType>=Token::Type::Assign || tokType<=Token::Type::GreaterEq){
			++iter;
			nodes->emplace_back();
			std::move_backward(nodes->end()-exprSize-1, nodes->end()-1, nodes->end());
			new(&*nodes->end()-exprSize-1) Token{tokType};
			exprSize += 1 + parse3(tokenI, info);
			tokType = iter->type;
		}
	} return exprSize;
}


std::vector<FuncRunInfo> parseTokens(const Token *input){	// VarValues should be array of pointers to variable values stored in nodes
	const Token *iter = input;
	FuncParseInfo funcInfo;
	funcInfo.funcs.emplace_back();

	for (;;){
		if (iter[0].type==Token::Type::Variable && iter[1].type==Token::Type::Assign){
			const char *varName = std::begin(iter->name);
			iter += 2;
			if (iter->type == Token::Type::ParenL){
				++iter;
				funcInfo.funcNames.emplace_back();
				strcpy(std::begin(funcInfo.funcNames.back().name), varName);
				parseFunction(&iter, &funcInfo);
			} else{
				for (auto I=std::begin(funcInfo.varNames); I!=std::end(funcInfo.varNames); ++I)
					if (!strcmp(std::begin(*I), varName)) raiseError("redefinition of variable: ", varName, "");
				
				funcInfo.funcs.front().code.emplace_back(Token::Type::VariableDefinition, funcInfo.funcs.front().varsCount);
				++funcInfo.funcs.front().varsCount;
				funcInfo.varNames.emplace_back();
				strcpy(std::begin(funcInfo.varNames.front()), varName);
				
				parse3(&iter, &funcInfo);
				iter += iter->type == Token::Type::Comma;
			}
		} else{
			if (iter->type == Token::Type::End) raiseError("no returning statement");
			parse3(&iter, &funcInfo);
			if (iter->type != Token::Type::End) raiseError("statement after returning expression in function");
			break;
		}
	}
	return std::move(funcInfo.funcs);
}

void parseFunction(const Token **tokenI, FuncParseInfo *info){	// VarValues should be array of pointers to variable values stored in nodes
	const Token *&iter = *tokenI;


	size_t argsCount = 0;
	while (iter->type != Token::Type::ParenR){
		if (iter->type != Token::Type::Variable)
			raiseError("missing argument's name inside definition of function: ", std::begin(info->funcNames.back().name), "");
		for (auto I=std::begin(info->varNames)+info->funcs.front().varsCount; I!=std::end(info->varNames); ++I)
			if (!strcmp(std::begin(*I), std::begin(iter->name))) raiseError("duplicated function's argument ", std::begin(info->funcNames.back().name), "");
		info->varNames.emplace_back();
		strcpy(std::begin(info->varNames.back()), std::begin(iter->name));
		++argsCount;
		++iter;
	}

	if (iter[1].type != Token::Type::BraceL)
		raiseError("missing opening brace in:", std::begin(info->funcNames.back().name), " function definition");
	iter += 2;

	info->funcNames.back().argsCount = argsCount;
	info->funcIndex = std::size(info->funcs);
	info->funcs.emplace_back();
	info->funcs.back().varsCount = argsCount;
	
	for (;;){
		if (iter[0].type==Token::Type::Variable && iter[1].type==Token::Type::Assign){
			const char *varName = std::begin(iter->name);
			iter += 2;

			if (iter->type == Token::Type::ParenL) raiseError("function defined inside another function");		
			for (auto I=std::begin(info->varNames)+info->funcs.front().varsCount; I!=std::end(info->varNames); ++I)
				if (!strcmp(std::begin(*I), varName)) raiseError("redefinition of variable ", varName, "");
			
			info->funcs.back().code.emplace_back(Token::Type::LocalVariableDefinition, info->funcs.back().varsCount);
			++info->funcs.back().varsCount;
			info->varNames.emplace_back();
			strcpy(std::begin(info->varNames.back()), varName);
			
			parse3(&iter, info);
			iter += iter->type == Token::Type::Comma;
		} else{
			if (iter->type==Token::Type::BraceR) raiseError("no returning statement in function: ", std::begin(info->funcNames.back().name), "");
			parse3(tokenI, info);
			if (iter->type != Token::Type::BraceR) raiseError("statement after returning expression in function:", std::begin(info->funcNames.back().name), "");
			break;
		}
	}
	++iter;
	info->funcIndex = 0;
	info->varNames.resize(info->funcs.front().varsCount);
}



double recCalculate(const std::vector<FuncRunInfo> &functions, RunInfo &runInfo){
Repeat:
	switch (functions[runInfo.funcIndex].code[runInfo.nodeIndex].type){
	case Token::Type::Value: ++runInfo.nodeIndex;
		return functions[runInfo.funcIndex].code[runInfo.nodeIndex-1].value;
	case Token::Type::LocalVariable: ++runInfo.nodeIndex;
		return runInfo.variables[runInfo.baseIndex + functions[runInfo.funcIndex].code[runInfo.nodeIndex-1].number];
	case Token::Type::Variable: ++runInfo.nodeIndex;
		return runInfo.variables[functions[runInfo.funcIndex].code[runInfo.nodeIndex-1].number];
	case Token::Type::LocalVariableDefinition:{
			size_t varIndex = runInfo.nodeIndex;
			++runInfo.nodeIndex;
			runInfo.variables[runInfo.baseIndex + functions[runInfo.funcIndex].code[varIndex].number] = recCalculate(functions, runInfo);
		} goto Repeat;
	case Token::Type::VariableDefinition:{
			size_t varIndex = runInfo.nodeIndex;
			++runInfo.nodeIndex;
			runInfo.variables[functions[runInfo.funcIndex].code[varIndex].number] = recCalculate(functions, runInfo);
		} goto Repeat;
	case Token::Type::Call:{
			size_t funcIndex = functions[runInfo.funcIndex].code[runInfo.nodeIndex].numbers[0];
			size_t argsCount = functions[runInfo.funcIndex].code[runInfo.nodeIndex].numbers[1];
			++runInfo.nodeIndex;

			size_t prevBaseIndex = runInfo.baseIndex;
			size_t newBaseIndex = std::size(runInfo.variables);

			runInfo.variables.resize(newBaseIndex + functions[funcIndex].varsCount);
			for (size_t i=0; i!=argsCount; ++i)
				runInfo.variables[newBaseIndex+i] = recCalculate(functions, runInfo);

			size_t callerIndex = runInfo.funcIndex;
			size_t returnNodeIndex = runInfo.nodeIndex;

			runInfo.funcIndex = funcIndex;
			runInfo.baseIndex = newBaseIndex;
			runInfo.nodeIndex = 0;
			
			double result = recCalculate(functions, runInfo);

			runInfo.funcIndex = callerIndex;
			runInfo.baseIndex = prevBaseIndex;
			runInfo.nodeIndex = returnNodeIndex;

			return result;
		}
	case Token::Type::Add:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			return x + recCalculate(functions, runInfo);
		}
	case Token::Type::Subtract:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			return x - recCalculate(functions, runInfo);
		}
	case Token::Type::Multiply:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			return x * recCalculate(functions, runInfo);
		}
	case Token::Type::Divide:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			double y = recCalculate(functions, runInfo);
			if (y == 0.0) raiseError("division by zero");
			return x / y;
		}
	case Token::Type::Power:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			double y = recCalculate(functions, runInfo);
			if (x<0.0 && fmod(y, 1.0)!=0.0) raiseError("negative base raised to non integral exponent");
			return pow(x, y);
		}
	case Token::Type::Abs: ++runInfo.nodeIndex;
		return abs(recCalculate(functions, runInfo));
	case Token::Type::UMinus: ++runInfo.nodeIndex;
		return -recCalculate(functions, runInfo);
	case Token::Type::Factorial: ++runInfo.nodeIndex;
		return tgamma(recCalculate(functions, runInfo) + 1.0);
	case Token::Type::Acos:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x<-1.0 || x>1.0) raiseError("argument is not in domain of acos");
			return acosh(x);
		}
	case Token::Type::Acosh:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x < 0.0) raiseError("argument is not in domain of acosh");
			return acosh(x);
		}
	case Token::Type::Asin:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x<-1.0 || x>1.0) raiseError("argument is not in domain of asin");
			return asin(x);
		}
	case Token::Type::Asinh:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x < 0.0) raiseError("argument is not in domain of asinh");
			return asinh(x);
		}
	case Token::Type::Atan: ++runInfo.nodeIndex;
		return atan(recCalculate(functions, runInfo));
	case Token::Type::Atanh:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x<-1.0 || x>1.0) raiseError("argument is not in domain of atanh");
			return atanh(x);
		}
	case Token::Type::Atan2:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			return atan2(x, recCalculate(functions, runInfo));
		}
	case Token::Type::Cbrt: ++runInfo.nodeIndex;
		return cbrt(recCalculate(functions, runInfo));
	case Token::Type::Ceil: ++runInfo.nodeIndex;
		return ceil(recCalculate(functions, runInfo));
	case Token::Type::Cos: ++runInfo.nodeIndex;
		return cos(recCalculate(functions, runInfo));
	case Token::Type::Cosh: ++runInfo.nodeIndex;
		return cosh(recCalculate(functions, runInfo));
	case Token::Type::Erf: ++runInfo.nodeIndex;
		return erf(recCalculate(functions, runInfo));
	case Token::Type::Exp: ++runInfo.nodeIndex;
		return exp(recCalculate(functions, runInfo));
	case Token::Type::Exp2: ++runInfo.nodeIndex;
		return exp2(recCalculate(functions, runInfo));
	case Token::Type::Floor: ++runInfo.nodeIndex;
		return floor(recCalculate(functions, runInfo));
	case Token::Type::Gamma: ++runInfo.nodeIndex;
		return tgamma(recCalculate(functions, runInfo));
	case Token::Type::Ln:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x <= 0.0) raiseError("logarithm of non positive argument");
			return log(x);
		}
	case Token::Type::Log2:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x <= 0.0) raiseError("logarithm of non positive argument");
			return log2(x);
		}
	case Token::Type::Log10:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x <= 0.0) raiseError("logarithm of non positive argument");
			return log10(x);
		}
	case Token::Type::Log:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			double y = recCalculate(functions, runInfo);
			if (y <= 0.0 || y == 1.0) raiseError("wrong logarithm's base");
			if (x == 0.0) raiseError("division by zero");
			return log(x) / log(y);
		}
	case Token::Type::Mod: { ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			double y = recCalculate(functions, runInfo);
			if (y == 0.0) raiseError("modolo operation of base zero");
			return fmod(x, y);
		}
	case Token::Type::Product:{
			size_t indexVarIndex = functions[runInfo.funcIndex].code[runInfo.nodeIndex].number;
			++runInfo.nodeIndex;
			double first = recCalculate(functions, runInfo);
			double last = recCalculate(functions, runInfo);
			if (fmod(first, 1.0)!=0.0 || fmod(last, 1.0)!=0.0) raiseError("product has non intergral boundaries");
			
			double product = 1.0;
			size_t funcNodeIndex = runInfo.nodeIndex;
			for (double i=first; i<=last; i+=1.0){
				runInfo.variables[indexVarIndex] = i;
				runInfo.nodeIndex = funcNodeIndex;
				product *= recCalculate(functions, runInfo);
			}
			return product;
		}
	case Token::Type::Round: ++runInfo.nodeIndex;
		return round(recCalculate(functions, runInfo));
	case Token::Type::Sqrt:{ ++runInfo.nodeIndex;
			double x = recCalculate(functions, runInfo);
			if (x <= 0.0) raiseError("square roo of negative value");
			return sqrt(x);
		}
	case Token::Type::Step: ++runInfo.nodeIndex;
		return (double)!signbit(recCalculate(functions, runInfo));
	case Token::Type::Sin: ++runInfo.nodeIndex;
		return sin(recCalculate(functions, runInfo));
	case Token::Type::Sinh: ++runInfo.nodeIndex;
		return sinh(recCalculate(functions, runInfo));
	case Token::Type::Sum:{
			size_t indexVarIndex = functions[runInfo.funcIndex].code[runInfo.nodeIndex].number;
			++runInfo.nodeIndex;
			double first = recCalculate(functions, runInfo);
			double last = recCalculate(functions, runInfo);
			if (fmod(first, 1.0)!=0.0 || fmod(last, 1.0)!=0.0) raiseError("sum has non intergral boundaries");
			
			double sum = 0.0;
			size_t funcNodeIndex = runInfo.nodeIndex;
			for (double i=first; i<=last; i+=1.0){
				runInfo.variables[indexVarIndex] = i;
				runInfo.nodeIndex = funcNodeIndex;
				sum += recCalculate(functions, runInfo);
			}
			return sum;
		}
	case Token::Type::Sign: ++runInfo.nodeIndex;
		return sp::sign(recCalculate(functions, runInfo));
	case Token::Type::Tan: ++runInfo.nodeIndex;
		return tan(recCalculate(functions, runInfo));
	case Token::Type::Tanh: ++runInfo.nodeIndex;
		return tanh(recCalculate(functions, runInfo));
	case Token::Type::Integral:{
			size_t indexVarIndex = functions[runInfo.funcIndex].code[runInfo.nodeIndex].number;
			++runInfo.nodeIndex;
			double first = recCalculate(functions, runInfo);
			double last = recCalculate(functions, runInfo);
			double scale = (last - first) / 2.0;
			double midpoint = (last + first) / 2.0;

			/* CALCULATIONS */{
				size_t funcNodeIndex = runInfo.nodeIndex;

				double offset = sp::QuadratureNodes<double>[0] * scale;
				runInfo.variables[indexVarIndex] = midpoint - offset;
				double funcResult = recCalculate(functions, runInfo);
				runInfo.variables[indexVarIndex] = midpoint + offset;

				runInfo.nodeIndex = funcNodeIndex;
				double sum = (funcResult + recCalculate(functions, runInfo)) * sp::QuadratureWeights<double>[0];
				for (size_t i=1; i<32; ++i){
					offset = sp::QuadratureNodes<double>[i] * scale;
					runInfo.variables[indexVarIndex] = midpoint - offset;
					runInfo.nodeIndex = funcNodeIndex;
					funcResult = recCalculate(functions, runInfo);
					runInfo.variables[indexVarIndex] = midpoint + offset;
					runInfo.nodeIndex = funcNodeIndex;
					sum += (funcResult + recCalculate(functions, runInfo)) * sp::QuadratureWeights<double>[i];
				}
				return sum * scale;
			}
		}
	default:
		break;
	}
	return 0.002137;
}
