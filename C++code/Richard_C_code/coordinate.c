/** Quick knock-up of the coordination game, model B; our signal and meaning spaces
    are binary strings for now.
    
    This version has a fixed uniform meaning distribution, and the grammar values phi(m,s)
    can now be interpreted as the probability of producing m given s. These remain normalised
    such that sum_s phi(m,s) = 1 at all times. This version uses the implicit tanh walk.
    
    Author:  Richard Blythe, University of Edinburgh <R.A.Blythe@ed.ac.uk>
    Version: 0.04, 20th October 2016
    License: Apache License, Version 2.0 https://www.apache.org/licenses/
**/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/* This code relies on having the GNU scientific library installed, see https://www.gnu.org/software/gsl/

   On a Mac you will also need argp-standalone, see https://github.com/jahrome/argp-standalone

   To compile (Mac):
   $ clang -Wall -o coordinate coordinate.c $(gsl-config --cflags) $(gsl-config --libs) -largp
   
   To compile (Linux):
   $ gcc -Wall -std=c99 -o coordinate coordinate.c $(gsl-config --cflags) $(gsl-config --libs)
   
   To compile (Windows):
   you're on your own, sorry
   
   Alternatively, choose the appropriate makefile and run GNU Make
*/

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <argp.h>

unsigned agents = 10;		// Number of agents
unsigned rounds = 100;		// Number of interaction rounds to run

unsigned bits = 4;			// Number of bits in meaning/signal space
double meaning_leakage = 0.1; // Total contribution to pseudo-associations from neighbouring meanings
double signal_leakage = 0.1; // Total contribution to pseudo-associations from neighbouring signals

double feedback = 1.0; // Quality of feedback
double stepsize = 0.01; // USM style lambda parameter

unsigned positivity = 0; // Suppress negative feedback if set

typedef enum {
	ic_identity,
	ic_random,
	ic_rook
} ic_t;

ic_t initial_condition = ic_random;

const double EPSILON = 1e-2; // Perturbations from unity when using tanh rule

// Allow the command line to override defaults

static struct argp_option options[] = {
	{"agents",	'N', "COUNT",	0, "System size"},
	{"rounds",	't', "COUNT",	0, "Number of interaction rounds"},
    {"bits",	'b', "COUNT",   0, "Number of meaning/signal bits"},
    {"mleak",	'm', "NUMBER",	0, "Weight acquired by neighbouring meanings"},
    {"sleak",	's', "NUMBER",  0, "Weight acquired by neighbouring signals"},
    {"feedback",'f', "NUMBER",  0, "Feedback quality (0 for deterministic feedback)"},
    {"vary",	'v', "NUMBER",  0, "Variation in association per interaction"},
    {"init",    'i', "IC",      0, "Initial condition [uniform|identity|rook]"},
    {"positivity", 'p', 0,      0, "Suppress negative feedback"},
    {0}
};

