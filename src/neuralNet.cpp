#define SP_MATRIX_DEBUG
#include "NeuralNet.hpp"


void print_matrix(const sp::Matrix<float> &A){
	for (uint32_t i=0, j; i<sp::rows(A); ++i){
		for (j=0; j<sp::cols(A); ++j)
			printf("%8.2lf", A(i, j));
		putchar('\n');
	}
}


int main(){
	sp::Vector<float> inputs;

	sp::resize(inputs, 2);

	sp::NNLayer input_layer = sp::make_layer(2, 10);
	input_layer.activation_function = sp::nn_ramp;
	input_layer.activation_derivative = sp::nn_ramp_derivative;
	input_layer.inputs = &inputs;

	sp::NNLayer hidden_layer = sp::make_layer(10, 10);
	hidden_layer.activation_function = sp::nn_ramp;
	hidden_layer.activation_derivative = sp::nn_ramp_derivative;
	hidden_layer.inputs = &input_layer.outputs;

	sp::NNLayer output_layer = sp::make_layer(10, 1);
	output_layer.activation_function = tanh;
	output_layer.activation_derivative = [](float x){ float y=tanh(x); return 1.f - y*y; };
	output_layer.inputs = &hidden_layer.outputs;


	float learning_rate = 0.1f;
	sp::Rand32 rng(clock());
	
	set_random_weights(input_layer, rng, 0.f, 1.f);
	set_random_weights(hidden_layer, rng, 0.f, 1.f);
	set_random_weights(output_layer, rng, 0.f, 1.f);

	input_layer.learning_rate = learning_rate;
	hidden_layer.learning_rate = learning_rate;
	output_layer.learning_rate = learning_rate;


	sp::Vector<float> gradients;


	for (uint32_t i=0; i<1000; ++i){
		int a = rng() & 1;
		int b = rng() & 1;
		float correct_output = (float)(a & b);

		inputs[0] = a;
		inputs[1] = b;

		sp::feed_forward(input_layer);
		sp::feed_forward(hidden_layer);
		sp::feed_forward(output_layer);

		resize(gradients, sp::len(output_layer.outputs));
		
		float predicted_output = output_layer.outputs[0];
		gradients[0] = predicted_output - correct_output;

	//	printf("pred: %10.4f\tcorr: %10.4f\n", predicted_output, correct_output);

		sp::back_propagate(output_layer, gradients);
		sp::back_propagate(hidden_layer, gradients);
		sp::back_propagate(input_layer, gradients);
	}

	puts("Learning is finished.\n");

	for (;;){
		if (scanf("%f %f", &inputs[0], &inputs[1]) != 2) return 0;

		feed_forward(input_layer);
		feed_forward(hidden_layer);
		feed_forward(output_layer);

		printf("= %10.4f\n\n", output_layer.outputs[0]);
	}



	return 0;
}
