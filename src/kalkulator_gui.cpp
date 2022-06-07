#include <stdint.h>
#include <stdio.h>

#include <complex>
#include <vector>

#include <raylib.h>


enum class NodeType : uint8_t{
	Add = 0, Subtract,
	Multiply, Divide,
	Power,

	UnaryPlus, UnaryMinus,

	Real, Imaginary,
	Memory,

	OpenPar, ClosePar,
	End,
};

// tablica zawierajaca prirytety operatorow
int PrecedenceTable[5] = {
	1, 1, // Add, Subtract
	2, 2, // Multiply, Divide
	4,    // Power
};

struct Node{
	NodeType type;
	uint32_t position;

	union{
		double value;
		size_t index;
	};
};


enum class ErrorType : uint32_t{
	None = 0,
	UnrecognizedToken,
	TooHighMemoryIndex,
	TooManyClosingParenthesis,
	MissingValue,
	WrongSyntax,
	MissingClosingParenthesis,
	DivisionByZero,
};

struct ErrorInfo{
	ErrorType type;	
	uint32_t position;
};




struct TokensResult{
	std::vector<Node> tokens;
	ErrorInfo error;
};

auto make_tokens(const char *str){
	const char *it = str;
	std::vector<Node> tokens;
	for (;;){
		Node token;
		token.position = it - str;
		switch (*it){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token.type = NodeType::Real;
			token.value = strtod(it, (char **)&it);
			if (*it == 'i')
				token.type = NodeType::Imaginary;
			else
				--it;
			break;
		case 'i':
			token.type = NodeType::Imaginary;
			token.value = 1.0;
			break;
		case 'm':{
			++it;
			token.type = NodeType::Memory;
			token.index = *it - '0';
			if (token.index >= 10)
				return TokensResult{{}, {ErrorType::TooHighMemoryIndex, token.position}};
			break;
		}
		case '+':
			token.type = NodeType::Add;
			break;
		case '-':
			token.type = NodeType::Subtract;
			break;
		case '*':
			token.type = NodeType::Multiply;
			break;
		case '/':
			token.type = NodeType::Divide;
			break;
		case '^':
			token.type = NodeType::Power;
			break;
		case '(':
			token.type = NodeType::OpenPar;
			break;
		case ')':
			token.type = NodeType::ClosePar;
			break;
		case ' ':
		case '\t':
			++it;
			continue;
		case '\n':
		case '\0':
			token.type = NodeType::End;
			tokens.push_back(token);
			return TokensResult{std::move(tokens), {ErrorType::None}};
		default:
		Default:
			return TokensResult{{}, {ErrorType::UnrecognizedToken, token.position}};
		}
		tokens.push_back(token);
		++it;
	}
}



struct PrecedenceInfo{
	int value;                                 // prirytet operacji
	typename std::vector<Node>::iterator node; // iterator wskazujacy na opearcje w tablicy 'nodes'
};