static error_t ArgumentParser(int key, char* arg, struct argp_state* state) {
    (void)state;
    
    switch(key) {
            
        case 'N':	agents = atoi(arg); break;
        case 't':	rounds = atoi(arg); break;
        case 'b':	bits = atoi(arg); break;
        case 'm':	meaning_leakage = atof(arg); break;
        case 's':	signal_leakage = atof(arg); break;
        case 'f':	feedback = atof(arg); break;
        case 'v':	stepsize = atof(arg); break;
        case 'p':	positivity = 1; break;
        case 'i':	
        	if(arg[0] == 'i') initial_condition = ic_identity;
        	else if(arg[0] == 'r') initial_condition = ic_rook;
        	else initial_condition = ic_random;
        	break;
        	            
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {options, ArgumentParser, 0,
    "Coordination Model B"
};


/** Define signal and meaning spaces; both of these will be binary strings of the same length **/

typedef unsigned meaning_t;
typedef unsigned signal_t;

/* Final part of the initialisation (since bits may be reconfigured on the command line) */

unsigned meanings;
unsigned signals;

void init_globals() {
	meanings  =  1 << bits;
	signals   =  1 << bits;
}

/* Helper function: return how many of the lowest n bits in b1 and b2 are the same */
unsigned common_bits(unsigned b1, unsigned b2, unsigned n) { 
	unsigned x = b1 ^ b2; // Find the xor of b1, b2; now count the number of bits that are clear
	unsigned m = 0;
	for(int i=0; i<n; i++) {
		if((x & 1) == 0) m++;
		x>>=1; // Shift one bit to the right
	} // There is probably a more efficient way to do this...
	return m;
}

/* Return a number between 0 and 1 as a measure of similarity of two meanings, 0=no similarity, 1=same.
   I am going to use the number of bits that are the same */
double meaning_similarity(meaning_t m1, meaning_t m2) {
	return m1 == m2 ? 1.0 : (double)common_bits(m1,m2,bits) / (double)bits; 
}

/* Return a number between 0 and 1 as a measure of similarity of two signals, 0=no similarity, 1=same.
   I am going to use the same metric as for meanings */
double signal_similarity(signal_t s1, signal_t s2) {
	return s1 == s2 ? 1.0 : (double)common_bits(s1,s2,bits) / (double)bits; 
}

/* Structure to get nearest neighbour meanings */
typedef struct {
	meaning_t base;
	unsigned next;
} meaning_iterator;

/* To iterator over neigbours of m, create a meaning iterator at the start of your code
   meaning_iterator* mi = mi_init(); // Raise error if zero
   
   Then, when you need it, do
   mi_start(mi,m); // This returns the number of neighbours, should you need it...
   while(mi_hasnext(mi)) {
      mm = mi_next(mi);
      // m and mm are nearest neighbours
   }

   Ultimately, 
   mi_free(mi);
*/   

meaning_iterator* mi_init() {
	meaning_iterator* mi = malloc(sizeof(*mi));
	return mi;
}

unsigned mi_start(meaning_iterator* mi, meaning_t m) {
	mi->base = m;
	mi->next = 0;
	return bits;
}

unsigned mi_hasnext(meaning_iterator* mi) {
	return mi->next < bits;
}

meaning_t mi_next(meaning_iterator* mi) {
	return mi->base ^ (1 << (mi->next++)); // This should bring tears to your eyes
}

void mi_free(meaning_iterator* mi) {
	if(mi) free(mi);
}

/* And now, the same thing for signals; we just duplicate code here because meanings
   and signals are represented in the same way, but eventually they might not be */
   
typedef struct {
	signal_t base;
	unsigned next;
} signal_iterator;

signal_iterator* si_init() {
	signal_iterator* si = malloc(sizeof(*si));
	return si;
}

unsigned si_start(signal_iterator* si, signal_t s) {
	si->base = s;
	si->next = 0;
	return bits;
}

unsigned si_hasnext(signal_iterator* si) {
	return si->next < bits;
}

signal_t si_next(signal_iterator* si) {
	return si->base ^ (1 << (si->next++)); // This should bring tears to your eyes
}

void si_free(signal_iterator* si) {
	if(si) free(si);
}


/** Define grammars **/

typedef struct {
	double* msraw; // Raw association
	double* msp;   // Pseudo association (convolution of the raw association)
} *grammar_t;

/* Some workspace for the following functions */
struct {
	double* m;
	double* s;
	meaning_iterator* mi;
	signal_iterator* si;
} _gws;

/* Initialise the workspace; returns nonzero for success, zero otherwise */
unsigned wsinit() {
	_gws.m = malloc(sizeof(*(_gws.m))*meanings);
	_gws.s = malloc(sizeof(*(_gws.s))*signals);
	_gws.mi = mi_init();
	_gws.si = si_init();
	return (_gws.m && _gws.s && _gws.mi && _gws.si);
}

/* Dispose of workspace */
void wsfree() {
	if(_gws.m) free(_gws.m);
	if(_gws.s) free(_gws.s);
	if(_gws.mi) mi_free(_gws.mi);
	if(_gws.si) si_free(_gws.si);
}

/* Initialise a new grammar; return 0 if unsuccessful */
grammar_t ginit() {
	grammar_t g = malloc(sizeof(g));
	if(!g) return 0;
	g->msraw = malloc(sizeof(g->msraw)*(meanings*signals));
	if(!(g->msraw)) { free(g); return 0; }
	g->msp = malloc(sizeof(g->msraw)*(meanings*signals));
	if(!(g->msp)) { free(g->msraw); free(g); return 0; }
	return g;
}	

/* Dispose of a grammar */
void gfree(grammar_t g) {
	if(g) {
		if(g->msraw) free(g->msraw);
		free(g);
	}	
}

/* Use a random number generator to create a set of random associations */
void randomise_associations(grammar_t g, gsl_rng* rng) {
	if(!g) return;
	for(meaning_t m=0; m<meanings; m++) {
		double Z = 0.0;
		for(signal_t s=0; s<signals; s++) {
			Z += (g->msraw[m*signals+s] = gsl_rng_uniform(rng));
		}
		// Renormalise so that sum_s phi(m,s) =1 for all m
		for(signal_t s=0; s<signals; s++) {
			g->msraw[m*signals+s] /= Z;
		}
	}
}

/* Fixed 1-1  relationship between meanings and signals */

void identity_associations(grammar_t g) {
	if(!g) return;
	for(meaning_t m=0; m<meanings; m++) {
		for(signal_t s=0; s<signals; s++) {
			g->msraw[m*signals+s] = m == s ? 1.0-EPSILON : 0.0+EPSILON/(double)(signals-1);
		}
	}
}

/* Random 1-1 relationship between meanings and signals; NB won't work if meanings>signals */
void rook_associations(grammar_t g, gsl_rng* rng) {
	if(!g) return;
	assert(signals>=meanings);
	// Get a random permutation of the integers
	int sigs[signals];
	for(signal_t s=0; s<signals; s++) {
		sigs[s] = s;
	}
	gsl_ran_shuffle(rng, sigs, signals, sizeof(*sigs));
	// Now take sig[m] to be the signal for meaning m
	for(meaning_t m=0; m<meanings; m++) {
		for(signal_t s=0; s<signals; s++) {
			g->msraw[m*signals+s] = s == sigs[m] ? 1.0-EPSILON : 0.0+EPSILON/(double)(signals-1);
		}
	}
	
}


/* Get the raw association between a meaning and a signal (probability you produce this signal for specified meaning) */
double get_association(grammar_t g, meaning_t m, signal_t s) {
	return g->msraw[m*signals+s];
}

/* Construct the pseudo-associations from the raw associations.
   In this version we just do nearest-neighbour leakage.
*/

void renew_pseudo_associations(grammar_t g) {
	for(meaning_t m=0; m<meanings; m++) {
		for(signal_t s=0; s<signals; s++) {
			g->msp[m*signals+s] = (1.0 - meaning_leakage - signal_leakage) * get_association(g,m,s); // By default, a pseudo-association is the same as the assocation
			// Rattle through neighbours in meaning and signal space, and apply leakage
			if(meaning_leakage>0.0) {
				double wt = (double)mi_start(_gws.mi, m);
				wt = meaning_leakage / wt; // This might not be norm preserving on general meaning-signal networks
				while(mi_hasnext(_gws.mi)) {
					g->msp[m*signals+s] += get_association(g,mi_next(_gws.mi),s) * wt;
				}
			}
			if(signal_leakage>0.0) {
				double wt = (double)si_start(_gws.si, s);
				wt = signal_leakage / wt; // This might not be norm preserving on general meaning-signal networks
				while(si_hasnext(_gws.si)) {
					g->msp[m*signals+s] += get_association(g,m,si_next(_gws.si)) * wt;
				}
			}
		}
	}					
}

/* Get the pseudo association between a meaning and a signal; note this may not
   reflect recent changes to the grammar until renew_pseudo_associations(g) is called */
double get_pseudo_association(grammar_t g, meaning_t m, signal_t s) {
	return g->msp[m*signals+s];
}

/* Helper function, given a cumulative distribution and a point in the range, find the corresponding bin */
unsigned find_bin(double* cumdist, unsigned n, double pt) {
	unsigned lo = 0, hi = n; // Bin lies somewhere in [lo,hi)
	while(hi>lo+1) {
		// Shrink one end of the range to the midpoint
		unsigned at = (hi+lo-1)/2;
		if(cumdist[at] <= pt) lo = at+1;
		else hi = at+1;
	}
	return lo;
}

/* Choose a meaning to convey, given a grammar and a random number generator.
   In this version, meanings are chosen from a uniform distribution
   We use the pseudo-association for this */
meaning_t sample_meaning(grammar_t g, gsl_rng* rng) {
	return (meaning_t)gsl_rng_uniform_int(rng, meanings);
}

/* Produce a signal for a specified meaning. Again use pseudo-association */
signal_t produce_signal(grammar_t g, meaning_t m, gsl_rng* rng) {
	/* Construct the cumulative distribution of signals */
	double Z = 0.0;
	for(signal_t s=0; s<signals; s++) {
		Z = _gws.s[s] = Z + g->msp[m*signals+s];	
	}
	/* Sample from this distribution */
	return (signal_t)find_bin(_gws.s, signals, Z*gsl_rng_uniform(rng));
}

/* Infer a meaning for a specified signal */
meaning_t infer_meaning(grammar_t g, signal_t s, gsl_rng* rng) {
	/* Construct the cumulative distribution of meaning inferences */
	double Z = 0.0;
	for(meaning_t m=0; m<meanings; m++) {
		Z = _gws.m[m] = Z + g->msp[m*signals+s];	
	}
	/* Sample from this distribution */
	return (meaning_t)find_bin(_gws.m, meanings, Z*gsl_rng_uniform(rng));
}

/* Feedback functions: return a number from -1 to 1 indicating a range from strongly negative
   to strongly positive feedback */
   
/* Deterministic feedback. This allows a "hand of God" to determine whether the meanings
   directly align or not. Return +1 for same meaning; -1 for different meanings */

double deterministic_feedback(meaning_t m1, meaning_t m2) {
	return m1 == m2 ? 1.0 : (positivity ? 0.0 : -1.0);
}

/* Stochastic feedback. Here feedback is noisy, with an increased chance of getting
   more positive feedback as meanings get more similar. The feedback parameter sets the
   maximum probability of obtaining positive (negative) feedback in the case of perfect
   (mis)alignment
*/

double stochastic_feedback(meaning_t m1, meaning_t m2, gsl_rng* rng) {
	/* The specific formula is +1 w.p. (feedback) (dm)^2
	                           -1 w.p. (feedback) (1-dm)^2
	                           0  otherwise
	*/
	double dm = meaning_similarity(m1,m2);
	double r = gsl_rng_uniform(rng);
	if ( r < feedback * dm * dm) return 1.0;
	else if ( positivity || r < 1.0 - feedback*(1.0-dm)*(1.0-dm)) return 0.0;
	else return -1.0;
}

/* Vary the association between a meaning and a signal by an amount; make the
   'by' parameter positive to strengthen, negative to weaken; you should call
   renew_pseudo_associations(g) when you want the change to take effect */
void vary_association_usm(grammar_t g, meaning_t m, signal_t s, double by) {
	if(by > 0.0) {
		// Classic USM rule
		double Z = 0.0;
		g->msraw[m*signals+s] += by;
		for(signal_t ss=0; ss<signals; ss++) {
			Z+= g->msraw[m*signals+ss];
		}
		for(signal_t ss=0; ss<signals; ss++) {
			g->msraw[m*signals+ss] /= Z;
		}
	} 
	else if(by < 0.0) {
		// Inverse USM rule - we increase all other m-s associations by a smaller amount 
		by = -by / (double)(signals - 1);
		double Z = 0.0;
		for(signal_t ss=0; ss<signals; ss++) {
			Z+= (g->msraw[m*signals+ss] = (g->msraw[m*signals+ss] + (s == ss ? 0.0 : by)));
		}
		for(signal_t ss=0; ss<signals; ss++) {
			g->msraw[m*signals+ss] /= Z;
		}
	}
	
}

void vary_association_tanh(grammar_t g, meaning_t m, signal_t s, double by) {
	double Z = 0.0;
	double x = g->msraw[m*signals+s];
	g->msraw[m*signals+s] += by * x*(1.0-x);
	for(signal_t ss=0; ss<signals; ss++) {
		Z+= g->msraw[m*signals+ss];
	}
	for(signal_t ss=0; ss<signals; ss++) {
		g->msraw[m*signals+ss] /= Z;
	}
	double Znow = 0;
	for(signal_t ss=0; ss<signals; ss++) Znow += g->msraw[m*signals+ss];
}

// Choose an update rule
void (*vary_association) (grammar_t, meaning_t, signal_t, double) = vary_association_tanh;
	

/* A measure of coherence of two grammars. We define this as the probability that, 
   if g1 is a speaker's grammar and g2 the listener's grammar, the intended meaning
   is correctly reconstructed; note this is not symmetric in g1, g2 */

double coherence(grammar_t g1, grammar_t g2) {
	// (1/M) sum_m sum_s phi(m,s|g1) phi(m,s|g2) / Z2(s)
	// Z2(s) = sum_m phi(m,s|g2)
	for(signal_t s=0; s<signals; s++) {
		_gws.s[s] = 0;
		for(meaning_t m=0; m<meanings; m++) {
			_gws.s[s] += g2->msp[m*signals+s];
		}
	}
	double ch = 0.0;
	for(signal_t s=0; s<signals; s++) {
		for(meaning_t m=0; m<meanings; m++) {
			ch += g1->msp[m*signals+s] * g2->msp[m*signals+s] / _gws.s[s];
		}
	}
	return ch / (double)meanings;
}

int main(int argc, char* argv[]) {
	// Initialisation
	argp_parse(&argp, argc, argv, 0, 0, 0);
	
	init_globals();
	
	if(! wsinit() ) {
		fprintf(stderr, "Could not allocate grammar workspace\n");
		return 1;
	}
	
	gsl_rng* rng = gsl_rng_alloc( gsl_rng_env_setup() );
	if(!rng) {
		fprintf(stderr, "Could not create rng\n");
		return 1;
	}

	// Create a set of grammars according to desired initial condition
	grammar_t grammars[agents];
	for(unsigned a=0; a<agents; a++) {
		if(! (grammars[a] = ginit()) ) {
			fprintf(stderr, "Could not create grammar\n");
		}
		switch(initial_condition) {
		case ic_random:
	 		randomise_associations(grammars[a], rng);
	 		break;
	 	case ic_identity:
			identity_associations(grammars[a]);
			break;
		case ic_rook:
			rook_associations(grammars[a], rng);
			break;
		default:
			fprintf(stderr, "Unknown initial condition\n");
			return 1;
		}
		renew_pseudo_associations(grammars[a]);
	}
	
	// Print out initial coherence matrix
	printf("coherence agents=%u round=0\n", agents);
	for(unsigned a1=0; a1<agents; a1++) {
		for(unsigned a2=0; a2<agents; a2++) {
			printf("%g ", coherence(grammars[a1], grammars[a2]));
		}
		printf("\n");
	}
	printf("\n");

	unsigned interactions_per_round = agents * meanings * signals;
	
	for(unsigned t=0;t<rounds*interactions_per_round;t++) {

		// Update loop starts here

		// 1. Choose a pair of speakers
		unsigned a1 = gsl_rng_uniform_int(rng, agents);
		unsigned a2 = gsl_rng_uniform_int(rng, agents-1);
		if (a2 >= a1) a2++;
		
		// 2. Speaker selects a topic
		meaning_t m1 = sample_meaning(grammars[a1], rng);
		
		// 3. Speaker produces a signal for m
		signal_t s = produce_signal(grammars[a1], m1, rng);
		
		// 4. Listener infers a meaning
		meaning_t m2 = infer_meaning(grammars[a2], s, rng);
		
		// 5. Obtain feedback
		double fb = feedback > 0.0 ? stochastic_feedback(m1,m2,rng) : deterministic_feedback(m1,m2);
				
		// 6. Update grammars
		vary_association(grammars[a1], m1, s, fb*stepsize );
		vary_association(grammars[a2], m2, s, fb*stepsize );
	
		renew_pseudo_associations(grammars[a1]);
		renew_pseudo_associations(grammars[a2]);

		// Update loop ends here
		
		// Print out the coherence matrix at the end of each rounds		
		if((t+1) % interactions_per_round == 0) {
			printf("coherence agents=%u round=%u\n", agents, (t+1)/interactions_per_round);
			for(unsigned a1=0; a1<agents; a1++) {
				for(unsigned a2=0; a2<agents; a2++) {
					printf("%g ", coherence(grammars[a1], grammars[a2]));
				}
				printf("\n");
			}
			printf("\n");
		}
	}
	
	// Print out grammars at the end
	for(unsigned a=0; a<agents; a++) {
		printf("grammar agent=%u meanings=%u signals=%u round=%u\n", a, meanings, signals, rounds);
		for(meaning_t m=0; m<meanings; m++) {
			for(signal_t s=0;s<signals; s++) {
				printf("%g ", get_association(grammars[a], m, s));
			}
			printf("\n");
		}
		printf("\n");
	}

	// Print out pseudo grammars at the end
	for(unsigned a=0; a<agents; a++) {
		printf("pseudo agent=%u meanings=%u signals=%u round=%u\n", a, meanings, signals, rounds);
		for(meaning_t m=0; m<meanings; m++) {
			for(signal_t s=0;s<signals; s++) {
				printf("%g ", get_pseudo_association(grammars[a], m, s));
			}
			printf("\n");
		}
		printf("\n");
	}

	
	return 0;
}

