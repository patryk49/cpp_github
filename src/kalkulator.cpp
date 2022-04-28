#include <stdint.h>
#include <stdio.h>

#include <complex>
#include <vector>

// wyrazenie matemetematycne powinno byc podane jako argument tego programu

// czego brakuje:
// - raportowania bledow
// - zapamietywania wynikow 10 ostatnich operacji
// - poprawnego odczytywania tekstu




enum class NodeType : uint8_t{
	Add = 0, Subtract,
	Multiply, Divide,
	Power,

	UnaryPlus, UnaryMinus,

	Real, Imaginary,
	Memory,

	OpenPar, ClosePar,
	Null,
};

// tablica zawierajaca prirytety operatorow
int PrecedenceTable[5] = {
	1, 1, // Add, Subtract
	2, 2, // Multiply, Divide
	4,    // Power
};

struct Node{
	NodeType type;
	uint32_t pos;

	union{
		double value;
		size_t index;
	};
};



std::vector<Node> make_tokens(const char *it){
	std::vector<Node> tokens;
	for (;;){
		Node token;
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
			token.type = NodeType::Null;
			tokens.push_back(token);
			return tokens;
		default:
			puts("niewlasciwy znak");
			exit(1); // cos z tym zrobic
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
NodeType parse_expression(std::vector<Node> &nodes) noexcept{
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
			exit(1); // spodziewano sie wartosci liczbowej
		}
		
		// OBSLUGA ZAMYKANIA NAWIASOW
		while (token_it->type == NodeType::ClosePar){
			++token_it;
			prec_offset -= ParOffset;
			if (prec_offset < 0)
				exit(1); // za duzo prawych nawiasow
		}

		// OBSLUGA OPERATOROW BINARNYCH
		token = *token_it;
		++token_it;

		// brak operatora binarnego oznacza koniec wyrazenia 
		if (token.type > NodeType::Power){
			nodes.resize(node_it - std::begin(nodes));
			return token.type;
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





std::complex<double> calculate(const std::vector<Node> &expr){
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
				if (y.real() == 0.0 && y.imag() == 0.0){
					printf("dzielenie przez 0");
					exit(1);
				}
				stack[std::size(stack)-2] = stack.back() / y; 
			} break;
		case NodeType::Power:
			stack[std::size(stack)-2] = std::pow(stack.back(), stack[std::size(stack)-2]); 
			break;
		}
		stack.pop_back();
	}
	return stack.front();
}










void print_nodes(const std::vector<Node> &nodes);

int main(int argc, char **argv){
	if (argc != 2){
		puts("padaj wyrazenie matematyczne jako argument funkcji");
		return 1;
	}
	std::vector<Node> tokens = make_tokens(argv[1]);

	parse_expression(tokens);
//	print_nodes(tokens);

	std::complex<double> result = calculate(tokens);
	printf("= %lg", result.real());
	if (result.imag() != 0.0) printf(" + %lgi", result.imag());
	putchar('\n');

	return 0;
}


/*
void print_nodes(const std::vector<Node> &nodes){
	for (auto it : nodes){
		switch (it.type){
		case NodeType::Add:
			puts("Dodwanie");
			break;
		case NodeType::Subtract:
			puts("Odejmowanie");
			break;
		case NodeType::Multiply:
			puts("Mnozenie");
			break;
		case NodeType::Divide:
			puts("Dzielenie");
			break;
		case NodeType::Power:
			puts("Potegowanie");
			break;
		case NodeType::UnaryMinus:
			puts("Negacja");
			break;
		case NodeType::Real:
			printf("Liczba Rzeczywista : %lf\n", it.value);
			break;
		case NodeType::Imaginary:
			printf("Liczba Urojona : %lf\n", it.value);
			break;
		default:
			puts("blad -> Napraw to!");
			return;
		}
	}
}*/