// funkcja 'parse expression' usowa nieptrzebne wezly oraz zmienia ich
// kolejnosc tak aby tablica stala sie tablicowa reprezentacja drzewa
ErrorInfo parse_expression(std::vector<Node> &nodes) noexcept{
	std::vector<PrecedenceInfo> precs;
	precs.push_back(PrecedenceInfo{0, {}});

	auto token_it = std::begin(nodes);
	auto node_it = token_it;
	
	static constexpr int ParOffset = 8;
	int prec_offset = 0;

	// OBSLUGA  OPEARTOROW UNARNYCH I WARTOSCI LICZBOWYCH
	for (;;){
		auto first_node = node_it;

		Node token = *token_it;
		++token_it;

		switch (token.type){
		case NodeType::Add:
			node_it->type = NodeType::UnaryPlus;
			continue;
		case NodeType::Subtract:
			token.type = NodeType::UnaryMinus;
			*node_it = token;
			++node_it;
			{
				int precedence = prec_offset + 5;
				if (precs.back().value < precedence)
					precs.push_back(PrecedenceInfo{precedence, first_node});
			}
			continue;
		case NodeType::OpenPar:
			prec_offset += ParOffset;
			continue;
		case NodeType::Real:
		case NodeType::Imaginary:
		case NodeType::Memory:
			*node_it = token;
			++node_it;
			break;
		default:
			return ErrorInfo{ErrorType::MissingValue, token.position};
		}
		
		// OBSLUGA ZAMYKANIA NAWIASOW
		while (token_it->type == NodeType::ClosePar){
			prec_offset -= ParOffset;
			if (prec_offset < 0)
				return ErrorInfo{ErrorType::TooManyClosingParenthesis, token_it->position};
			++token_it;
		}

		// OBSLUGA OPERATOROW BINARNYCH
		token = *token_it;
		++token_it;

		// brak operatora binarnego oznacza koniec wyrazenia 
		if (token.type > NodeType::Power){
			nodes.resize(node_it - std::begin(nodes));
			if (token.type != NodeType::End)
				return ErrorInfo{ErrorType::WrongSyntax, token.position};
			if (prec_offset != 0)
				return ErrorInfo{ErrorType::MissingClosingParenthesis, token.position};
			return {ErrorType::None};
		}
		int precedence = PrecedenceTable[(size_t)token.type] + prec_offset;

		if (precedence <= precs.back().value){
			while (precedence <= precs[std::size(precs)-2].value) precs.pop_back();
			precs.back().value = precedence;
		} else{
			// w celu sprawienia by operator potegownia byl 'prawostronnie laczny'
			// jego prirytet jest w tym miejscu zmniejszany o 1
			precs.push_back(PrecedenceInfo{precedence-(token.type==NodeType::Power), first_node});
		}

		std::move_backward(precs.back().node, node_it, node_it+1);
		++node_it;
		*precs.back().node = token;
	}
}



struct CalculationResult{
	std::complex<double> value;
	ErrorInfo error;
};


// PAMIEC DZIESIECIU OSTATNICH OPERACJI
std::complex<double> calc_memory[10];

CalculationResult calculate(const std::vector<Node> &expr){
	std::vector<std::complex<double>> stack;

	auto it = std::end(expr);
	while (it != std::begin(expr)){
		--it;
		switch (it->type){
		case NodeType::Real:
			stack.push_back(std::complex<double>{it->value, 0.0});
			continue;
		case NodeType::Imaginary:
			stack.push_back(std::complex<double>{0.0, it->value});
			continue;
		case NodeType::Memory:
			stack.push_back(calc_memory[it->index]);
			continue;
		case NodeType::UnaryMinus:
			stack.back() = -stack.back();
			continue;
		case NodeType::Add:
			stack[std::size(stack)-2] += stack.back();
			break;
		case NodeType::Subtract:
			stack[std::size(stack)-2] = stack.back() - stack[std::size(stack)-2];
			break;
		case NodeType::Multiply:
			stack[std::size(stack)-2] = stack.back() * stack[std::size(stack)-2]; 
			break;
		case NodeType::Divide:{
				std::complex<double> y = stack[std::size(stack)-2];
				if (y.real() == 0.0 && y.imag() == 0.0)
					return CalculationResult{{}, {ErrorType::DivisionByZero, it->position}};
				stack[std::size(stack)-2] = stack.back() / y; 
			} break;
		case NodeType::Power:
			stack[std::size(stack)-2] = std::pow(stack.back(), stack[std::size(stack)-2]); 
			break;
		}
		stack.pop_back();
	}
	return CalculationResult{stack.front(), {ErrorType::None}};
}






void print_error(const char *text, uint32_t position){
	for (size_t i=0; i!=position; ++i) putchar(' ');
	printf("^= %s\n", text);
};


void print_nodes(const std::vector<Node> &nodes);

