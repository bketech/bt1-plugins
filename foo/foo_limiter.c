#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#endif

#define         _ISOC9X_SOURCE  1
#define         _ISOC99_SOURCE  1
#define         __USE_ISOC99    1
#define         __USE_ISOC9X    1

#include <math.h>
#include "utils.h"
#include "ladspa.h"

//#line 30 "foo_limiter.xml"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOO_LIMITER_RAMP_UP_MILLISECONDS 2.5f

#define FOO_LIMITER_MAX_LOGSCALE 10.0f

typedef struct _Envelope {
    int ramp_samples;
    //int sustain_samples;
    int release_samples;
    int length_samples; // ramp_samples + sustain_samples + release_samples
  
    float start_gain;
    float limit_gain;
  
    // ramp_delta = (env->limit_gain - env->start_gain)/(float)env->ramp_samples
    float ramp_delta;
  
    // release_delta = (1.0f - env->limit_gain)/(float)env->release_samples
    float release_delta;
  
    int at_sample; 

    // Logarithmic release envelope
    float logscale;
} Envelope;


// logscale parameter is given in 0.0f .. 1.0f, NOT in real scale values, this function sets the right scale
void FooLimiter_triggerEnvelope(Envelope *env, int ramp_samples, int release_samples, float current_gain, float new_limit_gain, float logscale)
{
    float new_ramp_delta = (new_limit_gain - current_gain)/(float)ramp_samples;

    // New envelopes are of suspect if previous envelope is still ramping up
    if (env->at_sample < env->ramp_samples) {
  
        // the new ramp delta is not as steep as the old one
        if (new_ramp_delta > env->ramp_delta) {
            // instead of creating a new envelope, we extend the current one
            env->at_sample = 0;
            env->start_gain = current_gain;
            env->limit_gain = current_gain + env->ramp_delta * (float)env->ramp_samples;
            env->release_delta = (1.0f - env->limit_gain)/(float)env->release_samples;

            // Logscale parameter will not be changed during ramp-up as it could
            // cause peaks to escape
            return;
        }

    }

    env->ramp_samples    = ramp_samples;
    env->release_samples = release_samples;
    env->length_samples  = env->ramp_samples + env->release_samples;
  
    env->start_gain      = current_gain;
    env->limit_gain      = new_limit_gain;
  
    env->ramp_delta      = (env->limit_gain - env->start_gain)/(float)env->ramp_samples;
    env->release_delta   = (1.0f - env->limit_gain)/(float)env->release_samples;
  
    env->logscale        = 1 / expf(1.0f) + logscale * (FOO_LIMITER_MAX_LOGSCALE - 1/expf(1.0f));
#ifdef FOO_TESTER
    printf("envelope logscale = %f (parameter %f)\n",env->logscale, logscale);

#endif 

    env->at_sample       = 0;
}

#define LOGSCALE(position, scale)        ( logf( (position) * exp(scale) + 1.0f - (position)) / (scale))

float FooLimiter_calculateEnvelope(Envelope *env, int offset)
{
    int at = env->at_sample + offset;
  
    if (at >= env->length_samples) return 1.0f;
    if (at <  0) return env->start_gain;
  
    // RAMP
    if (at <  env->ramp_samples) {
        return env->start_gain + (float)at * env->ramp_delta;
    }
  
    // RELEASE

    return env->limit_gain + ( (1.0f - env->limit_gain) * LOGSCALE( ((float) (at - env->ramp_samples))/(float)(env->release_samples), env->logscale));

    //return env->limit_gain + (float)(at - env->ramp_samples) * env->release_delta;
}

#define FOO_LIMITER_INPUT_GAIN_DB      0
#define FOO_LIMITER_MAX_PEAK_DB        1
#define FOO_LIMITER_RELEASE_TIME       2
#define FOO_LIMITER_ATTENUATION        3
#define FOO_LIMITER_INPUT_LEFT         4
#define FOO_LIMITER_INPUT_RIGHT        5
#define FOO_LIMITER_OUTPUT_LEFT        6
#define FOO_LIMITER_OUTPUT_RIGHT       7
#define FOO_LIMITER_LATENCY            8
#define FOO_LIMITER_RELEASE_SCALE      9

