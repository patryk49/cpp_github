#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "matrix/Matrix.hpp"




namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////


float nn_ramp(float x) noexcept{ return x<0.f ? 0.f : x; }
float nn_ramp_derivative(float x) noexcept{ return x<0.f ? 0.f : 1.f; } 

float nn_psigmoid(float x) noexcept{ return 1.f - 1.f / (x + 2.f); }
float nn_psigmoid_derivaive(float x) noexcept{ return 1.f / ((x + 2.f) * (x + 2.f)); }




struct NNLayer{
	Matrix<float> weights;
	Vector<float> biases;
	
	Vector<float> sums;
	Vector<float> outputs;

	const Vector<float> *inputs;

	float (*activation_function)(float) = (float (*)(float))nn_ramp;
	float (*activation_derivative)(float) = (float (*)(float))nn_ramp_derivative;

	float learning_rate = 1.f;
};







NNLayer make_layer(size_t input_size, size_t output_size) noexcept{
	NNLayer res;
	
	sp::resize(res.weights, output_size, input_size);
	sp::resize(res.biases, output_size);
	sp::resize(res.sums, output_size);
	sp::resize(res.outputs, output_size);

	return res;
}


template<class R>
void set_random_weights(NNLayer &layer, R &rng, float min = -1.f, float max = 1.f){
	auto frng = [&rng, min, max]() mutable{
		return min + (
			(float)(rng()-sp::min_val(rng)) / (float)(sp::max_val(rng)-sp::min_val(rng))
		) * (max-min);
	};

	layer.weights = sp::generate(sp::rows(layer.weights), sp::cols(layer.weights), frng);
	layer.biases = sp::generate(sp::len(layer.biases), frng);
}


void feed_forward(NNLayer &layer){
	layer.sums = layer.weights * *layer.inputs + layer.biases;
	layer.outputs = apply(layer.sums, layer.activation_function);
}


void back_propagate(NNLayer &layer, Vector<float> &gradients){
	layer.sums = elwise_mul(apply(layer.sums, layer.activation_derivative), gradients);
	
	gradients = tr(layer.weights) * layer.sums;

	layer.biases -= layer.sums * layer.learning_rate;
	layer.weights -= outer_prod(layer.sums, *layer.inputs);
}


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////
