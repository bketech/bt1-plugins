/*
    This file contains a part of a testing suite for foo-plugins
    Copyright (C) 2006 Sampo Savolainen <v2@iki.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define FOO_TESTER

//#define TEST_TRANSIENT_ARCHITECT
#define TEST_LIMITER

#ifdef TEST_LIMITER
#include "foo_limiter.c"
#endif

#ifdef TEST_TRANSIENT_ARCHITECT
#include "foo_transients.c"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

void write_file_constant(char *file, float value, int size)
{
	int i;
	FILE *fd = fopen(file, "w+");
	for (i = 0; i < size; i++) {
		fprintf(fd, "%f\n", value);
	}
	fclose(fd);
}

void write_file(char *file, float *buf, int size)
{
	int i;
	FILE *fd = fopen(file, "w+");
	for (i = 0; i < size; i++) {
		fprintf(fd, "%f\n", buf[i]);
	}
	fclose(fd);
}

void create_ABCD(float *data, int buffer_size)
{
	int m = buffer_size / 4;
	int start_offset = m / 10;

	data[0 * m + start_offset] = 1.2f;
	data[0 * m + start_offset + 33 ] = 2.0f;

	data[1 * m + start_offset] = 1.2f;
	data[1 * m + start_offset + 33 ] = 1.3f;

	data[2 * m + start_offset] = 1.2f;
	data[2 * m + start_offset + 33 ] = 1.3f;

	data[3 * m + start_offset] = 1.2f;
	data[3 * m + start_offset + 33 ] = 0.9f;
}

void create_random(float *data, int buffer_size, float scale, float offset)
{
	int i;
	
	for (i = 0; i < buffer_size; i++) {
		data[i] = (float)rand()/(float)RAND_MAX * scale + offset;
	}
}

#ifdef TEST_LIMITER
int test_limiter(int argc, char **argv)
{
	float input_gain = 0.0f;
	float max_peak = 0.0f;
	float release_time = 0.05f; // seconds 
	float release_scale = 0.75f;

	float attenuation;
	float latency;


	int buffer_size = 1024 * 4;

	float *input_left   = (float*) malloc(buffer_size * sizeof(float));
	float *input_right  = (float*) malloc(buffer_size * sizeof(float));

	float *output_left  = (float*) malloc(buffer_size * sizeof(float));
	float *output_right = (float*) malloc(buffer_size * sizeof(float));

	int seed;

	fake_init();

	
	LADSPA_Handle limiter = foo_limiterDescriptor->instantiate( foo_limiterDescriptor, 44100);


	connectPortFoo_limiter(limiter, FOO_LIMITER_INPUT_GAIN_DB, &input_gain);
	connectPortFoo_limiter(limiter, FOO_LIMITER_MAX_PEAK_DB,   &max_peak);
	connectPortFoo_limiter(limiter, FOO_LIMITER_RELEASE_TIME,  &release_time);
	connectPortFoo_limiter(limiter, FOO_LIMITER_ATTENUATION,   &attenuation);
	connectPortFoo_limiter(limiter, FOO_LIMITER_LATENCY,       &latency);
	connectPortFoo_limiter(limiter, FOO_LIMITER_RELEASE_SCALE, &release_scale);

	connectPortFoo_limiter(limiter, FOO_LIMITER_INPUT_LEFT,    input_left);
	connectPortFoo_limiter(limiter, FOO_LIMITER_INPUT_RIGHT,   input_right);

	connectPortFoo_limiter(limiter, FOO_LIMITER_OUTPUT_LEFT,   output_left);
	connectPortFoo_limiter(limiter, FOO_LIMITER_OUTPUT_RIGHT,  output_right);

	memset(input_left,  0, buffer_size * sizeof(float));
	memset(input_right, 0, buffer_size * sizeof(float));

	seed = time(NULL);
	printf("using seed %d\n",seed);
	srand(seed);
	//create_random(input_left, buffer_size, 1.05f, 0.0f);
	create_ABCD(input_left, buffer_size);


	runFoo_limiter(limiter, buffer_size);

	printf("output[2216] = %f\n", output_left[2216]);
	
	write_file_constant("plot/max_peak.txt", max_peak, buffer_size);
	write_file("plot/input.txt",    input_left,   buffer_size);
	write_file("plot/output.txt",   output_left,  buffer_size);
	write_file("plot/envelope.txt", output_right, buffer_size);




	fake_fini();
	return 0;
}
#endif

void load_file(float *buffer, char *file, int samples)
{
	int fd = open(file, O_RDONLY);
	read(fd, buffer, samples * sizeof(float));
	close(fd);
}

#ifdef TEST_TRANSIENT_ARCHITECT
int test_architect(int argc, char **argv)
{
	int i;
	int buffer_size = 18238;

	float *input_left   = (float*) malloc(buffer_size * sizeof(float));
	float *input_right  = (float*) malloc(buffer_size * sizeof(float));

	float *output_left  = (float*) malloc(buffer_size * sizeof(float));
	float *output_right = (float*) malloc(buffer_size * sizeof(float));

	float latency;
	float atten_attack;
	float atten_release;

	float attack_gain_db  = 9.897f;
	float release_gain_db = 0.0f;

	float average_difference = 0.078f;

	int seed;
	seed = time(NULL);
	printf("using seed %d\n",seed);
	srand(seed);

	fake_init();

	LADSPA_Handle transients = foo_transientsDescriptor->instantiate( foo_transientsDescriptor, 44100);


	connectPortFoo_transients(transients, FOO_TRANSIENTS_LATENCY,         &latency);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_ACTUAL_ATTACK_GAIN, &atten_attack);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_ACTUAL_RELEASE_GAIN, &atten_release);

	connectPortFoo_transients(transients, FOO_TRANSIENTS_ATTACK_GAIN_DB,  &attack_gain_db);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_RELEASE_GAIN_DB, &release_gain_db);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_AVERAGE_DIFFERENCE, &average_difference);

	connectPortFoo_transients(transients, FOO_TRANSIENTS_INPUT_LEFT,      input_left);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_INPUT_RIGHT,     input_right);

	connectPortFoo_transients(transients, FOO_TRANSIENTS_OUTPUT_LEFT,     output_left);
	connectPortFoo_transients(transients, FOO_TRANSIENTS_OUTPUT_RIGHT,    output_right);


	//create_random(input_left, buffer_size, 0.8f, -0.4f);
	load_file(input_left, "snare.raw", buffer_size);
	memset(input_left,  0, sizeof(float) * 2048); // 2048 samples of zero at the start
	memset(input_right, 0, sizeof(float) * buffer_size);

	printf("input_left[%d] = %f\n", 2047, input_left[2047]);
	printf("input_left[%d] = %f\n", 2048, input_left[2048]);

	runFoo_transients(transients, buffer_size);

	write_file("plot/architect_input.txt",    input_left,   buffer_size);
	write_file("plot/architect_fast_avg.txt", output_left,  buffer_size);
	write_file("plot/architect_slow_avg.txt", output_right, buffer_size);

	for (i = 0; i < buffer_size; i++) {
		output_left[i] = fabsf(output_left[i] - output_right[i]);
	}

	write_file("plot/architect_complete_env.txt", output_left, buffer_size);



	fake_fini();
	return 0;
}
#endif

int main(int argc, char **argv)
{
#ifdef TEST_LIMITER
	return test_limiter(argc, argv);
#endif

#ifdef TEST_TRANSIENT_ARCHITECT
	return test_architect(argc, argv);
#endif
}
