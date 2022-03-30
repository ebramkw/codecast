/*
 * Copyright (c) 2011, ETH Zurich.
 * Copyright (c) 2013, Olaf Landsiedel.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Federico Ferrari <ferrari@tik.ee.ethz.ch>
 * Author: Olaf Landsiedel <olafl@chalmers.se>
 *
 */

/**
 *
 * \mainpage
 *           These files document the source code of Chaos, a data collection, aggregation, and dissemination system,
 *           for wireless sensor networks (and IoT, M2M, etc.). Chaos is implemented in
 *           <a href="http://www.sics.se/contiki/">Contiki</a> and bases on Tmote Sky sensor nodes.
 *           It uses the
 *           <a href="http://people.ee.ethz.ch/~ferrarif/sw/glossy/">Glossy</a> source code.
 *
 *           Chaos was published at ACM SenSys'13 in the paper titled
 *           <a href="http://www.cse.chalmers.se/~olafl/papers/2013-11-sensys-landsiedel-chaos.pdf">
 *           Chaos: Versatile and Efficient All-to-All Data Sharing and In-Network Processing at Scale</a>.
 *
 *           This documentation is divided into three main parts:
 *           \li \ref chaos-test "Simple application for testing Chaos":
 *           Example of a simple application that periodically floods a packet and prints related statistics.
 *           \li \ref chaos_interface "Chaos API":
 *           API provided by Chaos for an application that wants to use it.
 *           \li \ref chaos_internal "Chaos internal functions":
 *           Functions used internally by Chaos during a round.
 *
 *           A complete overview of the documentation structure is available <a href="modules.html">here</a>.
 *
 * \author
 *           <a href="http://www.cse.chalmers.se/~olafl/">Olaf Landsiedel</a> <olafl@chalmers.se>
 * \author
 *           <a href="http://www.tik.ee.ethz.ch/~ferrarif">Federico Ferrari</a> <ferrari@tik.ee.ethz.ch>
 *
 */

/**
 * \defgroup chaos-test Simple application for testing Chaos
 *
 *           This application runs Chaos periodically.
 *           A round is started by one node (initiator), the application prints statistics at the end of the round.
 *
 *           The application schedules Chaos periodically with a fixed period \link CHAOS_PERIOD \endlink.
 *
 *           The duration of each Chaos phase is given by \link CHAOS_DURATION \endlink.
 *
 *           During each Chaos phase, the maximum number of transmissions in Chaos (N)
 *           is set to \link N_TX \endlink.
 *
 *           The initiator of the floods is the node having nodeId \link INITIATOR_NODE_ID \endlink.
 *
 *           The packet to be flooded has the format specified by data structure \link chaos_data_struct \endlink.
 *
 *           Receivers synchronize by computing the reference time during each Chaos phase.
 *
 *           To synchronize fast, at startup receivers run Chaos with a significantly shorter period
 *           (\link CHAOS_INIT_PERIOD \endlink) and longer duration (\link CHAOS_INIT_DURATION \endlink).
 *
 *           Receivers exit the bootstrapping phase when they have computed the reference time for
 *           \link CHAOS_BOOTSTRAP_PERIODS \endlink consecutive Chaos phases.
 *
 * @{
 */

/**
 * \file
 *         A simple example of an application that uses Chaos, source file.
 *
 * \author
 *         Olaf Landsiedel <olafl@chalmers.se>
 * \author
 *         Federico Ferrari <ferrari@tik.ee.ethz.ch>
 */

#include "codecast-test.h"
#include "chaos.h"
#include "oofg.h" 

/**
 * \defgroup chaos-test-variables Application variables
 * @{
 */

/**
 * \defgroup chaos-test-variables-sched-sync Scheduling and synchronization variables
 * @{
 */

static chaos_data_struct chaos_data;     /**< \brief Flooding data. */
static struct rtimer rt;                   /**< \brief Rtimer used to schedule Chaos. */
static struct pt pt;                       /**< \brief Protothread used to schedule Chaos. */
static rtimer_clock_t t_ref_l_old = 0;     /**< \brief Reference time computed from the Chaos
                                                phase before the last one. \sa get_t_ref_l */
