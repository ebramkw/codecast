#include <stdio.h>
#include <stdlib.h>
#include "oofg.h"
#include "codecast-test.h"

/* GX = Y */
uint8_t G[COL][MERGE_LEN];

uint8_t pkt_decoded[COL];		// Bit Vector indicating which packet is decoded
int solved[COL];				// List storing the id of the packets decoded already
int num_solved;					// Storing the count of the packets decoded
int nbr_solved;
int nbr_source;
int nbr_destination;

int empty_rows;
int num_ones[COL];

// MM: Nbr solved list
uint8_t nbr_solved_bv_list[NBR_LIST][MERGE_LEN];
int nbr_solved_list[NBR_LIST];
int nbr_common_list[NBR_LIST];
int nbr_source_list[NBR_LIST];
int nbr_relay_list[NBR_LIST];
int nbr_count;
int nbr_list_full;

// MM: Contains the count of latest decoded packets
int decode_count;
int missing_count;
int curr_degree;

// MM: Temporary global
int missing_pkt = -1;
int missing_list[CHAOS_NODES];
int missing_list_ptr[CHAOS_NODES];
int missing_list_count = 0;

// MM: All my neighbors solved, so don't transmit
int neighbor_complete;
int im_complete;
int decoded_new;

// MM: Log distribution
int encode_counter[COL];

// M: Neighbor info
uint8_t solved_bv[MERGE_LEN];

// M: Soliton Distribution
uint8_t robustsoliton[] = {85, 93, 96, 98, 99, 100}; //C=0.9 D=0.1
int robustsoliton_len = 6;
// uint8_t soliton[] = {5, 52, 68, 77, 83, 87, 89, 91, 93, 94, 95, 96, 97, 98, 99, 100};
// int soliton_len = 16;

// M: Original Growth Code
int origGC[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 5, 6, 7, 10, 15, 15, 15};//Original

// M: New Growth Code
// uint8_t newGC[][31] = { 
//  {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  2,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  2,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  2,  3,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  2,  3,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  2,  2,  4,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  2,  3,  4,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  2,  2,  3,  5, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  2,  2,  3,  5, 11,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  2,  2,  3,  4,  6, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  2,  2,  3,  4,  6, 13,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  4,  7, 14,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  3,  3,  5,  7, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  4,  5,  8, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  4,  5,  8, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  3,  4,  6,  9, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  3,  4,  6,  9, 19,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  4,  5,  6, 10, 20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  3,  3,  4,  5,  7, 10, 21,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  4,  5,  7, 11, 22,  0,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  4,  5,  7, 11, 23,  0,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  4,  4,  6,  8, 12, 24,  0,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  4,  5,  6,  8, 12, 25,  0,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  4,  5,  6,  8, 13, 26,  0,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  4,  5,  6,  9, 13, 27,  0,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  4,  4,  5,  7,  9, 14, 28,  0,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  4,  4,  5,  7,  9, 14, 29,  0,  0}, 
//  {1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  4,  5,  6,  7, 10, 15, 30,  0}
// };
int8_t newGC[][31] = { 
	{0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1,	 1},
	{1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2},
	{1,	1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2,	 2},
	{1,	1,	1,	2,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3},
	{1,	1,	1,	1,	2,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3,	 3},
	{1,	1,	1,	1,	2,	2,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4},
	{1,	1,	1,	1,	1,	2,	3,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4,	 4},
	{1,	1,	1,	1,	1,	2,	2,	3,	5,	5,	5,	5,	5,	5,	5,	5,	5,	5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5},
	{1,	1,	1,	1,	1,	1,	2,	2,	3,	5,	5,	5,	5,	5,	5,	5,	5,	5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5,	 5},
	{1,	1,	1,	1,	1,	1,	2,	2,	3,	4,	6,	6,	6,	6,	6,	6,	6,	6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6},
	{1,	1,	1,	1,	1,	1,	1,	2,	2,	3,	4,	6,	6,	6,	6,	6,	6,	6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6,	 6},
	{1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	4,	7,	7,	7,	7,	7,	7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7},
	{1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	3,	3,	5,	7,	7,	7,	7,	7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7,	 7},
	{1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	4,	5,	8,	8,	8,	8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	4,	5,	8,	8,	8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8,	 8},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	3,	4,	6,	9,	9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	3,	4,	6,	9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9,	 9},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	3,	4,	5,	6,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	3,	3,	4,	5,	 7,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10,	10},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	3,	3,	4,	 5,	 7,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	3,	3,	 4,	 5,	 7,	11,	11,	11,	11,	11,	11,	11,	11,	11,	11},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	3,	3,	 4,	 4,	 6,	 8,	12,	12,	12,	12,	12,	12,	12,	12,	12},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	3,	 3,	 4,	 5,	 6,	 8,	12,	12,	12,	12,	12,	12,	12,	12},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	2,	 3,	 3,	 4,	 5,	 6,	 8,	13,	13,	13,	13,	13,	13,	13},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	 3,	 3,	 3,	 4,	 5,	 6,	 9,	13,	13,	13,	13,	13,	13},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	2,	 2,	 3,	 3,	 4,	 4,	 5,	 7,	 9,	14,	14,	14,	14,	14},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	 2,	 2,	 3,	 3,	 4,	 4,	 5,	 7,	 9,	14,	14,	14,	14},
	{1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	2,	2,	2,	 2,	 2,	 3,	 3,	 3,	 4,	 5,	 6,	 7,	10,	15,	15,	15}
};


