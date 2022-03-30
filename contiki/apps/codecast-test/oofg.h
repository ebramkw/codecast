#include "codecast-test.h"

#define RLNC 1

#define DEBUG 0
#define COL CHAOS_NODES

/* Usage: CHECK_BIT(temp, 3) */
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

/* Global Variables */
extern int solved[COL];
extern int num_solved;
extern int nbr_solved;
extern int nbr_source;
extern int nbr_destination;

/* Generate multiple BV packets */
#define NUM_PKTS 1
extern int decode_count;
extern int curr_degree;

// N: For fast termination
extern int neighbor_complete;
extern int im_complete;
extern int decoded_new;

// M: Neighbor info
extern uint8_t solved_bv[MERGE_LEN];

// MM: Temporary global
extern int missing_pkt;
extern int missing_list[CHAOS_NODES];
extern int missing_list_ptr[CHAOS_NODES];
extern int missing_list_count;

// MM: Nbr solved list
#define NBR_LIST 2
extern uint8_t nbr_solved_bv_list[NBR_LIST][MERGE_LEN];
extern int nbr_solved_list[NBR_LIST];
extern int nbr_common_list[NBR_LIST];
extern int nbr_source_list[NBR_LIST];
extern int nbr_relay_list[NBR_LIST];
extern int nbr_count;

/* Function declarations */
void encode(uint8_t*);							// Encoder for OFGE
int decode(uint8_t*, uint8_t*);					// Decoder for OFGE
void substitution(uint8_t, uint8_t*);			// Performs backward/forward substition
int find_min(int*);								// Returns minimum element in array
int recv_degree();								// Return degree based on how much neighbors have solved	
uint8_t leftmost_one(uint8_t*);					// Returns index of leftmost one
uint8_t degree_eq(uint8_t*);					// Returns degree of given equation
void xor_bv_matrix(uint8_t*, uint8_t);			// XOR: bv = bv^row
void xor_matrix_bv(uint8_t, uint8_t*);			// XOR: row = row^eq
void xor_matrix_matrix(uint8_t, uint8_t);		// XOR: row = row^row
void swap_eq(uint8_t*, uint8_t);				// SWAP: Two equations
void copy_eq(uint8_t*, uint8_t);				// COPY: Matrix row to Equation
void print_uint8_t(uint8_t);					// PRINT: Element of matrix
void print_g();									// PRINT: Matrix
void ofge_init();								// Initialize global variables

// TX Condition
extern int tx_cond[2][2][2];

// Slot Activity:
#define ACTIVITY_LOG_SIZE 200
extern int curr_relay;
extern int tx_activity[ACTIVITY_LOG_SIZE];
extern int tx_activity_cnt;
extern int rx_activity[ACTIVITY_LOG_SIZE];
extern int rx_activity_cnt;
extern int rxn_activity[ACTIVITY_LOG_SIZE];
extern int rxn_activity_cnt;

// Log Encoding/Decoding Time
extern rtimer_clock_t encode_time_log[ACTIVITY_LOG_SIZE];
extern rtimer_clock_t decode_time_log[ACTIVITY_LOG_SIZE];
extern rtimer_clock_t encode_time;
extern rtimer_clock_t decode_time;
extern int dp_count;

// Log help info
extern int tx_to_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];
extern int rx_from_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];
extern int rxn_from_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];