static uint8_t skew_estimated = 0;         /**< \brief Not zero if the clock skew over a period of length
                                                \link CHAOS_PERIOD \endlink has already been estimated. */
static uint8_t sync_missed = 0;            /**< \brief Current number of consecutive phases without
                                                synchronization (reference time not computed). */
static rtimer_clock_t t_start = 0;         /**< \brief Starting time (low-frequency clock)
                                                of the last Chaos phase. */
static int period_skew = 0;                /**< \brief Current estimation of clock skew over a period
                                                of length \link CHAOS_PERIOD \endlink. */

/** @} */

/**
 * \defgroup chaos-test-variables-stats Statistics variables
 * @{
 */

static unsigned long packets_received = 0; /**< \brief Current number of received packets. */
static unsigned long packets_missed = 0;   /**< \brief Current number of missed packets. */
static unsigned long latency = 0;          /**< \brief Latency of last Chaos phase, in us. */
static unsigned long sum_latency = 0;      /**< \brief Current sum of latencies, in ticks of low-frequency
                                                clock (used to compute average). */

/** @} */

/**
 * \defgroup Mobashir Variables
 * @{
 */

unsigned int arrayIndex; 
unsigned int arrayOffset;
int has_data;

unsigned long curr_slot;
unsigned long slots;
unsigned long ticks;

#define MERGE 1
#define JUST_STARTED 2
int just_started;
unsigned long curr_seq = 1;

/** @} */
/** @} */

/**
 * \defgroup chaos-test-processes Application processes and functions
 * @{
 */

/**
 * \defgroup chaos-test-print-stats Print statistics information
 * @{
 */