// TX Condition
//					   { P, 1, P, 1, P, X, P, X };	
int tx_cond[2][2][2] = { 0, 1, 1, 1, 1, 1, 0, 1 };

// Slot Activity:
int curr_relay;
int tx_activity[ACTIVITY_LOG_SIZE];
int tx_activity_cnt;
int rx_activity[ACTIVITY_LOG_SIZE];
int rx_activity_cnt;
int rxn_activity[ACTIVITY_LOG_SIZE];
int rxn_activity_cnt;

// Log Encoding/Decoding Time
rtimer_clock_t encode_time_log[ACTIVITY_LOG_SIZE];
rtimer_clock_t decode_time_log[ACTIVITY_LOG_SIZE];
rtimer_clock_t encode_time;
rtimer_clock_t decode_time;
int dp_count;

// Log help info
int tx_to_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];
int rx_from_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];
int rxn_from_nbr[ACTIVITY_LOG_SIZE][NBR_LIST];

static inline void set_bit(uint8_t *x, int bitNum) {
	// Set the bit
    *x |= (1 << bitNum);
}

static inline void clear_bit(uint8_t *x, int bitNum) {
	// Clear the bit
    *x &= ~(1 << bitNum);
}

uint8_t m_bv[NBR_LIST][MERGE_LEN];
int m_bv_src[NBR_LIST];
int m_bv_cnt;
int m_bv_full;

