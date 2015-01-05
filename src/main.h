/*!
 * \file main.h
 * \brief Header file to DDoS detection system.
 * \author Jan Neuzil <neuzija1@fit.cvut.cz>
 * \date 2014
 */
/*
 * Copyright (C) 2014 ISEP
 *
 * LICENSE TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#ifndef _MAIN_
#define _MAIN_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>

/*!
 * \name Default values.
 * Defines macros used by DDoS detection program.
 * \{ */
#define VERBOSITY 1 /*!< Default verbosity level. */
#define NUMBER_LEN 5 /*!< Maximal length of number for buffer. */
#define ARRAY_EXTRA 4 /*!< Extra array size for a circle buffer. */
#define PADDING 16 /*!< Padding width for log files. */

#define PROTOCOL_TCP 6 /*!< TCP protocol number. */
#define PROTOCOL_UDP 17 /*!< UDP protocol number. */

#define BUFFER_TMP 256 /*!< Size of a temporary buffer. */
#define BUFFER_SIZE 8192 /*!< Size of a buffer for reading standard input. */

#define PORTS_INIT 8 /*!< Init size of array with network ports. */
#define HOSTS_INIT 32768 /*!< Init size of array with hosts. */

#define ALL_PORTS 65535 /*!< Maximum number of network ports. */

#define BITS_PORT 16 /*!< Number of bits in network port. */
#define MASK_PORT 0x8000 /*!< Mask number for network port. */
#define BITS_IP4 32 /*!< Number of bits in IPv4 address. */
#define MASK_IP4 0x80000000 /*!< Mask number for 32 bit address. */

#define FLUSH_ITER 0 /*!< Default number of iteration after the graph is flushed. */
#define ARRAY_MIN 32 /*!< Minimum number of intervals. */
#define INTERVAL 60 /*!< Default observation interval of SYN packets in seconds. */
#define PORT_WINDOW 300 /*!< Default observation port scan window in seconds before flushing ports. */
#define TIME_WINDOW 3600 /*!< Default observation time window defined in seconds. */

#define CLUSTERS 2 /*!< Default number of clusters to be used in k-means algorithm. */
#define OBSERVATIONS 2 /*!< Default minumum number of observations in the cluster. */
#define square(x) ((x) * (x)) /*!< Square function used in k-means algorithm. */

#define DELIMITER ' ' /*!< Default delimiter for parsing CSV files. */
#define FILE_FORMAT "%Y-%m-%d_%H-%M-%S" /*!< Default file name in time format. */
#define TIME_FORMAT "%a %b %d %Y %H:%M:%S" /*!< Default human readable time format. */
#define DATA_FILE "/tmp/data.txt" /*!< Data file location used by gnuplot.*/
#define GNUPLOT "/tmp/config.gpl" /*!< Gnuplot configuration file location.*/
#define OPTIONS "d:e:f:hHk:L:p:t:w:" /*!< Options for for command line. */
/*! \} */

/*!
 * \brief Detection mode enumeration.
 * Mode type of the DDoS detection.
 */
enum detection_mode {
   MODE_SYN_FLOODING = 0x01, /*!< SYN flooding detection mode. */
   MODE_PORTSCAN_VER = 0x02, /*!< Portscan detection mode. */
   MODE_PORTSCAN_HOR = 0x04, /*!< Portscan detection mode. */
   MODE_ALL = 0x07, /*!< All detection modes. */
};

/*!
 * \brief Verbose level enumeration.
 * Verbose level for printing data graph structure.
 */
enum verbose_level {
   VERBOSE_BRIEF = 1, /*!< Verbose level to print short brief information. */
   VERBOSE_BASIC = 2, /*!< Verbose level to print basic information about number of hosts and create plot of suspicious hosts. */
   VERBOSE_ADVANCED = 3, /*!< Verbose level to print information about every host in the graph. */
   VERBOSE_EXTRA = 4, /*!< Verbose level to print all data of every host, it can consume lot of disk memory. */
   VERBOSE_FULL = 5 /*!< Verbose level to print and translate domain name of hosts. */
};

/*!
 * \brief Examination level enumaration.
 * Level mode of host examination to get more precise data about the host.
 */
enum host_level {
    LEVEL_INFO = 1, /*!< Basic examination level to inspect only briefly the given host. */
    LEVEL_TRACE = 2 /*!< Extra examination level to inspect also the ports of given host. */
};

/*!
 * \brief Binary tree structure.
 * Structure containing pointers to left and right whether the bit
 * is 0 or 1, leafs also contains pointer to host structure.
 */
typedef struct node {
   struct node *left; /*!< Pointer to another node if result is 1. */
   struct node *right; /*!< Pointer to another node if result is 0. */
   void *val; /*!< Pointer to value structure if node is a leaf. */
} node_t;

/*!
 * \brief Interval structure.
 * Structure of interval containing number of SYN packets in the given interval
 * and information of assigned cluster to determine whether the traffic in the given
 * interval is SYN flooding attack or not.
 */
typedef struct intvl {
    //char cluster; /*!< Flag of assigned cluster. */
    double syn_packets; /*!< Number of SYN packets. */
} intvl_t;

/*!
 * \brief Port structure.
 * Structure of ports containing number of the destination port and times accesses
 * to detect if the host was under vertical port scan attack or not.
 */