PROCESS(chaos_print_stats_process, "Chaos print stats");
PROCESS_THREAD(chaos_print_stats_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		leds_on(LEDS_RED);
		// Print statistics only if Chaos is not still bootstrapping.
		if (!CHAOS_IS_BOOTSTRAPPING()) {
			if (get_rx_cnt()) {	// Packet received at least once.
				// Increment number of successfully received packets.
				packets_received++;
				// Compute latency during last Chaos phase.
				rtimer_clock_t lat = get_t_first_rx_l() - get_t_ref_l();
				// Add last latency to sum of latencies.
				sum_latency += lat;
				// Convert latency to microseconds.
				latency = (unsigned long)(lat) * 1e6 / RTIMER_SECOND;
				// Print information about last packet and related latency.
				printf("seq_no %lu\n", chaos_data.seq_no);
			} else {	// Packet not received.
				// Increment number of missed packets.
				packets_missed++;
				// Print failed reception.
				printf("Chaos NOT received\n");
			}
#if CHAOS_DEBUG
//			printf("skew %ld ppm\n", (long)(period_skew * 1e6) / CHAOS_PERIOD);
			printf("high_T_irq %u, rx_timeout %u, bad_length %u, bad_header %u, bad_crc %u, T_slot_h %u, rc_up %u\n",
					high_T_irq, rx_timeout, bad_length, bad_header, bad_crc, get_T_slot_h(), rc_update);
#endif /* CHAOS_DEBUG */
#if ENERGEST_CONF_ON
			// Compute average radio-on time, in microseconds.
			// unsigned long avg_radio_on = (unsigned long)CHAOS_PERIOD * 1e6 / RTIMER_SECOND *
			// 		(energest_type_time(ENERGEST_TYPE_LISTEN) + energest_type_time(ENERGEST_TYPE_TRANSMIT)) /
			// 		(energest_type_time(ENERGEST_TYPE_CPU) + energest_type_time(ENERGEST_TYPE_LPM));
			// Print information about average radio-on time.
			// printf("average radio-on time %lu.%03lu ms\n",
			// 		avg_radio_on / 1000, avg_radio_on % 1000);
#endif /* ENERGEST_CONF_ON */
		}

#ifdef LOG_TIRQ
		print_tirq();
#endif
#ifdef LOG_FLAGS
		print_flags_tx();
		print_flags_rx();
		print_stats();
		// print_g();
#endif /* LOG_FLAGS */

		// latency
		if(slots==0&&ticks==0) {
			// if couldn't solve completely
			slots = MERGE;
		}
		unsigned long total_ticks = ((3*slots)*(RTIMER_SECOND/2)) + ticks;
		unsigned long total_secs = (total_ticks*1e3)/RTIMER_SECOND;
		printf("Elapsed: %3lu.%03lu secs \n",total_secs/1000, total_secs%1000);

		int i;

		if(1||num_solved<CHAOS_NODES) {
			printf("Solved(%d): ",num_solved);
			for(i=0;i<num_solved;i++) {
				printf("%d ", solved[i]+1);
			}
			printf("\n");
		}

		// rtimer_clock_t emax = 0;
		// rtimer_clock_t dmax = 0;
		// printf("DP1(%d): ",dp_count);
		// for(i=0;i<dp_count;i++) {
		// 	if(decode_time_log[i]>emax)
		// 		dmax = decode_time_log[i];
		// 	printf("%u ",decode_time_log[i]);
		// }
		// printf("\n");
		// printf("DP2(%d): ",dp_count);
		// for(i=0;i<dp_count;i++) {
		// 	if(encode_time_log[i]>emax)
		// 		emax = encode_time_log[i];
		// 	printf("%u ",encode_time_log[i]);
		// }
		// printf("\n");
		// printf("MAX: Encode %u Decode %u Sum %u\n", emax, dmax, emax+dmax);
		// // dp_count = 0;

		// printf("DP(%d): ",dp_count);
		rtimer_clock_t max_ofg = 0;
		for(i=0;i<dp_count;i++) {
			rtimer_clock_t temp_ofg = encode_time_log[i]+decode_time_log[i];
			if(temp_ofg>max_ofg)
				max_ofg = temp_ofg;
			// printf("%u ",temp_ofg);
		}
		// printf("\n");
		printf("max_ofg(%d): %u\n", dp_count, max_ofg);

// NB: tx_to_nbr, rx_from_nbr, rxn_from_nbr are all actual node ID and not

#if RLNC == 1
		printf("tx_activity(%d): ",tx_activity_cnt);
		for(i=0;i<tx_activity_cnt;i++) {
			printf("%d-%d ",tx_activity[i],tx_to_nbr[i][0]);
		}
		printf("\n");
		printf("rxn_activity(%d): ",rxn_activity_cnt);
		for(i=0;i<rxn_activity_cnt;i++) {
			printf("%d-%d ",rxn_activity[i],rxn_from_nbr[i][0]);
		}
		printf("\n");
		printf("rx_activity(%d): ",rx_activity_cnt);
		for(i=0;i<rx_activity_cnt;i++) {
			printf("%d-%d ",rx_activity[i],rx_from_nbr[i][0]);
		}
		printf("\n");
#elif RLNC == 2
		printf("tx_activity(%d): ",tx_activity_cnt);
		for(i=0;i<tx_activity_cnt;i++) {
			printf("%d-%d-%d ",tx_activity[i],tx_to_nbr[i][0],tx_to_nbr[i][1]);
		}
		printf("\n");
		printf("rxn_activity(%d): ",rxn_activity_cnt);
		for(i=0;i<rxn_activity_cnt;i++)
			printf("%d ",rxn_activity[i]);
		printf("\n");
		printf("rx_activity(%d): ",rx_activity_cnt);
		for(i=0;i<rx_activity_cnt;i++)
			printf("%d ",rx_activity[i]);
		printf("\n");
#endif		
		leds_off(LEDS_RED);
	}
	PROCESS_END();
}

/** @} */

/**
 * \defgroup chaos-test-skew Clock skew estimation
 * @{
 */