int main(){
	char str[128];
	std::fill(std::begin(calc_memory), std::end(calc_memory), std::complex<double>{});

	InitWindow(500, 300, "kalkulator");

	Rectangle   num_7{ 24,  24, 47, 47};
	Rectangle   num_8{ 79,  24, 47, 47};
	Rectangle   num_9{134,  24, 47, 47};
	Rectangle   par_l{189,  24, 47, 47};
	Rectangle  op_div{244,  24, 47, 47};
	Rectangle   num_4{ 24,  79, 47, 47};
	Rectangle   num_5{ 79,  79, 47, 47};
	Rectangle   num_6{134,  79, 47, 47};
	Rectangle   par_r{189,  79, 47, 47};
	Rectangle  op_mul{244,  79, 47, 47};
	Rectangle   num_1{ 24, 134, 47, 47};
	Rectangle   num_2{ 79, 134, 47, 47};
	Rectangle   num_3{134, 134, 47, 47};
	Rectangle  op_pow{189, 134, 47, 47};
	Rectangle  op_sub{244, 134, 47, 47};
	Rectangle   num_0{ 24, 189, 47, 47};
	Rectangle dec_dot{ 79, 189, 47, 47};
	Rectangle  imag_i{134, 189, 47, 47};
	Rectangle get_res{189, 189, 47, 47};
	Rectangle  op_add{244, 189, 47, 47};





	while (!WindowShouldClose()){
		BeginDrawing();
			ClearBackground(WHITE);
			
			DrawRectangleRec(num_0, RAYWHITE);
			DrawRectangleRec(num_1, RAYWHITE);
			DrawRectangleRec(num_2, RAYWHITE);
			DrawRectangleRec(num_3, RAYWHITE);
			DrawRectangleRec(num_4, RAYWHITE);
			DrawRectangleRec(num_5, RAYWHITE);
			DrawRectangleRec(num_6, RAYWHITE);
			DrawRectangleRec(num_7, RAYWHITE);
			DrawRectangleRec(num_8, RAYWHITE);
			DrawRectangleRec(num_9, RAYWHITE);
			DrawRectangleRec(op_add, RAYWHITE);
			DrawRectangleRec(op_sub, RAYWHITE);
			DrawRectangleRec(op_mul, RAYWHITE);
			DrawRectangleRec(op_div, RAYWHITE);
			DrawRectangleRec(op_pow, RAYWHITE);
			DrawRectangleRec(get_res, RAYWHITE);
			DrawRectangleRec(dec_dot, RAYWHITE);
			DrawRectangleRec(imag_i, RAYWHITE);
			DrawRectangleRec(par_l, RAYWHITE);
			DrawRectangleRec(par_r, RAYWHITE);








		EndDrawing();



/*
		if (!fgets(str, sizeof(str), stdin)) return 0;
	
		auto [tokens, token_error] = make_tokens(str);
		switch (token_error.type){
		case ErrorType::UnrecognizedToken:
			print_error("nieorozpoznany znak", token_error.position);
			continue;
		case ErrorType::TooHighMemoryIndex:
			print_error("po znaku 'm' powinna wystapis liczba od 0 do 9", token_error.position);
			continue;
		default: break;
		}
		
	
		ErrorInfo expr_error = parse_expression(tokens);
		switch (expr_error.type){
		case ErrorType::MissingClosingParenthesis:
			print_error("brak zamykajacych nawiasow", expr_error.position);
			continue;
		case ErrorType::MissingValue:
			print_error("brak wartosci liczbowej", expr_error.position);
			continue;
		case ErrorType::TooManyClosingParenthesis:
			print_error("zbyt duzo zamykajacych nawiasow", expr_error.position);
			continue;
		case ErrorType::WrongSyntax:
			print_error("niewlasciwa skladnia", expr_error.position);
			continue;
		default: break;
		}	
	//	print_nodes(tokens);
	
		auto [result, calc_error] = calculate(tokens);
		if (calc_error.type == ErrorType::DivisionByZero){
			print_error("dzielenie przez zero", calc_error.position);
			continue;
		}

		printf("= %lg", result.real());
		if (result.imag() != 0.0) printf(" + %lgi", result.imag());
		putchar('\n');

		std::move(std::begin(calc_memory), std::end(calc_memory)-1, std::begin(calc_memory)+1);
		calc_memory[0] = result; */
	}

	CloseWindow();
	return 0;
}