static LADSPA_Descriptor *foo_limiterDescriptor = NULL;

typedef struct {
	LADSPA_Data *input_gain_db;
	LADSPA_Data *max_peak_db;
	LADSPA_Data *release_time;
	LADSPA_Data *attenuation;
	LADSPA_Data *input_left;
	LADSPA_Data *input_right;
	LADSPA_Data *output_left;
	LADSPA_Data *output_right;
	LADSPA_Data *latency;
	LADSPA_Data *release_scale;
	float        current_gain;
	Envelope     envelope;
	int          ramp_up_time_samples;
	unsigned long samplerate;
	float *      workbuffer_left;
	float *      workbuffer_right;
	int          workbuffer_size;
	LADSPA_Data run_adding_gain;
} Foo_limiter;

const LADSPA_Descriptor *ladspa_descriptor(unsigned long index) {

	switch (index) {
	case 0:
		return foo_limiterDescriptor;
	default:
		return NULL;
	}
}

static void cleanupFoo_limiter(LADSPA_Handle instance) {
//#line 160 "foo_limiter.xml"
	Foo_limiter *plugin_data = (Foo_limiter *)instance;
	free(plugin_data->workbuffer_left);
	free(plugin_data->workbuffer_right);
	free(instance);
}

static void connectPortFoo_limiter(
 LADSPA_Handle instance,
 unsigned long port,
 LADSPA_Data *data) {
	Foo_limiter *plugin;

	plugin = (Foo_limiter *)instance;
	switch (port) {
	case FOO_LIMITER_INPUT_GAIN_DB:
		plugin->input_gain_db = data;
		break;
	case FOO_LIMITER_MAX_PEAK_DB:
		plugin->max_peak_db = data;
		break;
	case FOO_LIMITER_RELEASE_TIME:
		plugin->release_time = data;
		break;
	case FOO_LIMITER_ATTENUATION:
		plugin->attenuation = data;
		break;
	case FOO_LIMITER_INPUT_LEFT:
		plugin->input_left = data;
		break;
	case FOO_LIMITER_INPUT_RIGHT:
		plugin->input_right = data;
		break;
	case FOO_LIMITER_OUTPUT_LEFT:
		plugin->output_left = data;
		break;
	case FOO_LIMITER_OUTPUT_RIGHT:
		plugin->output_right = data;
		break;
	case FOO_LIMITER_LATENCY:
		plugin->latency = data;
		break;
	case FOO_LIMITER_RELEASE_SCALE:
		plugin->release_scale = data;
		break;
	}
}

static LADSPA_Handle instantiateFoo_limiter(
 const LADSPA_Descriptor *descriptor,
 unsigned long s_rate) {
	Foo_limiter *plugin_data = (Foo_limiter *)malloc(sizeof(Foo_limiter));
	float current_gain;
	Envelope envelope;
	int ramp_up_time_samples;
	unsigned long samplerate;
	float *workbuffer_left = NULL;
	float *workbuffer_right = NULL;
	int workbuffer_size;

//#line 133 "foo_limiter.xml"
	samplerate = s_rate;
	ramp_up_time_samples = (int)floor((float)samplerate * (float)FOO_LIMITER_RAMP_UP_MILLISECONDS / 1000.0f);
#ifdef FOO_TESTER
	printf("Instantiating FooLimiter, ramp_up_time_samples = %d\n",ramp_up_time_samples);
#endif

	workbuffer_size  = 4096 + ramp_up_time_samples;
	workbuffer_left  = (float *) malloc(sizeof(float) * workbuffer_size);
	workbuffer_right = (float *) malloc(sizeof(float) * workbuffer_size);

	memset(workbuffer_left,  0, sizeof(float) * workbuffer_size);
	memset(workbuffer_right, 0, sizeof(float) * workbuffer_size);

	current_gain = 1.0f;

	// create dummy envelope which has already been passed
	// make sure triggerEnvelope doesn't use uninitialized values
	envelope.at_sample       = -1;
	envelope.ramp_samples    = 1;
	envelope.ramp_delta      = 0.0f;
	envelope.release_samples = 1;
	FooLimiter_triggerEnvelope(&envelope, 1, 1, 1.0f, 1.0f, 1.0f);
	envelope.at_sample       = 3;

	plugin_data->current_gain = current_gain;
	plugin_data->envelope = envelope;
	plugin_data->ramp_up_time_samples = ramp_up_time_samples;
	plugin_data->samplerate = samplerate;
	plugin_data->workbuffer_left = workbuffer_left;
	plugin_data->workbuffer_right = workbuffer_right;
	plugin_data->workbuffer_size = workbuffer_size;

	return (LADSPA_Handle)plugin_data;
}