typedef struct port {
    uint16_t port_num; /*!< Destination port number. */
    uint32_t accesses; /*!< Number of times the given address has been accessed. */
} port_t;

/*!
 * \brief Extra structure.
 * Extra host structure with additional information about the given host such as binary
 * tree of all ports that have been accessed.
 */
typedef struct extra {
   uint16_t ports_cnt; /*!< Number of different ports used to reach the given host. */
   uint16_t ports_max; /*!< Number of different ports used to reach the given host. */
   node_t *root; /*!< Pointer to root of binary tree with all network ports. */
   port_t **ports; /*!< Pointer to array of used network ports structures. */
} extra_t;

/*!
 * \brief Cluster structure.
 * Cluster structure to keep information about number of host in the given structure
 * and centroid coordinates.
 */
typedef struct cluster {
   double dev; /*!< Sums of squared deviations of the cluster. */
   uint64_t hosts_cnt; /*!< Number of hosts in the given cluster. */
   intvl_t *centroid; /*!< Centroid coordinates of the given cluster. */
} cluster_t;

/*!
 * \brief Local host structure.
 * Structure containing information about local host such as IP address and other
 * peers with whom was communicating during the given time period. It also contains
 * information about mutual contacts with other local hosts.
 */
typedef struct host {
   in_addr_t ip; /*!< IP address of the local host. */
   uint8_t stat; /*!< Host status for further examination. */
   uint8_t level; /*!< Host examination level. */
   uint8_t cluster; /*!< Assigned cluster to the host. */
   double distance; /*!< Distance to the nearest centroid. */
   uint32_t accesses; /*!< Number of times the given address has been accessed. */
   intvl_t *intervals; /*!< Array of SYN packets number in the given interval. */
   extra_t *extra; /*!< Pointer to extra information about the host. */
} host_t;

/*!
 * \brief Parameters structure.
 * Structure of parameters containing default or set parameters during initialization
 * of module to be used in following functions.
 */
typedef struct params {
   int mode; /*!< Flag which type of DDoS detection mode should be used. */
   int clusters; /*!< Number of clusters to be used in k-means algorithm. */
   int flush_cnt; /*!< Counter of flush iterations. */
   int flush_iter; /*!< Number of iterations for flushing the graph. */
   int progress; /*!< Parameter for printing dots of received flows. */
   int level; /*!< Verbosity level for printing graph structure. */
   int interval; /*!< Observation interval of SYN packets in seconds. */
   int time_window; /*!< Observation time window in seconds. */
   int intvl_max; /*!< Maximum size of SYN packets array. */
   int iter_max; /*!< Maximum number of intervals before flushing all ports. */
   int window_sum; /*!< Number of reached windows during the runtime. */
   char *file; /*!< CSV file to be processed by the algorithm. */
} params_t;

/*!
 * \brief Flow record structure.
 * Structure containing all fields of a flow record.
 */
typedef struct flow {
    in_addr_t dst_ip; /*!< Destination IP address. */
    in_addr_t src_ip; /*!< Source IP address. */
    uint16_t dst_port; /*!< Destination port. */
    uint16_t src_port; /*!< Source port. */
    uint8_t protocol; /*!< Used protocol. */
    time_t time_first; /*!< Timestamp of the first packet. */
    time_t time_last; /*!< Timestamp of the last packet. */
    uint64_t bytes; /*!< Number of transmitted bytes. */
    uint32_t packets; /*!<  Number of transmitted packets. */
    uint8_t syn_flag; /*!< SYN flag. */
} flow_t;

/*!
 * \brief Graph structure.
 * Structure containing pointers to allocated nodes and hosts in graph scheme
 * to be further examined.
 */
typedef struct graph {
   uint8_t host_level; /*!< Flag to identify host examination level. */
   uint16_t interval_idx; /*!< Index number of given interval. */
   uint16_t interval_cnt; /*!< Number of reached intervals. */
   uint32_t window_cnt; /*!< Number of reached windows before flushing the graph. */
   uint32_t ports[ALL_PORTS]; /*!< Array of all ports and number of accesses in the given interval. */
   time_t interval_first; /*!< Given Unix timestamp of the interval begging. */
   time_t interval_last; /*!< Calculated Unix timestamp of the interval end. */
   time_t window_first; /*!< Given Unix timestamp of the time window begging. */
   time_t window_last; /*!< Calculated Unix timestamp of the time window end. */
   uint64_t hosts_cnt; /*!< Number of hosts determined by destination IP address in graph. */
   uint64_t hosts_max; /*!< Maximum number of hosts in graph. */
   params_t *params; /*!< Pointer to structure with all initialized parameters. */
   node_t *root; /*!< Pointer to root of binary tree with all IPv4 addresses. */
   host_t **hosts; /*!< Pointer to array of host structures. */
   cluster_t **clusters; /*!< Pointer to array of cluster structures. */
} graph_t;

/*!
 * \brief Main function.
 * Main function to parse given arguments and run the DDoS detection system.
 * \param[in] argc Number of given parameters.
 * \param[in] argv Array of given parameters.
 * \return EXIT_SUCCESS on success, otherwise EXIT_FAILURE.
 */
int main(int argc, char **argv);

#endif /* _MAIN_ */