static inline void estimate_period_skew(void) {
	// Estimate clock skew over a period only if the reference time has been updated.
	if (CHAOS_IS_SYNCED()) {
		// Estimate clock skew based on previous reference time and the Chaos period.
		period_skew = get_t_ref_l() - (t_ref_l_old + (rtimer_clock_t)CHAOS_PERIOD);
		// Update old reference time with the newer one.
		t_ref_l_old = get_t_ref_l();
		// If Chaos is still bootstrapping, count the number of consecutive updates of the reference time.
		if (CHAOS_IS_BOOTSTRAPPING()) {
			// Increment number of consecutive updates of the reference time.
			skew_estimated++;
			// Check if Chaos has exited from bootstrapping.
			if (!CHAOS_IS_BOOTSTRAPPING()) {
				// Chaos has exited from bootstrapping.
				//leds_off(LEDS_RED);
				// Initialize Energest values.
				energest_init();
#if CHAOS_DEBUG
				high_T_irq = 0;
				bad_crc = 0;
				bad_length = 0;
				bad_header = 0;
#endif /* CHAOS_DEBUG */

			}
		}
	}
}

/** @} */

/**
 * \defgroup chaos-test-scheduler Periodic scheduling
 * @{
 */

uint16_t node_index;

static inline void setArrayIndex(void){
	//all flags to zero
	memset(&chaos_data.flags[0], 0, MERGE_LEN * sizeof(uint8_t));
	//all bv to zero
	memset(&chaos_data.bv[0], 0, MERGE_LEN * sizeof(uint8_t));
	//find my index and offset
	arrayIndex = node_index / 8;
	arrayOffset = node_index % 8;
	//set to one at index and offset
  	// chaos_data.bv[arrayIndex] = 1 << arrayOffset;
  	return;
}
	
static inline void setData(){
	//set dummy payload
	uint8_t i;
	for( i=0; i < PAYLOAD_LEN; i++ ){
		chaos_data.payload[i] = (uint8_t) (0x11 * i);
	}
	//set xor payload
	// chaos_data.payload[0] = (uint8_t) X[node_index];
	return;
}