void encode(uint8_t* bv) {

	// If nothing solved so far, relay whatever is received
	// This would be realised for only the relay nodes initially
	// Others have at least one to begin with
	if(num_solved==0)	{
		return;
	}

	// Get the intersection between us: solved_bv and pkt_decoded[]
	// Also compute missing nodes but transmitting just one and removing rest
	int i;
	int intersection = 0;
	memset(missing_list_ptr, 0, CHAOS_NODES * sizeof(int));
	missing_list_count = 0;
	for(i=0;i<CHAOS_NODES;i++) {
		int index = i/8;
		if( index >= MERGE_LEN ){
			//out of bound error -> bail out
			continue;
		}
		int offset = i%8;
		// ebramkw: fixed bug in original codecast code
		// using array pointer instead of array values
		// pkt_decoded ==> pkt_decoded[i]
		// if(pkt_decoded) {
		if(pkt_decoded[i]) {
			if(CHECK_BIT(solved_bv[index],offset)) {
				intersection++;
			} else {
				missing_list[missing_list_count++] = i;
				missing_list_ptr[i] = 1;
			}
		}
	}

	// Log the info shared by nbr in current packet
	nbr_solved_list[nbr_count] = nbr_solved;
	nbr_common_list[nbr_count] = intersection;
	nbr_source_list[nbr_count] = nbr_source;
	nbr_count = (nbr_count+1)%NBR_LIST;

	// MM: Process in accordance to the node lagging behind
	int min_nbr_common = find_min(nbr_common_list);
	int min_nbr_solved = find_min(nbr_solved_list);

	// MM: Don't transmit since my neighbors are done
	neighbor_complete = (min_nbr_solved == CHAOS_NODES);

	// Clear bv
	memset(bv, 0, MERGE_LEN * sizeof(uint8_t));

#if 1
	// Generate a temporary packet list and initialize it
	int temp_solved[COL];
	int temp_num_solved = 0;
	for(i=0;i<num_solved;i++) {
		int curr_pkt = solved[i];
		if(!missing_list_ptr[curr_pkt]) {
			temp_solved[temp_num_solved++] = curr_pkt;
		}
	}
#else
	// Do this when GC needs to be tested alone - Comment above
	// Generate a temporary packet list and initialize it
    int temp_solved[COL];
    int temp_num_solved;
    memcpy(temp_solved, solved, COL*sizeof(int));
    temp_num_solved = num_solved;
    missing_list_count = 0;
#endif

	// My Growth Code Degree

	// ebramkw: fixing a bug when index is out of range for newGC
	if (num_solved > 30)
		num_solved = 30;
	if (min_nbr_common > 30)
		min_nbr_common = 30;

	int degree = (int)newGC[num_solved][min_nbr_common];
	if (degree > temp_num_solved) {
		degree = temp_num_solved;
	}
	degree = degree==0?1:degree;
	curr_degree = degree;

	// Soliton Degree
	// int degree;
	// int random_num = ((rand()+32768) % 100);
	// for(degree=0;degree<robustsoliton_len;degree++) {
	// 	if(robustsoliton[degree]>random_num)
	// 		break;
	// }
	// degree++;
	// if (degree > temp_num_solved) {
	// 	degree = temp_num_solved;
	// }
	// // degree = degree==0?1:degree;
	// curr_degree = degree;

	// Orig Growth Code Degree
	// int degree = origGC[min_nbr_solved];
	// if (degree > temp_num_solved) {
	// 	degree = temp_num_solved;
	// }
	// degree = degree==0?1:degree;
	// curr_degree = degree;

	// Number of latest decoded packets to be added	
	decode_count = degree / 3;

	// This wont be modified in case I didn't make use of feedback
	nbr_destination = 0;

	// Start encoding packets
	int temp_solved_index;
	int encode_id;
	while(degree) {
		if(missing_list_count) {
			// Computing missing nodes but transmitting just one and removing rest
			// Pick a random missing packet to add to bv
			temp_solved_index = ((rand()+32768) % missing_list_count);
			// Get the packet number
			encode_id = missing_list[temp_solved_index];
			// Add the packet number to maintain distribution
			encode_counter[encode_id]++;
			// Compute and set the bv
			int index = encode_id/8;
			if( index >= MERGE_LEN ){
				//out of bound error -> bail out
				continue;
			}
			int offset = encode_id%8;
			bv[index] |= (1 << offset);
			//Added one missing packet, unset the flag
			missing_list_count = 0;
			//Add the nbr to which the packet was transmitted
			nbr_destination = nbr_source;
#if 0			
			// Break if only feedback to be tested
			break;
#endif			
		} else if (decode_count) {
			// Get the latest packet's number
			encode_id = temp_solved[temp_num_solved-1];
			// Add the packet number to maintain distribution
			encode_counter[encode_id]++;
			// Compute and set the bv
			int index = encode_id/8;
			if( index >= MERGE_LEN ){
				//out of bound error -> bail out
				continue;
			}
			int offset = encode_id%8;
			bv[index] |= (1 << offset);
			// Delete the packet we just XORed
			temp_num_solved--;
			// Reduce the decode counter by one to add other latest packets
			decode_count--;
		} else {
	 		// Pick a random missing packet to add to bv
			temp_solved_index = ((rand()+32768) % temp_num_solved);
			// Get the packet number
			encode_id = temp_solved[temp_solved_index];
			// Add the packet number to maintain distribution
			encode_counter[encode_id]++;
			// Compute and set the bv
			int index = encode_id/8;
			if( index >= MERGE_LEN ){
				//out of bound error -> bail out
				continue;
			}
			int offset = encode_id%8;
			bv[index] |= (1 << offset);
			// Delete the packet we just XORed and replace it with the last packet in the array
			temp_solved[temp_solved_index] = temp_solved[temp_num_solved-1];
			temp_num_solved--;
		}
		degree--;
	}

	// Add logs regarding who was it transmitted for
	tx_to_nbr[tx_activity_cnt][0] = nbr_destination;

	// Update the solved bv to the transmitted packet
	memset(solved_bv, 0, MERGE_LEN * sizeof(uint8_t));
	for(i=0;i<num_solved;i++) {
		int index = solved[i]/8;
		if( index >= MERGE_LEN ){
			//out of bound error -> bail out
			continue;
		}
		int offset = solved[i]%8;
		solved_bv[index] |= (1 << offset);
	}
}