#undef buffer_write
#undef RUN_ADDING
#undef RUN_REPLACING

#define buffer_write(b, v) (b = v)
#define RUN_ADDING    0
#define RUN_REPLACING 1

static void runFoo_limiter(LADSPA_Handle instance, unsigned long sample_count) {
	Foo_limiter *plugin_data = (Foo_limiter *)instance;

	/* Input gain (dB) (float value) */
	const LADSPA_Data input_gain_db = *(plugin_data->input_gain_db);

	/* Max level (dB) (float value) */
	const LADSPA_Data max_peak_db = *(plugin_data->max_peak_db);

	/* Release time (s) (float value) */
	const LADSPA_Data release_time = *(plugin_data->release_time);

	/* Input L (array of floats of length sample_count) */
	const LADSPA_Data * const input_left = plugin_data->input_left;

	/* Input R (array of floats of length sample_count) */
	const LADSPA_Data * const input_right = plugin_data->input_right;

	/* Output L (array of floats of length sample_count) */
	LADSPA_Data * const output_left = plugin_data->output_left;

	/* Output R (array of floats of length sample_count) */
	LADSPA_Data * const output_right = plugin_data->output_right;

	/* Linear/log release (float value) */
	const LADSPA_Data release_scale = *(plugin_data->release_scale);
	float current_gain = plugin_data->current_gain;
	Envelope envelope = plugin_data->envelope;
	int ramp_up_time_samples = plugin_data->ramp_up_time_samples;
	unsigned long samplerate = plugin_data->samplerate;
	float * workbuffer_left = plugin_data->workbuffer_left;
	float * workbuffer_right = plugin_data->workbuffer_right;
	int workbuffer_size = plugin_data->workbuffer_size;

//#line 165 "foo_limiter.xml"
	
	        unsigned long n,i,lookahead, buffer_offset;
	        float min_gain = 1.0f;
	
	        float peak_left, peak_right, tmp, max_peak, input_gain;
	
	        max_peak   = DB_CO(max_peak_db);
	        input_gain = DB_CO(input_gain_db);
	
	        buffer_offset = 0;
	
	        while (sample_count > 0) {
	            n = sample_count;
	
	            // Make sure we process in slices where "lookahead + slice" fit in the work buffer
	            if (n > workbuffer_size - ramp_up_time_samples)
	                n = workbuffer_size - ramp_up_time_samples;
	
	            // The start of the workbuffer contains the "latent" samples from the previous run
	            memcpy(workbuffer_left  + ramp_up_time_samples, input_left  + buffer_offset, sizeof(float) * n);
	            memcpy(workbuffer_right + ramp_up_time_samples, input_right + buffer_offset, sizeof(float) * n);
	
	            lookahead = ramp_up_time_samples;
	            for (i = 0; i < n; i++, lookahead++) {
	
	                workbuffer_left [lookahead] *= input_gain;
	                workbuffer_right[lookahead] *= input_gain;
	
	                current_gain = FooLimiter_calculateEnvelope(&envelope, 0);
	
	                // Peak detection
	                peak_left  = fabsf(workbuffer_left [lookahead]);
	                peak_right = fabsf(workbuffer_right[lookahead]);
	
	                tmp = fmaxf(peak_left, peak_right) * FooLimiter_calculateEnvelope(&envelope, ramp_up_time_samples);
	
	                if (tmp > max_peak) {
	
	                    // calculate needed_limit_gain
	                    float needed_limit_gain = max_peak / fmaxf(peak_left, peak_right);
	
	                    int release_samples = (int)floor( release_time * (float)samplerate);
	
	#ifdef FOO_TESTER                            
	                    printf("new envelope! release samples = %d, max_peak = %f, current_gain = %f, peak = %f, limit_gain = %f\n", release_samples, max_peak, current_gain, tmp, needed_limit_gain);
	#endif
	                    FooLimiter_triggerEnvelope(&envelope, ramp_up_time_samples, release_samples, current_gain, needed_limit_gain, release_scale);
	                }
	                
	                if (current_gain < min_gain) min_gain = current_gain;
	
	                if (envelope.at_sample <= envelope.length_samples)
	                    envelope.at_sample++;
	
	
	                buffer_write(output_left [i+buffer_offset], workbuffer_left [i] * current_gain);
	#ifndef FOO_TESTER
	                buffer_write(output_right[i+buffer_offset], workbuffer_right[i] * current_gain);
	#else
	                buffer_write(output_right[i+buffer_offset], current_gain);
	                if (fabsf(output_left[i+buffer_offset]) > max_peak) {
	                        printf("######### Limiter failed at %ld, max_peak = %f and peaked at %f\n", i+buffer_offset, max_peak, output_left[i+buffer_offset]);
	                        printf("## Envelope: @%d, ramp_delta = %f, release_delta = %f, ramp_samples = %d, release_samples = %d, start_gain = %f, limit_gain = %f\n",
	                               envelope.at_sample,
	                               envelope.ramp_delta,
	                               envelope.release_delta,
	                               envelope.ramp_samples,
	                               envelope.release_samples,
	                               envelope.start_gain,
	                               envelope.limit_gain);
	                }
	#endif
	
	            }
	
	            // copy the "end" of input data to the lookahead buffer
	            // copy input[ (n-ramp_up_time_samples] .. n ] => lookahead
	
	            if (n < ramp_up_time_samples) {
	                memmove(workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
	                memmove(workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
	            } else {
	                memcpy (workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
	                memcpy (workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
	            }
	
	            sample_count -= n;
	            buffer_offset += n;
	        }
	
	        *(plugin_data->attenuation) = -CO_DB(min_gain);
	        *(plugin_data->latency)     = ramp_up_time_samples;
	
	        plugin_data->current_gain   = current_gain;
	        plugin_data->envelope       = envelope;
}
#undef buffer_write
#undef RUN_ADDING
#undef RUN_REPLACING

#define buffer_write(b, v) (b += (v) * run_adding_gain)
#define RUN_ADDING    1
#define RUN_REPLACING 0

static void setRunAddingGainFoo_limiter(LADSPA_Handle instance, LADSPA_Data gain) {
	((Foo_limiter *)instance)->run_adding_gain = gain;
}

static void runAddingFoo_limiter(LADSPA_Handle instance, unsigned long sample_count) {
	Foo_limiter *plugin_data = (Foo_limiter *)instance;
	LADSPA_Data run_adding_gain = plugin_data->run_adding_gain;

	/* Input gain (dB) (float value) */
	const LADSPA_Data input_gain_db = *(plugin_data->input_gain_db);

	/* Max level (dB) (float value) */
	const LADSPA_Data max_peak_db = *(plugin_data->max_peak_db);

	/* Release time (s) (float value) */
	const LADSPA_Data release_time = *(plugin_data->release_time);

	/* Input L (array of floats of length sample_count) */
	const LADSPA_Data * const input_left = plugin_data->input_left;

	/* Input R (array of floats of length sample_count) */
	const LADSPA_Data * const input_right = plugin_data->input_right;

	/* Output L (array of floats of length sample_count) */
	LADSPA_Data * const output_left = plugin_data->output_left;

	/* Output R (array of floats of length sample_count) */
	LADSPA_Data * const output_right = plugin_data->output_right;

	/* Linear/log release (float value) */
	const LADSPA_Data release_scale = *(plugin_data->release_scale);
	float current_gain = plugin_data->current_gain;
	Envelope envelope = plugin_data->envelope;
	int ramp_up_time_samples = plugin_data->ramp_up_time_samples;
	unsigned long samplerate = plugin_data->samplerate;
	float * workbuffer_left = plugin_data->workbuffer_left;
	float * workbuffer_right = plugin_data->workbuffer_right;
	int workbuffer_size = plugin_data->workbuffer_size;

//#line 165 "foo_limiter.xml"
	
	        unsigned long n,i,lookahead, buffer_offset;
	        float min_gain = 1.0f;
	
	        float peak_left, peak_right, tmp, max_peak, input_gain;
	
	        max_peak   = DB_CO(max_peak_db);
	        input_gain = DB_CO(input_gain_db);
	
	        buffer_offset = 0;
	
	        while (sample_count > 0) {
	            n = sample_count;
	
	            // Make sure we process in slices where "lookahead + slice" fit in the work buffer
	            if (n > workbuffer_size - ramp_up_time_samples)
	                n = workbuffer_size - ramp_up_time_samples;
	
	            // The start of the workbuffer contains the "latent" samples from the previous run
	            memcpy(workbuffer_left  + ramp_up_time_samples, input_left  + buffer_offset, sizeof(float) * n);
	            memcpy(workbuffer_right + ramp_up_time_samples, input_right + buffer_offset, sizeof(float) * n);
	
	            lookahead = ramp_up_time_samples;
	            for (i = 0; i < n; i++, lookahead++) {
	
	                workbuffer_left [lookahead] *= input_gain;
	                workbuffer_right[lookahead] *= input_gain;
	
	                current_gain = FooLimiter_calculateEnvelope(&envelope, 0);
	
	                // Peak detection
	                peak_left  = fabsf(workbuffer_left [lookahead]);
	                peak_right = fabsf(workbuffer_right[lookahead]);
	
	                tmp = fmaxf(peak_left, peak_right) * FooLimiter_calculateEnvelope(&envelope, ramp_up_time_samples);
	
	                if (tmp > max_peak) {
	
	                    // calculate needed_limit_gain
	                    float needed_limit_gain = max_peak / fmaxf(peak_left, peak_right);
	
	                    int release_samples = (int)floor( release_time * (float)samplerate);
	
	#ifdef FOO_TESTER                            
	                    printf("new envelope! release samples = %d, max_peak = %f, current_gain = %f, peak = %f, limit_gain = %f\n", release_samples, max_peak, current_gain, tmp, needed_limit_gain);
	#endif
	                    FooLimiter_triggerEnvelope(&envelope, ramp_up_time_samples, release_samples, current_gain, needed_limit_gain, release_scale);
	                }
	                
	                if (current_gain < min_gain) min_gain = current_gain;
	
	                if (envelope.at_sample <= envelope.length_samples)
	                    envelope.at_sample++;
	
	
	                buffer_write(output_left [i+buffer_offset], workbuffer_left [i] * current_gain);
	#ifndef FOO_TESTER
	                buffer_write(output_right[i+buffer_offset], workbuffer_right[i] * current_gain);
	#else
	                buffer_write(output_right[i+buffer_offset], current_gain);
	                if (fabsf(output_left[i+buffer_offset]) > max_peak) {
	                        printf("######### Limiter failed at %ld, max_peak = %f and peaked at %f\n", i+buffer_offset, max_peak, output_left[i+buffer_offset]);
	                        printf("## Envelope: @%d, ramp_delta = %f, release_delta = %f, ramp_samples = %d, release_samples = %d, start_gain = %f, limit_gain = %f\n",
	                               envelope.at_sample,
	                               envelope.ramp_delta,
	                               envelope.release_delta,
	                               envelope.ramp_samples,
	                               envelope.release_samples,
	                               envelope.start_gain,
	                               envelope.limit_gain);
	                }
	#endif
	
	            }
	
	            // copy the "end" of input data to the lookahead buffer
	            // copy input[ (n-ramp_up_time_samples] .. n ] => lookahead
	
	            if (n < ramp_up_time_samples) {
	                memmove(workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
	                memmove(workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
	            } else {
	                memcpy (workbuffer_left,  workbuffer_left  + n , sizeof(float) * ramp_up_time_samples);
	                memcpy (workbuffer_right, workbuffer_right + n , sizeof(float) * ramp_up_time_samples);
	            }
	
	            sample_count -= n;
	            buffer_offset += n;
	        }
	
	        *(plugin_data->attenuation) = -CO_DB(min_gain);
	        *(plugin_data->latency)     = ramp_up_time_samples;
	
	        plugin_data->current_gain   = current_gain;
	        plugin_data->envelope       = envelope;
}

#ifndef FOO_TESTER
void _init() {
#else
void fake_init() {
#endif
	char **port_names;
	LADSPA_PortDescriptor *port_descriptors;
	LADSPA_PortRangeHint *port_range_hints;

#ifdef ENABLE_NLS
#define D_(s) dgettext(PACKAGE, s)
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, PACKAGE_LOCALE_DIR);
#else
#define D_(s) (s)
#endif


	foo_limiterDescriptor =
	 (LADSPA_Descriptor *)malloc(sizeof(LADSPA_Descriptor));

	if (foo_limiterDescriptor) {
		foo_limiterDescriptor->UniqueID = 3181;
		foo_limiterDescriptor->Label = "foo_limiter";
		foo_limiterDescriptor->Properties =
		 LADSPA_PROPERTY_HARD_RT_CAPABLE;
		foo_limiterDescriptor->Name =
		 D_("Foo Lookahead Limiter");
		foo_limiterDescriptor->Maker =
		 "Sampo Savolainen <v2@iki.fi>";
		foo_limiterDescriptor->Copyright =
		 "GPL";
		foo_limiterDescriptor->PortCount = 10;

		port_descriptors = (LADSPA_PortDescriptor *)calloc(10,
		 sizeof(LADSPA_PortDescriptor));
		foo_limiterDescriptor->PortDescriptors =
		 (const LADSPA_PortDescriptor *)port_descriptors;

		port_range_hints = (LADSPA_PortRangeHint *)calloc(10,
		 sizeof(LADSPA_PortRangeHint));
		foo_limiterDescriptor->PortRangeHints =
		 (const LADSPA_PortRangeHint *)port_range_hints;

		port_names = (char **)calloc(10, sizeof(char*));
		foo_limiterDescriptor->PortNames =
		 (const char **)port_names;

		/* Parameters for Input gain (dB) */
		port_descriptors[FOO_LIMITER_INPUT_GAIN_DB] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_INPUT_GAIN_DB] =
		 D_("Input gain (dB)");
		port_range_hints[FOO_LIMITER_INPUT_GAIN_DB].HintDescriptor =
		 LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0;
		port_range_hints[FOO_LIMITER_INPUT_GAIN_DB].LowerBound = -20.0;
		port_range_hints[FOO_LIMITER_INPUT_GAIN_DB].UpperBound = +10.0;

		/* Parameters for Max level (dB) */
		port_descriptors[FOO_LIMITER_MAX_PEAK_DB] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_MAX_PEAK_DB] =
		 D_("Max level (dB)");
		port_range_hints[FOO_LIMITER_MAX_PEAK_DB].HintDescriptor =
		 LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_0;
		port_range_hints[FOO_LIMITER_MAX_PEAK_DB].LowerBound = -30.0;
		port_range_hints[FOO_LIMITER_MAX_PEAK_DB].UpperBound = +0.0;

		/* Parameters for Release time (s) */
		port_descriptors[FOO_LIMITER_RELEASE_TIME] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_RELEASE_TIME] =
		 D_("Release time (s)");
		port_range_hints[FOO_LIMITER_RELEASE_TIME].HintDescriptor =
		 LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_LOW;
		port_range_hints[FOO_LIMITER_RELEASE_TIME].LowerBound = 0.01;
		port_range_hints[FOO_LIMITER_RELEASE_TIME].UpperBound = 2.0;

		/* Parameters for Attenuation (dB) */
		port_descriptors[FOO_LIMITER_ATTENUATION] =
		 LADSPA_PORT_OUTPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_ATTENUATION] =
		 D_("Attenuation (dB)");
		port_range_hints[FOO_LIMITER_ATTENUATION].HintDescriptor =
		 LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
		port_range_hints[FOO_LIMITER_ATTENUATION].LowerBound = 0.0;
		port_range_hints[FOO_LIMITER_ATTENUATION].UpperBound = +70.0;

		/* Parameters for Input L */
		port_descriptors[FOO_LIMITER_INPUT_LEFT] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		port_names[FOO_LIMITER_INPUT_LEFT] =
		 D_("Input L");
		port_range_hints[FOO_LIMITER_INPUT_LEFT].HintDescriptor = 0;

		/* Parameters for Input R */
		port_descriptors[FOO_LIMITER_INPUT_RIGHT] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
		port_names[FOO_LIMITER_INPUT_RIGHT] =
		 D_("Input R");
		port_range_hints[FOO_LIMITER_INPUT_RIGHT].HintDescriptor = 0;

		/* Parameters for Output L */
		port_descriptors[FOO_LIMITER_OUTPUT_LEFT] =
		 LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_names[FOO_LIMITER_OUTPUT_LEFT] =
		 D_("Output L");
		port_range_hints[FOO_LIMITER_OUTPUT_LEFT].HintDescriptor = 0;

		/* Parameters for Output R */
		port_descriptors[FOO_LIMITER_OUTPUT_RIGHT] =
		 LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_names[FOO_LIMITER_OUTPUT_RIGHT] =
		 D_("Output R");
		port_range_hints[FOO_LIMITER_OUTPUT_RIGHT].HintDescriptor = 0;

		/* Parameters for latency */
		port_descriptors[FOO_LIMITER_LATENCY] =
		 LADSPA_PORT_OUTPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_LATENCY] =
		 D_("latency");
		port_range_hints[FOO_LIMITER_LATENCY].HintDescriptor = 0;

		/* Parameters for Linear/log release */
		port_descriptors[FOO_LIMITER_RELEASE_SCALE] =
		 LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[FOO_LIMITER_RELEASE_SCALE] =
		 D_("Linear/log release");
		port_range_hints[FOO_LIMITER_RELEASE_SCALE].HintDescriptor =
		 LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_DEFAULT_HIGH;
		port_range_hints[FOO_LIMITER_RELEASE_SCALE].LowerBound = 0.0;
		port_range_hints[FOO_LIMITER_RELEASE_SCALE].UpperBound = 1.0;

		foo_limiterDescriptor->activate = NULL;
		foo_limiterDescriptor->cleanup = cleanupFoo_limiter;
		foo_limiterDescriptor->connect_port = connectPortFoo_limiter;
		foo_limiterDescriptor->deactivate = NULL;
		foo_limiterDescriptor->instantiate = instantiateFoo_limiter;
		foo_limiterDescriptor->run = runFoo_limiter;
		foo_limiterDescriptor->run_adding = runAddingFoo_limiter;
		foo_limiterDescriptor->set_run_adding_gain = setRunAddingGainFoo_limiter;
	}
}

#ifndef FOO_TESTER
void _fini() {
#else
void fake_fini() {
#endif
	if (foo_limiterDescriptor) {
		free((LADSPA_PortDescriptor *)foo_limiterDescriptor->PortDescriptors);
		free((char **)foo_limiterDescriptor->PortNames);
		free((LADSPA_PortRangeHint *)foo_limiterDescriptor->PortRangeHints);
		free(foo_limiterDescriptor);
	}

}