char chaos_scheduler(struct rtimer *t, void *ptr) {
	PT_BEGIN(&pt);
	if (IS_INITIATOR()) {	// Chaos initiator.
		while (1) {
			// Increment sequence number.
			chaos_data.seq_no++;
			curr_seq = chaos_data.seq_no;
			if((curr_seq%MERGE)==0) {
				ofge_init(node_index);
				encode(chaos_data.bv);
			}
			// Initialise packet parameters
			chaos_data.solved = num_solved;
			chaos_data.source = node_id;
			//set my flag to one
			setArrayIndex();
			//set data
			// setData();
			// Chaos phase.
			if((curr_seq%MERGE)==0) {
				leds_on(LEDS_GREEN);
			}
			// Start Chaos.
			just_started = JUST_STARTED;
			chaos_start((uint8_t *)&chaos_data, /*DATA_LEN,*/ CHAOS_INITIATOR, /*CHAOS_SYNC,*/ N_TX);
			// Store time at which Chaos has started.
			t_start = RTIMER_TIME(t);
			// Schedule end of Chaos phase based on CHAOS_DURATION.
			rtimer_set(t, CHAOS_REFERENCE_TIME + CHAOS_DURATION, 1, (rtimer_callback_t)chaos_scheduler, ptr);
			// Yield the protothread.
			PT_YIELD(&pt);

			// Off phase.
			leds_off(LEDS_RED);
			if(((curr_seq+1)%MERGE)==0) {
				leds_off(LEDS_GREEN);
				leds_off(LEDS_BLUE);
			}
			// MM: Increment slot counter
			curr_slot++;
			// Stop Chaos.
			chaos_stop();
			if (!CHAOS_IS_BOOTSTRAPPING()) {
				// Chaos has already successfully bootstrapped.
				if (!CHAOS_IS_SYNCED()) {
					// The reference time was not updated: increment reference time by CHAOS_PERIOD.
					set_t_ref_l(CHAOS_REFERENCE_TIME + CHAOS_PERIOD);
					set_t_ref_l_updated(1);
				}
			}
			// Schedule begin of next Chaos phase based on CHAOS_PERIOD.
			rtimer_set_long(t, CHAOS_REFERENCE_TIME, CHAOS_PERIOD, (rtimer_callback_t)chaos_scheduler, ptr);
			// Estimate the clock skew over the last period.
			estimate_period_skew();
			// Poll the process that prints statistics (will be activated later by Contiki).
			if(0||((curr_seq+1)%MERGE)==0) {
				process_poll(&chaos_print_stats_process);
			}
			// Yield the protothread.
			PT_YIELD(&pt);
		}
	} else {	// Chaos receiver.
		while (1) {
			if(((curr_seq+1)%MERGE)==0) {
				ofge_init(node_index);
			}
			//set my flag to one
			setArrayIndex();
			// Chaos phase.
			if(((curr_seq+1)%MERGE)==0) {
				leds_on(LEDS_GREEN);
			}
			// Start Chaos.
			just_started = JUST_STARTED;
			chaos_start((uint8_t *)&chaos_data, /*DATA_LEN,*/ CHAOS_RECEIVER, /*CHAOS_SYNC,*/ N_TX);
			// Store time at which Chaos has started.
			t_start = RTIMER_TIME(t);
			if (CHAOS_IS_BOOTSTRAPPING()) {
				// Chaos is still bootstrapping:
				// Schedule end of Chaos phase based on CHAOS_INIT_DURATION.
				// MM: Duration+49ms
				rtimer_set(t, RTIMER_TIME(t) + CHAOS_INIT_DURATION, 1,
					(rtimer_callback_t)chaos_scheduler, ptr);
			} else {
				// Chaos has already successfully bootstrapped:
				// Schedule end of Chaos phase based on CHAOS_DURATION.
				// MM: Duration+(1+syncmissed) ms
				rtimer_set(t, RTIMER_TIME(t) + CHAOS_GUARD_TIME * (1 + sync_missed) + CHAOS_DURATION, 1,
					(rtimer_callback_t)chaos_scheduler, ptr);
			}
			// Yield the protothread.
			PT_YIELD(&pt);

			// Off phase.
			leds_off(LEDS_RED);
			if(((curr_seq+1)%MERGE)==0) {
				leds_off(LEDS_GREEN);
				leds_off(LEDS_BLUE);
			}
			// MM: Increment slot counter
			curr_slot++;
			// Stop Chaos.
			chaos_stop();
			if (CHAOS_IS_BOOTSTRAPPING()) {
				// Chaos is still bootstrapping.
				if (!CHAOS_IS_SYNCED()) {
					// The reference time was not updated: reset skew_estimated to zero.
					skew_estimated = 0;
				}
			} else {
				// Chaos has already successfully bootstrapped.
				if (!CHAOS_IS_SYNCED()) {
					// The reference time was not updated:
					// increment reference time by CHAOS_PERIOD + period_skew.
					set_t_ref_l(CHAOS_REFERENCE_TIME + CHAOS_PERIOD + period_skew);
					set_t_ref_l_updated(1);
					// Increment sync_missed.
					sync_missed++;
				} else {
					// The reference time was not updated: reset sync_missed to zero.
					sync_missed = 0;
				}
				// Poll the process that prints statistics (will be activated later by Contiki).
				if(0||((curr_seq+1)%MERGE)==0) {
					process_poll(&chaos_print_stats_process);
				}
			}
			// Estimate the clock skew over the last period.
			estimate_period_skew();
			if (CHAOS_IS_BOOTSTRAPPING()) {
				// Chaos is still bootstrapping.
				if (skew_estimated == 0) {
					// The reference time was not updated:
					// Schedule begin of next Chaos phase based on last begin and CHAOS_INIT_PERIOD.
					// MM: 10 ms
					rtimer_set(t, RTIMER_TIME(t) + CHAOS_INIT_PERIOD - CHAOS_INIT_DURATION, 1,
							(rtimer_callback_t)chaos_scheduler, ptr);
				} else {
					// The reference time was updated:
					// Schedule begin of next Chaos phase based on reference time and CHAOS_INIT_PERIOD.
					// MM: (Period-20) ms
					rtimer_set_long(t, CHAOS_REFERENCE_TIME, CHAOS_PERIOD - CHAOS_INIT_GUARD_TIME,
							(rtimer_callback_t)chaos_scheduler, ptr);
				}
			} else {
				// Chaos has already successfully bootstrapped:
				// Schedule begin of next Chaos phase based on reference time and CHAOS_PERIOD.
				// MM: Period-(1+syncmissed) ms
				rtimer_set_long(t, CHAOS_REFERENCE_TIME, CHAOS_PERIOD +
						period_skew - CHAOS_GUARD_TIME * (1 + sync_missed),
						(rtimer_callback_t)chaos_scheduler, ptr);
			}
			// Yield the protothread.
			PT_YIELD(&pt);
		}
	}
	PT_END(&pt);
}