int decode(uint8_t* bv, uint8_t* solved_bv) {

	/* local variables */
	int i;							// Counter
	uint8_t s, s_index, s_offset;	// Index of the leftmost 1
	int eq_ones;					// Degree of the current equation
	int prev_solved = num_solved;	// Num solved until now

	// Stores the number of packets decoded now
	decode_count = 0;

	// Optimization
	for(i=0;i<COL;i++) {
		if(pkt_decoded[i]) {
	     bv[i/8] &= ~(1 << (i%8));
		}
	}

	// Get the index and offset of leftmost one
	s = leftmost_one(bv);s_index = s/8;s_offset = s%8;
	
	// Find the degree of new equation
	eq_ones = degree_eq(bv);

	while((eq_ones > 0) && CHECK_BIT(G[s][s_index],s_offset)) {
		if(eq_ones >= num_ones[s]) {
			xor_bv_matrix(bv,s);
		} else {			
			swap_eq(bv, s);
			num_ones[s] = eq_ones;
			substitution(s, bv);
		}
		// Get the index and offset of leftmost one
		s = leftmost_one(bv);s_index = s/8;s_offset = s%8;
		// Find the degree of new equation
		eq_ones = degree_eq(bv);
	}
	if(eq_ones > 0) {
		copy_eq(bv, s);
		num_ones[s] = eq_ones;
		empty_rows--;
		substitution(s, bv);
	}

	// Set decode_count, pkt_decoded, solved, num_solved, decoded_new
	decoded_new = 0;
	for(i=0;i<COL;i++) {
		if((!pkt_decoded[i])&&(num_ones[i]==1)) {
			decode_count++;
			pkt_decoded[i] = 1;
			solved[num_solved++] = i;
			decoded_new = 1;
		}
	}

	if(decoded_new) {
		if(rxn_activity_cnt<ACTIVITY_LOG_SIZE) {
			rxn_from_nbr[rxn_activity_cnt][0] = nbr_destination;
			rxn_activity[rxn_activity_cnt++] = curr_relay;
		}
	}
	else {
		if(rx_activity_cnt<ACTIVITY_LOG_SIZE) {
			rx_from_nbr[rx_activity_cnt][0] = nbr_destination;
			rx_activity[rx_activity_cnt++] = curr_relay;
		}
	}

	// Return 1 if we have solved
	// if(num_solved == CHAOS_NODES) {
	// 	return 1;
	// }
	// 
	// return 0;

	return (num_solved == CHAOS_NODES);
}

void substitution(uint8_t s, uint8_t* bv) {
	uint8_t j, k;
	uint8_t s_index = s/8;
	uint8_t s_offset = s%8;
	for(j=0;j<s;j++) {
		if(CHECK_BIT(G[j][s_index],s_offset)) {
			xor_matrix_bv(j,bv);
			num_ones[j] = degree_eq(G[j]);
		}
	}
	for(j=s+1;j<COL;j++) {
		for(k=0;k<j;k++) {
			if(CHECK_BIT(G[j][j/8],j%8) && CHECK_BIT(G[k][j/8],j%8)) {
				xor_matrix_matrix(k,j);
				num_ones[k] = degree_eq(G[k]);
			}
		}
	}
}

int find_min(int list[]) {
	int i;
	int min_solved = 100;
	for(i=0;i<NBR_LIST;i++)
		if(nbr_solved_list[i]<min_solved)
			min_solved = nbr_solved_list[i];
	if(min_solved==100)
		return 0;	
	return min_solved;
}

uint8_t leftmost_one(uint8_t* bv) {
	int i, j;
	for (i=0;i<MERGE_LEN;i++) {
		for (j=0;j<8;j++) {
			if(((i*8)+j)<CHAOS_NODES && CHECK_BIT(bv[i],j)) {
				return ((i*8)+j);
			}
		}
	}
	return 0;
}

uint8_t degree_eq(uint8_t* bv) {
	int i;
	uint8_t c, v, degree = 0;
	for (i=0;i<MERGE_LEN;i++) {
		v = bv[i];
		for (c = 0; v; v >>= 1)
			c += v & 1;
		degree += c;
	}
	return degree;	
}

void xor_bv_matrix(uint8_t* bv, uint8_t s) {
	int i;
	for(i=0;i<MERGE_LEN;i++) {
		bv[i] ^= G[s][i]; 
	}
}

void xor_matrix_bv(uint8_t s, uint8_t* bv) {
	int i;
	for(i=0;i<MERGE_LEN;i++) {
		G[s][i] ^= bv[i]; 
	}
}

void xor_matrix_matrix(uint8_t s, uint8_t r) {
	int i;
	for(i=0;i<MERGE_LEN;i++) {
		G[s][i] ^= G[r][i]; 
	}
}

void swap_eq(uint8_t* bv, uint8_t s) {
	int i;
	for(i=0;i<MERGE_LEN;i++) {
		uint8_t temp = bv[i];
		bv[i] = G[s][i];
		G[s][i] = temp;
	}
}

void copy_eq(uint8_t* bv, uint8_t s) {
	int i;
	for(i=0;i<MERGE_LEN;i++) {
		G[s][i] = bv[i];
	}
}

void print_uint8_t(uint8_t bv) {
	uint8_t i;
	for(i=0;i<8;i++) {
		if(CHECK_BIT(bv,i))
			printf("1");
		else
			printf("0");
	}
}

void print_g() {
	
	#if DEBUG
	int i;
	#endif

	#if DEBUG
	printf("Printing G\n");
	for(i=0;i<COL;i++) {
		for(j=0;j<MERGE_LEN;j++) {
			print_uint8_t(G[i][j]);
		}
		printf("\n");
	}
	#endif

	#if DEBUG
	printf("NumOnes: ");
	for(i=0;i<COL;i++) {
		printf("%d ",num_ones[i]);
	}
	printf("\n");
	#endif

	#if DEBUG
	Print distribution
	printf("Distribution: ");
	for(i=0;i<COL;i++) {
		printf("%d ", encode_counter[i]);
	}
	printf("\n");
	#endif

	#if DEBUG
	printf("NbrList(%d): ", NBR_LIST);
	for(i=0;i<NBR_LIST;i++) {
		printf("%d ", nbr_solved_list[i]);
	}
	printf("\n");
	#endif
}

void ofge_init(int node_index) {
	int i, j;
	empty_rows = COL;
	num_solved = 0;
	for(i=0;i<COL;i++) {
		solved[i] = 0;
		num_ones[i] = 0;
		pkt_decoded[i] = 0;
		for(j=0;j<MERGE_LEN;j++) {
			G[i][j] = 0;
		}
	}
	// Initialise the following only if I am having data
	// Other relay nodes dont have node_index initialised
	if(has_data) {
		empty_rows--;
		solved[num_solved++] = node_index;	
		num_ones[node_index] = 1;
		pkt_decoded[node_index] = 1;
		G[node_index][node_index/8] |= (1 << (node_index%8));
	}

	nbr_solved = 0;
	nbr_count = 0;
	nbr_list_full = 0;
	//memset(nbr_solved_list,0,sizeof(nbr_solved_list));
	for(i=0;i<NBR_LIST;i++) {
		nbr_solved_list[i] = 100;
	}

	// To compute latency keep track of slot number
	curr_slot = 0;
	slots = 0;
	ticks = 0;

	// Flush encode counter
	//memset(encode_counter,0,sizeof(encode_counter));
	for(i=0;i<COL;i++)
		encode_counter[i] = 0;

	neighbor_complete = 0;
	im_complete = 0;
	decoded_new = 0;

	// Processing log counter
	dp_count = 0;

	// Activity counter
	//memset(tx_activity,0,sizeof(tx_activity));
	//memset(rx_activity,0,sizeof(rx_activity));
	//memset(rxn_activity,0,sizeof(rxn_activity));
	tx_activity_cnt = 0;
	rx_activity_cnt = 0;
	rxn_activity_cnt = 0;
}