/** @} */

/**
 * \defgroup chaos-test-init Initialization
 * @{
 */

static uint8_t init_mapping(uint16_t nodeID);

#ifdef NODE_ID_MAPPING

//mapping of node id to flag
static const uint16_t mapping[] = NODE_ID_MAPPING;

//execute mapping of node id to flag
static uint8_t init_mapping(uint16_t nodeID){
	unsigned int i;
	//lookup id
	for( i = 0; i < CHAOS_NODES; i++ ){
		if( nodeID == mapping[i] ){
			node_index = i;
			has_data = 1;
			return 1;
		}
	}
	has_data = 0;
	return 1;
}

#else

//execute mapping of node id to flag
static uint8_t init_mapping(uint16_t nodeID){
	//simple flags: node id minus one
	node_index = nodeID - 1; 
	if(node_index<CHAOS_NODES) {
		has_data = 1;
	} else {
		has_data = 0;
	}
	return 1;
}

#endif

PROCESS(chaos_test, "Chaos test");
AUTOSTART_PROCESSES(&chaos_test);
PROCESS_THREAD(chaos_test, ev, data)
{
	PROCESS_BEGIN();
	
	printf("PKT_LEN %d DATA_LEN %d\n", PACKET_LEN, DATA_LEN);

#ifdef TESTBED
	// wait a few seconds, so we can hopefully read the initial message
	static struct etimer et;
    etimer_set(&et, 10 * CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
#endif /* TESTBED */    

    if (IS_INITIATOR()) {
    	leds_on(LEDS_RED);
    	static struct etimer et;
#ifdef TESTBED
		etimer_set(&et, 120 * CLOCK_SECOND);
#else
		etimer_set(&et, 1 * CLOCK_SECOND);
#endif		
		PROCESS_WAIT_UNTIL(etimer_expired(&et));
		leds_off(LEDS_RED);
    }

	printf("chaos test! tx power: %u, proc cycles: %u, timeouts: %u, max timeout: %u, min timeout: %u, tx on complete: %d, payload: %d, period %lu, duration %lu, node count: %u\n", CC2420_TXPOWER, (uint16_t)PROCESSING_CYCLES, TIMEOUT, MAX_SLOTS_TIMEOUT, MIN_SLOTS_TIMEOUT, N_TX_COMPLETE, PAYLOAD_LEN, CHAOS_PERIOD, CHAOS_DURATION, CHAOS_NODES);
	printf("node_id: %u\n", node_id);

	if (init_mapping(node_id)) {
		// Initialize Chaos data.
		chaos_data.seq_no = 0;
		// Start print stats processes.
		process_start(&chaos_print_stats_process, NULL);
		// Start Chaos busy-waiting process.
		process_start(&chaos_process, NULL);
		// Start Chaos experiment in one second.
		rtimer_set(&rt, RTIMER_NOW() + RTIMER_SECOND, 1, (rtimer_callback_t)chaos_scheduler, NULL);
	}

	PROCESS_END();
}

/** @} */
/** @} */
/** @} */

// uint8_t my_var = 240;						//11110000:240
// printf("my_var: %u\n",my_var);				//11110000:240
// uint8_t var = 170;							//10101010:170
// printf("old_var: %u\n",var);					//10101010:170
// uint8_t new_var = ~var;						//01010101: 85
// printf("inv_var: %u\n",new_var);				//01010101: 85
// new_var = new_var&my_var;					//01010000: 80
// printf("new_var: %u\n",new_var);				//01010000: 80

// while(1) {
// 	printf("%d\n",(((rand()+32768) % 10)==0));
// }