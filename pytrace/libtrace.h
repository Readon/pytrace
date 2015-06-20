/*
 * This file is part of libtrace
 *
 * Copyright (c) 2007,2008,2009,2010 The University of Waikato, Hamilton, 
 * New Zealand.
 *
 * Authors: Daniel Lawson 
 *          Perry Lorier
 *          Shane Alcock 
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * libtrace is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libtrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libtrace; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */

/** @file
 *
 * @brief Trace file processing library header
 *
 * @author Daniel Lawson
 * @author Perry Lorier
 * @author Shane Alcock
 *
 * @version $Id$
 *
 * This library provides a per packet interface into a trace file, or a live 
 * captures.  It supports ERF, DAG cards, PCAP, Linux and BSD native sockets,
 * legacy ERF formats etc.
 *
 * @par Usage
 * See the example/ directory in the source distribution for some simple 
 * examples
 *
 * @par Linking
 * To use this library you need to link against libtrace by passing -ltrace
 * to your linker. You may also need to link against a version of libpcap 
 * and of zlib which are compiled for largefile support (if you wish to access
 * traces larger than 2 GB). This is left as an exercise for the reader. Debian
 * Woody, at least, does not support large file offsets.
 *
 */

/** Opaque structure holding information about an output trace */
typedef struct libtrace_out_t libtrace_out_t;
	
/** Opaque structure holding information about a trace */
typedef struct libtrace_t libtrace_t;
        
/** Opaque structure holding information about a bpf filter */
typedef struct libtrace_filter_t libtrace_filter_t;

/** If the packet has allocated its own memory the buffer_control should be
 * set to TRACE_CTRL_PACKET, so that the memory will be freed when the packet
 * is destroyed. If the packet has been zerocopied out of memory owned by
 * something else, e.g. a DAG card, it should be TRACE_CTRL_EXTERNAL.
 *
 * @note The letters p and e are magic numbers used to detect if the packet
 * wasn't created properly.
 */
typedef enum {
	TRACE_CTRL_PACKET='p',  /**< Buffer memory is owned by the packet */
	TRACE_CTRL_EXTERNAL='e' /**< Buffer memory is owned by an external source */
} buf_control_t;

/** The size of a packet's buffer when managed by libtrace */
#define LIBTRACE_PACKET_BUFSIZE 65536

/** Libtrace error information */
typedef struct trace_err_t{
	int err_num; 		/**< error code */
	char problem[255];	/**< the format, uri etc that caused the error for reporting purposes */
} libtrace_err_t;

/** Enumeration of error codes */
enum {
	/** No Error has occured.... yet. */
	TRACE_ERR_NOERROR 	= 0,
	/** The URI passed to trace_create() is unsupported, or badly formed */
	TRACE_ERR_BAD_FORMAT 	= -1,
	/** The trace failed to initialise */
	TRACE_ERR_INIT_FAILED 	= -2,
	/** Unknown config option */
	TRACE_ERR_UNKNOWN_OPTION= -3,
	/** This output uri cannot write packets of this type */
	TRACE_ERR_NO_CONVERSION = -4,
	/** This packet is corrupt, or unusable for the action required */ 
	TRACE_ERR_BAD_PACKET	= -5,
	/** Option known, but unsupported by this format */
	TRACE_ERR_OPTION_UNAVAIL= -6,
	/** This feature is unsupported */
	TRACE_ERR_UNSUPPORTED	= -7,
	/** Illegal use of the API */
	TRACE_ERR_BAD_STATE	= -8,
	/** Failed to compile a BPF filter */
	TRACE_ERR_BAD_FILTER    = -9,
	/** RT communication breakdown */
	TRACE_ERR_RT_FAILURE    = -10,
	/** Compression format unsupported */
	TRACE_ERR_UNSUPPORTED_COMPRESS 	= -11
};

/** Enumeration of DLTs supported by libtrace 
 */
typedef enum {
	/* Special value used to indicate a failure to convert to libtrace
         * DLT */
        TRACE_DLT_ERROR = -1,
	
	/** pcap documents this as having the Address Family value in host byte order as the 
 	  * framing.  Ugly? Yes.
 	  */
	TRACE_DLT_NULL = 0, 
	TRACE_DLT_EN10MB = 1,
	TRACE_DLT_PPP = 9,
	TRACE_DLT_ATM_RFC1483 = 11,
	
	/** Ok, so OpenBSD has a different value for DLT_RAW as the rest of the planet, so detect
          * this.  When reading to/from files we should be using TRACE_DLT_LINKTYPE_RAW instead.
          * When talking about DLT's inside libtrace tho, we should be using /these/ DLT's.
          */
	TRACE_DLT_RAW = 12,
	TRACE_DLT_OPENBSD_LOOP = 108,
	TRACE_DLT_PPP_SERIAL = 50,
	TRACE_DLT_LINKTYPE_RAW = 101, /**< See TRACE_DLT_RAW for explainations of pain. */
	TRACE_DLT_C_HDLC = 104,
	TRACE_DLT_IEEE802_11 = 105,
	TRACE_DLT_LINUX_SLL = 113,
	TRACE_DLT_PFLOG = 117,
	TRACE_DLT_IEEE802_11_RADIO = 127 /**< Radiotap */
} libtrace_dlt_t ;

/** Enumeration of link layer types supported by libtrace */
typedef enum { 
       TRACE_TYPE_UNKNOWN = -1,		/**< Unable to determine link type */
    /* TRACE_TYPE_LEGACY = 0 		Obsolete */
       TRACE_TYPE_HDLC_POS = 1, 	/**< HDLC over POS */
       TRACE_TYPE_ETH = 2,		/**< 802.3 style Ethernet */
       TRACE_TYPE_ATM = 3,		/**< ATM frame */
       TRACE_TYPE_80211 = 4,		/**< 802.11 frames */
       TRACE_TYPE_NONE = 5,		/**< Raw IP frames */
       TRACE_TYPE_LINUX_SLL = 6,	/**< Linux "null" framing */
       TRACE_TYPE_PFLOG = 7,		/**< FreeBSD's PFlog */
    /* TRACE_TYPE_LEGACY_DEFAULT	Obsolete */
       TRACE_TYPE_POS = 9,		/**< Packet-over-SONET */
    /* TRACE_TYPE_LEGACY_ATM 		Obsolete */
    /* TRACE_TYPE_LEGACY_ETH 		Obsolete */
       TRACE_TYPE_80211_PRISM = 12,	/**< 802.11 Prism frames */
       TRACE_TYPE_AAL5 = 13,		/**< ATM Adaptation Layer 5 frames */
       TRACE_TYPE_DUCK = 14,	     /**< Pseudo link layer for DUCK packets */
       TRACE_TYPE_80211_RADIO = 15,  /**< Radiotap + 802.11 */
       TRACE_TYPE_LLCSNAP = 16,      /**< Raw LLC/SNAP */
       TRACE_TYPE_PPP = 17,	     /**< PPP frames */
       TRACE_TYPE_METADATA = 18,     	/**< WDCAP-style meta-data */
       TRACE_TYPE_NONDATA = 19,		/**< Not a data packet */
       TRACE_TYPE_OPENBSD_LOOP = 20	/**< OpenBSD loopback */
} libtrace_linktype_t;

/** RT protocol base format identifiers.
 * This is used to describe the capture format of the packet is being sent 
 * using the RT protocol.
 */ 
enum base_format_t {
        TRACE_FORMAT_ERF          =1,	/**< ERF (DAG capture format) */
        TRACE_FORMAT_PCAP         =2,	/**< Live PCAP capture */
        TRACE_FORMAT_PCAPFILE     =3,	/**< PCAP trace file */ 
        TRACE_FORMAT_WAG          =4,	/**< WAG live capture (Obsolete) */
        TRACE_FORMAT_RT           =5,	/**< RT network protocol */
        TRACE_FORMAT_LEGACY_ATM   =6,	/**< Legacy ERF for ATM capture */
	TRACE_FORMAT_LEGACY_POS	  =7,	/**< Legacy ERF for POS capture */
	TRACE_FORMAT_LEGACY_ETH   =8,	/**< Legacy ERF for ETH capture */
	TRACE_FORMAT_LINUX_NATIVE =9,	/**< Linux native interface capture */
	TRACE_FORMAT_DUCK	  =10,	/**< DAG Clock information */
	TRACE_FORMAT_BPF	  =11,	/**< BSD native interface capture */
	TRACE_FORMAT_TSH	  =12,	/**< TSH trace format */
	TRACE_FORMAT_ATMHDR	  =13,	/**< Legacy ATM header capture */
	TRACE_FORMAT_LEGACY_NZIX  =14,	/**< Legacy format used for NZIX traces */
	TRACE_FORMAT_LINUX_RING	  =15,	/**< Linux native interface capture PACKET_MMAP */
	TRACE_FORMAT_RAWERF	  =16,	/**< Special format for reading uncompressed ERF traces without checking for compression */
    TRACE_FORMAT_DPDK     =17, /**< The Intel Data Plane Development Kit format */
};

/** RT protocol packet types */
typedef enum {
	TRACE_RT_HELLO       	=1, /**< Connection accepted */
	TRACE_RT_START		=2, /**< Request for data transmission to begin 
				    */
	TRACE_RT_ACK		=3, /**< Data acknowledgement */
	TRACE_RT_STATUS		=4, /**< Fifo status packet */
	TRACE_RT_DUCK		=5, /**< Dag duck info packet */
	TRACE_RT_END_DATA	=6, /**< Server is exiting message */
	TRACE_RT_CLOSE		=7, /**< Client is exiting message */
	TRACE_RT_DENY_CONN	=8, /**< Connection has been denied */
	TRACE_RT_PAUSE		=9, /**< Request server to suspend sending data 
				     */
	TRACE_RT_PAUSE_ACK	=10,/**< Server is paused message */
	TRACE_RT_OPTION	 	=11,/**< Option request */
	TRACE_RT_KEYCHANGE	=12,/**< Anonymisation key has changed */ 
	TRACE_RT_DUCK_2_4	=13,/**< Dag 2.4 Duck */
	TRACE_RT_DUCK_2_5 	=14,/**< Dag 2.5 Duck */
	TRACE_RT_LOSTCONN 	=15,/**< Lost connection to server */
	TRACE_RT_SERVERSTART	=16,/**< Reliable server has been restarted */
	TRACE_RT_CLIENTDROP	=17,/**< Reliable client was lost */
	TRACE_RT_METADATA	=18,/**< Packet contains server meta-data */
	TRACE_RT_DUCK_5_0 	=19,/**< Dag 5.0 Duck */

	/** Not actually used - all DATA types begin from this value */
	TRACE_RT_DATA_SIMPLE	= 1000, 
	

	/** As PCAP does not store the linktype with the packet, we need to 
	 * create a separate RT type for each supported DLT, starting from
	 * this value */
	TRACE_RT_DATA_DLT		= 2000, 
	/** BPF does not store the linktype with the packet, so we need a
	 * separate RT type for each supported DLT. This value represents the
	 * starting point */
	TRACE_RT_DATA_BPF		= 3000,


	TRACE_RT_DATA_BPF_END		= 3999,
	/** Unused value marking the end of the valid range for all RT packet
	 * types */
	TRACE_RT_LAST			= 4000
} libtrace_rt_types_t;

/** IP Protocol values */
typedef enum {
	TRACE_IPPROTO_IP	= 0,	/**< IP pseudo protocol number */
	TRACE_IPPROTO_ICMP	= 1,	/**< Internet Control Message protocol */
	TRACE_IPPROTO_IGMP	= 2,	/**< Internet Group Management Protocol */
	TRACE_IPPROTO_IPIP	= 4,	/**< IP encapsulated in IP */
	TRACE_IPPROTO_TCP	= 6,	/**< Transmission Control Protocol */
	TRACE_IPPROTO_UDP	= 17,	/**< User Datagram Protocol */
	TRACE_IPPROTO_IPV6	= 41,	/**< IPv6 over IPv4 */
	TRACE_IPPROTO_ROUTING	= 43,	/**< IPv6 Routing header */
	TRACE_IPPROTO_FRAGMENT	= 44,	/**< IPv6 Fragmentation header */
	TRACE_IPPROTO_RSVP	= 46,	/**< Resource Reservation Protocol */
	TRACE_IPPROTO_GRE	= 47,	/**< General Routing Encapsulation */
	TRACE_IPPROTO_ESP	= 50,	/**< Encapsulated Security Payload [RFC2406] */
	TRACE_IPPROTO_AH	= 51,	/**< Authentication Header [RFC2402] */
	TRACE_IPPROTO_ICMPV6	= 58,	/**< ICMPv6 */
	TRACE_IPPROTO_NONE	= 59,	/**< IPv6 no next header */
	TRACE_IPPROTO_DSTOPTS	= 60,	/**< IPv6 destination options */
	TRACE_IPPROTO_OSPF	= 89,	/**< Open Shortest Path First routing protocol */
	TRACE_IPPROTO_PIM	= 103,	/**< Protocol Independant Multicast */
	TRACE_IPPROTO_SCTP	= 132	/**< Stream Control Transmission Protocol */
} libtrace_ipproto_t;

/** Ethertypes supported by Libtrace */
typedef enum {
	/* Numbers <=1500 are of course, LLC/SNAP */
	TRACE_ETHERTYPE_LOOPBACK= 0x0060,	/**< Ethernet Loopback */
	TRACE_ETHERTYPE_IP	= 0x0800,	/**< IPv4 */
	TRACE_ETHERTYPE_ARP	= 0x0806,	/**< Address resolution protocol */
	TRACE_ETHERTYPE_RARP	= 0x8035,	/**< Reverse ARP */
	TRACE_ETHERTYPE_8021Q	= 0x8100,	/**< 802.1q VLAN Extended Header */
	TRACE_ETHERTYPE_IPV6	= 0x86DD,	/**< IPv6 */
	TRACE_ETHERTYPE_MPLS	= 0x8847,	/**< MPLS Unicast traffic */
	TRACE_ETHERTYPE_MPLS_MC = 0x8848,	/**< MPLS Multicast traffic */
	TRACE_ETHERTYPE_PPP_DISC= 0x8863,	/**< PPPoE Service Discovery */
	TRACE_ETHERTYPE_PPP_SES = 0x8864	/**< PPPoE Session Messages */
} libtrace_ethertype_t;

/** The libtrace packet structure. Applications shouldn't be 
 * meddling around in here 
 */
typedef struct libtrace_packet_t {
	struct libtrace_t *trace; 	/**< Pointer to the trace */
	void *header;			/**< Pointer to the framing header */
	void *payload;			/**< Pointer to the link layer */
	void *buffer;			/**< Allocated buffer */
	libtrace_rt_types_t  type; 	/**< RT protocol type for the packet */
	buf_control_t buf_control; 	/**< Describes memory ownership */
	int capture_length;		/**< Cached capture length */
	int wire_length;		/**< Cached wire length */
	int payload_length;		/**< Cached payload length */
	void *l2_header;		/**< Cached link header */
	libtrace_linktype_t link_type;	/**< Cached link type */
	uint32_t l2_remaining;		/**< Cached link remaining */
	void *l3_header;		/**< Cached l3 header */
	uint16_t l3_ethertype;		/**< Cached l3 ethertype */
	uint32_t l3_remaining;		/**< Cached l3 remaining */
	void *l4_header;		/**< Cached transport header */
	uint8_t transport_proto;	/**< Cached transport protocol */
	uint32_t l4_remaining;		/**< Cached transport remaining */
} libtrace_packet_t;


/** Trace directions. 
 * Note that these are the directions used by convention. More directions 
 * are possible, not just these 3, and that they may not conform to this
 * convention.
 */
typedef enum {
	TRACE_DIR_OUTGOING = 0,		/**< Packets originating "inside" */
	TRACE_DIR_INCOMING = 1,		/**< Packets originating "outside" */
	TRACE_DIR_OTHER	   = 2,		/**< Packets with an unknown direction, or one that's unknown */
	TRACE_DIR_UNKNOWN = -1,		/**< No direction information available */
} libtrace_direction_t;

/** Enumeration of Radiotap fields */
typedef enum {
    TRACE_RADIOTAP_TSFT = 0, /**< Timer synchronisation function, in microseconds (uint64) */
    TRACE_RADIOTAP_FLAGS = 1, /**< Wireless flags (uint8) */
    TRACE_RADIOTAP_RATE = 2, /**< Bitrate in units of 500kbps (uint8) */
    TRACE_RADIOTAP_CHANNEL = 3, /**< Frequency in MHz (uint16) and channel flags (uint16) */
    TRACE_RADIOTAP_FHSS = 4, /**< FHSS hop set (uint8) and hopping pattern (uint8) */
    TRACE_RADIOTAP_DBM_ANTSIGNAL = 5, /**< Signal power in dBm (int8) */
    TRACE_RADIOTAP_DBM_ANTNOISE = 6, /**< Noise power in dBm (int8) */
    TRACE_RADIOTAP_LOCK_QUALITY = 7, /**< Barker Code lock quality (uint16) */
    TRACE_RADIOTAP_TX_ATTENUATION = 8, /**< TX attenuation as unitless distance from max power (uint16) */
    TRACE_RADIOTAP_DB_TX_ATTENUATION = 9, /**< TX attenutation as dB from max power (uint16) */
    TRACE_RADIOTAP_DBM_TX_POWER = 10, /**< TX Power in dBm (int8) */
    TRACE_RADIOTAP_ANTENNA = 11, /**< Antenna frame was rx'd or tx'd on (uint8) */
    TRACE_RADIOTAP_DB_ANTSIGNAL = 12, /**< Signal power in dB from a fixed reference (uint8) */
    TRACE_RADIOTAP_DB_ANTNOISE = 13, /**< Noise power in dB from a fixed reference (uint8) */
    TRACE_RADIOTAP_RX_FLAGS = 14, /** Properties of received frame (uint16) */
    TRACE_RADIOTAP_TX_FLAGS = 15, /** Properties of transmitted frame (uint16) */
    TRACE_RADIOTAP_RTS_RETRIES = 16, /** Number of rts retries frame used (uint8) */
    TRACE_RADIOTAP_DATA_RETRIES = 17, /** Number of unicast retries a transmitted frame used (uint8) */
    TRACE_RADIOTAP_EXT = 31
} libtrace_radiotap_field_t;


/** @name Protocol structures
 * These convenience structures provide portable versions of the headers
 * for a variety of protocols.
 * @{
 */


/** Generic IP header structure */
typedef struct libtrace_ip
{
    unsigned int ip_hl:4;		/**< Header Length */
    unsigned int ip_v:4;		/**< Version */
    uint8_t  ip_tos;			/**< Type of Service */
    uint16_t ip_len;			/**< Total Length */
    int16_t  ip_id;			/**< Identification */
    uint16_t ip_off;			/**< IP Fragment offset (and flags) */
    uint8_t  ip_ttl;			/**< Time to Live */
    uint8_t  ip_p;			/**< Protocol */
    uint16_t ip_sum;			/**< Checksum */
    struct in_addr ip_src;		/**< Source Address */
    struct in_addr ip_dst;		/**< Destination Address */
} libtrace_ip_t;

/** IPv6 header extension structure */
typedef struct libtrace_ip6_ext
{
	uint8_t nxt;	/**< Next header */
	uint8_t len;	/**< Length of the current header */
} libtrace_ip6_ext_t;

typedef struct libtrace_ip6_frag 
{
	uint8_t nxt;	/**< Next header */
	uint8_t res;	/**< Reserved */
	uint16_t frag_off;	/**< Fragment Offset (includes M flag) */
	uint32_t ident;	/** Fragment identification */
} libtrace_ip6_frag_t;

/** Generic IPv6 header structure
 *
 * @note The flow label field also includes the Version and Traffic Class
 * fields, because we haven't figured out a nice way to deal with fields
 * crossing byte boundaries on both Big and Little Endian machines */
typedef struct libtrace_ip6
{ 
    uint32_t flow;			/**< Flow label */
    uint16_t plen;			/**< Payload length */
    uint8_t nxt;			/**< Next header */
    uint8_t hlim;			/**< Hop limit */
    struct in6_addr ip_src;		/**< Source address */
    struct in6_addr ip_dst;		/**< Destination address */
} libtrace_ip6_t;

/** Generic TCP header structure */
typedef struct libtrace_tcp
  {
    uint16_t source;		/**< Source Port */
    uint16_t dest;		/**< Destination port */
    uint32_t seq;		/**< Sequence number */
    uint32_t ack_seq;		/**< Acknowledgement Number */
    unsigned int ecn_ns:1;      /**< ECN Nonce Sum */
    unsigned int res1:3;        /**< Reserved bits */
    unsigned int doff:4;        /**< Data Offset */
    unsigned int fin:1;         /**< FIN */
    unsigned int syn:1;         /**< SYN flag */
    unsigned int rst:1;         /**< RST flag */
    unsigned int psh:1;         /**< PuSH flag */
    unsigned int ack:1;         /**< ACK flag */
    unsigned int urg:1;         /**< URG flag */
    unsigned int ece:1;         /**< ECN Echo */
    unsigned int cwr:1;         /**< ECN CWR */
    uint16_t window;		/**< Window Size */
    uint16_t check;		/**< Checksum */
    uint16_t urg_ptr;		/**< Urgent Pointer */
} libtrace_tcp_t;

/** Generic UDP header structure */
typedef struct libtrace_udp {
  uint16_t	source;		/**< Source port */
  uint16_t	dest;		/**< Destination port */
  uint16_t	len;		/**< Length */
  uint16_t	check;		/**< Checksum */
} libtrace_udp_t;

/** Generic ICMP header structure */
typedef struct libtrace_icmp
{
  uint8_t type;		/**< Message Type */
  uint8_t code;		/**< Type Sub-code */
  uint16_t checksum;		/**< Checksum */
  union
  {
    struct
    {
      uint16_t	id;		/**< ID of the Echo request */
      uint16_t	sequence;	/**< Sequence number of the Echo request */
    } echo;			/**< Echo Datagram */
    uint32_t	gateway;	/**< Gateway Address */
    struct
    {
      uint16_t	unused;		/**< Unused */
      uint16_t	mtu;		/**< Next-hop MTU */
    } frag;			/**< Path MTU Discovery */
  } un;				/**< Union for Payloads of Various ICMP Codes */
} libtrace_icmp_t;

typedef struct libtrace_icmp6 {
  uint8_t type;		/**< Message Type */
  uint8_t code;		/**< Type Sub-code */
  uint16_t checksum;	/**< Checksum */

  union {
    struct {
	uint8_t length;	  /**< Length of original datagram content in 64 bit words */
	uint8_t unused;	  /**< Unused */	
	uint8_t unused1;  /**< Unused */
    } extend;	/**< Extensions added in RFC 4884 for Time Exceeded and Destination Unreachable Messages */

    uint32_t mtu;	/**< MTU from Packet Too Big Message */
    uint32_t pointer;	/**< Pointer from Parameter Problem Message */
    struct {
	uint16_t id;	/**< Echo Identifier */
	uint16_t sequence; /**< Echo Sequence Number */
    } echo; /**< Data required for Echo Request and Reply messages */
  } un;
} libtrace_icmp6_t;

/** Generic LLC/SNAP header structure */
typedef struct libtrace_llcsnap
{
/* LLC */
  uint8_t dsap;			/**< Destination Service Access Point */
  uint8_t ssap;			/**< Source Service Access Point */
  uint8_t control;		/**< Control field */
/* SNAP */
  unsigned int oui:24;		/**< Organisationally Unique Identifier (scope)*/
  uint16_t type;		/**< Protocol within OUI */
} libtrace_llcsnap_t;

/** 802.3 frame */
typedef struct libtrace_ether
{
  uint8_t ether_dhost[6];	/**< Destination Ether Addr */
  uint8_t ether_shost[6];	/**< Source Ether Addr */
  uint16_t ether_type;		/**< Packet Type ID Field (next-header) */
} libtrace_ether_t;

/** 802.1Q frame */
typedef struct libtrace_8021q 
{
  unsigned int vlan_pri:3;	 /**< VLAN User Priority */
  unsigned int vlan_cfi:1; 	 /**< VLAN Format Indicator, 
				   * 0 for ethernet, 1 for token ring */
  unsigned int vlan_id:12; 	 /**< VLAN Id */
  uint16_t vlan_ether_type;	 /**< VLAN Sub-packet Type ID Field 
				   * (next-header)*/
} libtrace_8021q_t;

/** ATM User Network Interface (UNI) Cell. */
typedef struct libtrace_atm_cell
{
  unsigned int gfc:4;		/**< Generic Flow Control */
  unsigned int vpi:8;		/**< Virtual Path Identifier */
  unsigned int vci:16;		/**< Virtual Channel Identifier */
  unsigned int pt:3;		/**< Payload Type */
  unsigned int clp:1;		/**< Cell Loss Priority */
  unsigned int hec:8;		/**< Header Error Control */
} libtrace_atm_cell_t;

/** ATM Network Node/Network Interface (NNI) Cell. */
typedef struct libtrace_atm_nni_cell
{
  unsigned int vpi:12;		/**< Virtual Path Identifier */
  unsigned int vci:16;		/**< Virtual Channel Identifier */
  unsigned int pt:3;		/**< Payload Type */
  unsigned int clp:1;		/**< Cell Loss Priority */
  unsigned int hec:8;		/**< Header Error Control */
} libtrace_atm_nni_cell_t;

/** Captured UNI cell.
 *
 * Endace don't capture the HEC, presumably to keep alignment.  This 
 * version of the \ref libtrace_atm_cell is used when dealing with DAG 
 * captures of uni cells.
 *
 */
typedef struct libtrace_atm_capture_cell
{
  unsigned int gfc:4;		/**< Generic Flow Control */
  unsigned int vpi:8;		/**< Virtual Path Identifier */
  unsigned int vci:16;		/**< Virtual Channel Identifier */
  unsigned int pt:3;		/**< Payload Type */
  unsigned int clp:1;		/**< Cell Loss Priority */
} libtrace_atm_capture_cell_t;

/** Captured NNI cell.
 *
 * Endace don't capture the HEC, presumably to keep alignment.  This 
 * version of the \ref libtrace_atm_nni_cell is used when dealing with DAG
 * captures of nni cells.
 *
 */
typedef struct libtrace_atm_nni_capture_cell
{
  unsigned int vpi:12;		/**< Virtual Path Identifier */
  unsigned int vci:16;		/**< Virtual Channel Identifier */
  unsigned int pt:3;		/**< Payload Type */
  unsigned int clp:1;		/**< Cell Loss Priority */
  unsigned int hec:8;		/**< Header Error Control */
} libtrace_atm_nni_capture_cell_t;

/** PPP header */
typedef struct libtrace_ppp
{
 /* I can't figure out where the hell these two variables come from. They're
  * definitely not in RFC 1661 which defines PPP. Probably some weird thing
  * relating to the lack of distinction between PPP, HDLC and CHDLC */
	
/* uint8_t address; */		/**< PPP Address (0xFF - All stations) */
/* uint8_t header;  */		/**< PPP Control (0x03 - Unnumbered info) */
 uint16_t protocol;		/**< PPP Protocol (htons(0x0021) - IPv4 */
} libtrace_ppp_t;

/** PPPoE header */
typedef struct libtrace_pppoe
{
 unsigned int version:4;	/**< Protocol version number */
 unsigned int type:4;		/**< PPPoE Type */
 uint8_t code;			/**< PPPoE Code */
 uint16_t session_id;		/**< Session Identifier */
 uint16_t length;		/**< Total Length of the PPP packet */
} libtrace_pppoe_t;

/** Libtrace local definition of GRE (Generalised Routing Protocol) header
 * RFC2890
 */
typedef struct libtrace_gre_t
{
    uint16_t flags;         /**< Flags and version */
    uint16_t ethertype;     /**< Payload ethertype */
    uint16_t checksum;      /**< Optional checksum */
    uint16_t reserved1;     /**< Optional reserved */
    uint16_t key;           /**< Optional key (or Tenant Network ID) */
    uint16_t seq;           /**< Optional sequence number */
} libtrace_gre_t;

#define LIBTRACE_GRE_FLAG_CHECKSUM 0x8000
#define LIBTRACE_GRE_FLAG_KEY      0x2000
#define LIBTRACE_GRE_FLAG_SEQ      0x1000
#define LIBTRACE_GRE_FLAG_VERMASK  0x0007

/** Libtrace local definition of VXLAN Header
 * (draft-mahalingam-dutt-dcops-vxlan)
 */
typedef struct libtrace_vxlan_t
{
    uint8_t flags;          /**< Flags */
    uint8_t reserved1[3];   /**< Reserved */
    uint8_t vni[3];         /**< VXLAN Network Identifier (VNI) */
    uint8_t reserved2;
} libtrace_vxlan_t;

/** 802.11 header */
typedef struct libtrace_80211_t {
        unsigned int      protocol:2;	/**< Protocol Version */
        unsigned int      type:2;	/**< Frame Type */
        unsigned int      subtype:4;	/**< Frame Subtype */

        unsigned int      to_ds:1;	/**< Packet to Distribution Service */
        unsigned int      from_ds:1;	/**< Packet from Distribution Service */
        unsigned int      more_frag:1;	/**< Packet has more fragments */
        unsigned int      retry:1;	/**< Packet is a retry */
        unsigned int      power:1;	/**< Power Management mode */
        unsigned int      more_data:1;	/**< More data is buffered at station */
        unsigned int      wep:1;	/**< WEP encryption indicator */
        unsigned int      order:1;	/**< Strictly-Ordered class indicator */
	
        uint16_t     duration;	/**< Duration value for NAV calculation */
        uint8_t      mac1[6];	/**< MAC Address 1 */
        uint8_t      mac2[6];	/**< MAC Address 2 */
        uint8_t      mac3[6];	/**< MAC Address 3 */
        uint16_t     SeqCtl;	/**< Sequence Control */	
        uint8_t      mac4[6];	/**< MAC Address 4 */
} libtrace_80211_t;

/** The Radiotap header pre-amble
 *
 * All Radiotap headers start with this pre-amble, followed by the fields
 * specified in the it_present bitmask. If bit 31 of it_present is set, then
 * another bitmask follows.
 * @note All of the radiotap data fields are in little-endian byte-order.
 */
typedef struct libtrace_radiotap_t {
    uint8_t     it_version; /**< Radiotap version */
    uint8_t     it_pad; /**< Padding for natural alignment */
    uint16_t    it_len; /**< Length in bytes of the entire Radiotap header */
    uint32_t    it_present; /**< Which Radiotap fields are present */
} libtrace_radiotap_t;

/** OSPF header */
typedef struct libtrace_ospf_v2_t
{
	uint8_t ospf_v;		/**< OSPF Version, should be 2 */
	uint8_t type;		/**< OSPF Packet Type */
	uint16_t ospf_len;	/**< Packet length, including OSPF header */
	struct in_addr router;	/**< Router ID of the packet source */
	struct in_addr area;	/**< Area the packet belongs to */
	uint16_t sum;		/**< Checksum */
	uint16_t au_type;	/**< Authentication procedure */
	uint16_t zero;		/**< Always zero */
	uint8_t au_key_id;	/**< Authentication Key ID */
	uint8_t au_data_len;	/**< Authentication Data Length */
	uint32_t au_seq_num;	/**< Cryptographic Sequence Number */
} libtrace_ospf_v2_t;

/** Options Field present in some OSPFv2 packets */
typedef struct libtrace_ospf_options_t {
	unsigned int unused1:1;
	unsigned int e_bit:1;
	unsigned int mc_bit:1;
	unsigned int np_bit:1;
	unsigned int ea_bit:1;
	unsigned int dc_bit:1;
	unsigned int unused2:2;
} libtrace_ospf_options_t;

/** LSA Header for OSPFv2 */
typedef struct libtrace_ospf_lsa_v2_t
{
	uint16_t age;		/**< Time in seconds since LSA originated */
	libtrace_ospf_options_t lsa_options;	/**< Options */
	uint8_t lsa_type;	/**< LSA type */
	struct in_addr ls_id;	/**< Link State ID */
	struct in_addr adv_router; /**< Router that originated this LSA */
	uint32_t seq;		/**< LS sequence number */
	uint16_t checksum;	/**< Checksum */ 
	uint16_t length;	/**< Length of the LSA including LSA header */
} libtrace_ospf_lsa_v2_t;

/** OSPFv2 Hello Packet */
typedef struct libtrace_ospf_hello_v2_t
{
	struct in_addr mask;	/**< Network mask for this interface */
	uint16_t interval;	/**< Interval between Hello packets (secs) */
	libtrace_ospf_options_t hello_options;	/**< Options */
	uint8_t priority;	/**< Router Priority */
	uint32_t deadint;	/**< Interval before declaring a router down */
	struct in_addr designated;	/**< Designated router for the network */
	struct in_addr backup;	/**< Backup designated router */

	/** Neighbors follow from here, but there can be anywhere from 1 to N
	 * neighbors so I can't include that here */
} libtrace_ospf_hello_v2_t;

/** OSPFv2 Database Description packet */
typedef struct libtrace_ospf_db_desc_v2_t
{
	uint16_t mtu;		/**< Interface MTU */
	libtrace_ospf_options_t db_desc_options;	/**< Options */
	unsigned int db_desc_ms:1;	/**< If set, this router is the master */
	unsigned int db_desc_m:1;	/**< If set, more packets to follow */
	unsigned int db_desc_i:1;	/**< If set, this is the first packet in sequence */
	unsigned int zero:5;
	uint32_t seq;		/**< Sequence number for DD packets */
} libtrace_ospf_db_desc_v2_t;

/** OSPF Link State Request Packet */
typedef struct libtrace_ospf_ls_req_t
{
	uint32_t ls_type;	/**< Link State Type */
	uint32_t ls_id;		/**< Link State Id */
	uint32_t advertising_router;	/**< Advertising Router */
} libtrace_ospf_ls_req_t;

/** OSPF Link State Update Packet */
typedef struct libtrace_ospf_ls_update_t
{
	uint32_t ls_num_adv;	/**< Number of LSAs in this packet */

	/* Followed by LSAs, use API functions to access these */
} libtrace_ospf_ls_update_t;

/** OSPFv2 AS External LSA Body */
typedef struct libtrace_ospf_as_external_lsa_t
{
	struct in_addr netmask;	/**< Netmask for the destination */
	unsigned int tos:7;
	unsigned int e:1;	/**< If set, metric is Type 2. Else, Type 1 */
	uint8_t metric_a;	/**< Byte 1 of the Metric field */
	uint8_t metric_b;	/**< Byte 2 of the Metric field */
	uint8_t metric_c;	/**< Byte 3 of the Metric field */
	struct in_addr forwarding;	/**< Forwarding address */
	uint32_t external_tag;		/**< External Route Tag */
} libtrace_ospf_as_external_lsa_v2_t;

/** OSPFv2 Summary LSA Body */
typedef struct libtrace_ospf_summary_lsa
{
	struct in_addr netmask;	/**< Netmask for the destination */
	uint8_t zero;		/**< Always zero */
	uint8_t metric_a;	/**< Byte 1 of the Metric field */
	uint8_t metric_b;	/**< Byte 2 of the Metric field */
	uint8_t metric_c;	/**< Byte 3 of the Metric field */
	
} libtrace_ospf_summary_lsa_v2_t;

/** OSPFv2 Network LSA Body */
typedef struct libtrace_ospf_network_lsa_t
{
	struct in_addr netmask;	/**< Netmask for the network */
	/* Followed by IDs of attached routers */
} libtrace_ospf_network_lsa_v2_t;

/** OSPFv2 Router Link structure */
typedef struct libtrace_ospf_link_t
{
	struct in_addr link_id;		/**< Object that link connects to */
	struct in_addr link_data;	/**< Link Data field */
	uint8_t type;			/**< Link Type */
	uint8_t num_tos;		/**< Number of TOS metrics */
	uint16_t tos_metric;		/**< Cost of router link */
} libtrace_ospf_link_v2_t;

/** OSPFv2 Router LSA */
typedef struct libtrace_ospf_router_lsa_t
{
	unsigned int b:1;	/**< Area Border Router Flag */
	unsigned int e:1;	/**< External Router Flag */
	unsigned int v:1;	/**< Virtual Endpoint Flag */
	unsigned int zero:5;
	uint8_t zero2;		/**< Always zero */
	uint16_t num_links;	/**< Number of links in LSA */
} libtrace_ospf_router_lsa_v2_t;

typedef enum {
	TRACE_OSPF_HELLO = 1,		/**< OSPF Hello */
	TRACE_OSPF_DATADESC = 2,	/**< OSPF Database Description */
	TRACE_OSPF_LSREQ = 3,		/**< OSPF Link State Request */
	TRACE_OSPF_LSUPDATE = 4,	/**< OSPF Link State Update */
	TRACE_OSPF_LSACK = 5		/**< OSPF Link State Acknowledgement */
} libtrace_ospf_types_t;

typedef enum {
        TRACE_OSPF_LS_ROUTER = 1,	/**< OSPF Router LSA */
        TRACE_OSPF_LS_NETWORK = 2,	/**< OSPF Network LSA */
        TRACE_OSPF_LS_SUMMARY = 3,	/**< OSPF Summary LSA */
        TRACE_OSPF_LS_ASBR_SUMMARY = 4,	/**< OSPF Summary LSA (ASBR) */
        TRACE_OSPF_LS_EXTERNAL = 5	/**< OSPF AS External LSA */
} libtrace_ospf_ls_types_t;

/** A local definition of an SLL header */
typedef struct libtrace_sll_header_t {
        uint16_t pkttype;               /**< Packet type */
        uint16_t hatype;                /**< Link-layer address type */
        uint16_t halen;                 /**< Link-layer address length */
        unsigned char addr[8];          /**< Link-layer address */
        uint16_t protocol;              /**< Protocol */
} libtrace_sll_header_t;


/* SLL packet types */

/** Packet was addressed for the local host */
#define TRACE_SLL_HOST          0
/** Packet was addressed for a broadcast address */
#define TRACE_SLL_BROADCAST     1
/** Packet was addressed for a multicast address */
#define TRACE_SLL_MULTICAST     2
/** Packet was addressed for another host but was captured by a promiscuous
 * device */
#define TRACE_SLL_OTHERHOST     3
/** Packet originated from the local host */
#define TRACE_SLL_OUTGOING      4



/*@}*/

/** Prints help information for libtrace 
 *
 * Function prints out some basic help information regarding libtrace,
 * and then prints out the help() function registered with each input module
 */
 void trace_help(void);

/** Causes a libtrace reader to stop blocking whilst waiting on new packets
 * and immediately return EOF.
 *
 * This function is useful if you are handling signals within your libtrace
 * program. If a live source is not receiving any packets (or they are being
 * filtered), a call to trace_read_packet will result in an infinite loop as
 * it will block until a packet is received. Normally, a SIGINT would cause the
 * program to end and thus break the loop, but if you are handling the signal
 * yourself then that signal will never reach libtrace.
 *
 * Instead this function sets a global variable within libtrace that will 
 * cause a blocking live capture to break on the next internal timeout, 
 * allowing control to be returned to the user and their own signal handling
 * to kick in.
 */
 void trace_interrupt(void); 

/** @name Trace management
 * These members deal with creating, configuring, starting, pausing and
 * cleaning up a trace object
 *@{
 */

/** Takes a uri and splits it into a format and uridata component. 
 * @param uri		The uri to be parsed
 * @param [out] format	A pointer that will be updated to point to an allocated 
 * 			string holding the format component of the URI
 * @return NULL if an error occured, otherwise return a pointer to the uridata 
 * component
 *
 * @note The format component is strdup'd by this function, so be sure to free
 * it when you are done with the split URI. Similarly, do not pass a pointer
 * to an allocated string into this function as the 'format' parameter, as
 * that memory will be leaked and replaced with the strdup'd format.
 */
 const char *trace_parse_uri(const char *uri, char **format);

/** Create an input trace from a URI
 * 
 * @param uri A valid libtrace URI to be opened
 * @return An opaque pointer to a libtrace_t
 *
 * Some valid URI's are:
 *  - erf:/path/to/erf/file
 *  - erf:-  (stdin)
 *  - dag:/dev/dagcard                  
 *  - pcapint:pcapinterface                (eg: pcap:eth0)
 *  - pcap:/path/to/pcap/file
 *  - pcap:-
 *  - rt:hostname
 *  - rt:hostname:port
 *
 *  If an error occured when attempting to open the trace file, a
 *  trace is still returned so trace_is_err() should be called to find out
 *  if an error occured. The trace is created in the configuration state, you 
 *  must call trace_start before attempting to read packets from the trace.
 */
 libtrace_t *trace_create(const char *uri);

/** Creates a "dummy" trace file that has only the format type set.
 *
 * @param uri A valid (but fake) URI indicating the format of the dummy trace that is to be created.
 * @return An opaque pointer to a (sparsely initialised) libtrace_t
 *
 * Only the format portion of the uri parameter matters - the 'file' being
 * opened does not have to exist.
 *
 * @note IMPORTANT: Do not attempt to call trace_read_packet or other such 
 * functions with the dummy trace. Its intended purpose is to provide access
 * to the format functions where the original trace may no longer exist or is
 * not the correct format, e.g. reading ERF packets from an RT input.
 */
 libtrace_t *trace_create_dead(const char *uri);

/** Creates a trace output file from a URI. 
 *
 * @param uri The uri string describing the output format and destination
 * @return An opaque pointer to a libtrace_output_t
 *
 * Valid URIs include:
 *  - erf:/path/to/erf/file
 *  - pcap:/path/to/pcap/file
 *
 *  If an error occured when attempting to open the output trace, a trace is 
 *  still returned but trace_errno will be set. Use trace_is_err_out() and 
 *  trace_perror_output() to get more information.
 */
 libtrace_out_t *trace_create_output(const char *uri);

/** Start an input trace
 * @param libtrace	The trace to start
 * @return 0 on success, -1 on failure
 *
 * This does the actual work of starting the input trace and applying
 * all the config options.  This may fail, returning -1. The libtrace error
 * handling functions can be used to get more information about what 
 * specifically went wrong.
 */
 int trace_start(libtrace_t *libtrace);

/** Pauses an input trace
 * @param libtrace	The trace to pause
 * @return 0 on success, -1 on failure
 *
 * This stops an input trace that is in progress and returns you to the 
 * configuration state.  Any packets that arrive on a live capture after 
 * trace_pause() has been called will be discarded.  To resume the trace, call 
 * trace_start().
 */
 int trace_pause(libtrace_t *libtrace);

/** Start an output trace
 * @param libtrace	The trace to start
 * @return 0 on success, -1 on failure
 *
 * This does the actual work with starting a trace capable of writing packets.  
 * This generally creates the output file.
 */
 int trace_start_output(libtrace_out_t *libtrace);

/** Valid configuration options for input traces */
typedef enum {
	/** Maximum number of bytes to be captured for any given packet */
	TRACE_OPTION_SNAPLEN, 	

	/** If enabled, places a live capture interface into promiscuous mode */
	TRACE_OPTION_PROMISC, 	

	/** Apply this filter to all packets read from this trace */
	TRACE_OPTION_FILTER,  	

	/** Defines the frequency of meta-data reporting, e.g. DUCK packets */
	TRACE_OPTION_META_FREQ,	

	/** If enabled, the libtrace event API will ignore time gaps between
	 * packets when reading from a trace file */
	TRACE_OPTION_EVENT_REALTIME
} trace_option_t;

/** Sets an input config option
 * @param libtrace	The trace object to apply the option to
 * @param option	The option to set
 * @param value		The value to set the option to
 * @return -1 if option configuration failed, 0 otherwise
 * This should be called after trace_create, and before trace_start
 */
 int trace_config(libtrace_t *libtrace,
		trace_option_t option,
		void *value);

/** Valid compression types 
 * Note, this must be kept in sync with WANDIO_COMPRESS_* numbers in wandio.h
 */ 
typedef enum {
	TRACE_OPTION_COMPRESSTYPE_NONE = 0, /**< No compression */
	TRACE_OPTION_COMPRESSTYPE_ZLIB = 1, /**< GZip Compression */
	TRACE_OPTION_COMPRESSTYPE_BZ2  = 2, /**< BZip2 Compression */
	TRACE_OPTION_COMPRESSTYPE_LZO  = 3,  /**< LZO Compression */
	TRACE_OPTION_COMPRESSTYPE_LZMA  = 4,  /**< LZO Compression */
        TRACE_OPTION_COMPRESSTYPE_LAST
} trace_option_compresstype_t;

/** Valid configuration options for output traces */
typedef enum {
	/** File flags to use when opening an output file, e.g. O_APPEND */
	TRACE_OPTION_OUTPUT_FILEFLAGS,
	/** Compression level: 0 = no compression, 1 = faster compression,
	 * 9 = better compression */
	TRACE_OPTION_OUTPUT_COMPRESS,
	/** Compression type, see trace_option_compresstype_t */
	TRACE_OPTION_OUTPUT_COMPRESSTYPE
} trace_option_output_t;

/** Sets an output config option
 *
 * @param libtrace	The output trace object to apply the option to
 * @param option	The option to set
 * @param value		The value to set the option to
 * @return -1 if option configuration failed, 0 otherwise
 * This should be called after trace_create_output, and before 
 * trace_start_output
 */
 int trace_config_output(libtrace_out_t *libtrace, 
		trace_option_output_t option,
		void *value
		);

/** Close an input trace, freeing up any resources it may have been using
 *
 * @param trace 	The input trace to be destroyed
 *
 */
 void trace_destroy(libtrace_t *trace);

/** Close a dummy trace file, freeing up any resources it may have been using
 * @param trace		The dummy trace to be destroyed
 */
 void trace_destroy_dead(libtrace_t *trace);

/** Close an output trace, freeing up any resources it may have been using
 * @param trace		The output trace to be destroyed
 */
 void trace_destroy_output(libtrace_out_t *trace);

/** Check (and clear) the current error state of an input trace
 * @param trace		The input trace to check the error state on
 * @return The current error status and message
 *
 * This reads and returns the current error state and sets the current error 
 * to "no error".
 */
 libtrace_err_t trace_get_err(libtrace_t *trace);

/** Indicate if there has been an error on an input trace
 * @param trace		The input trace to check the error state on
 * @return true if an error has occurred, false otherwise
 *
 * @note This does not clear the error status, and only returns true or false.
 */
 bool trace_is_err(libtrace_t *trace);

/** Outputs the error message for an input trace to stderr and clear the error 
 * status.
 * @param trace		The input trace with the error to output
 * @param msg		The message to prepend to the error
 *
 * This function does clear the error status.
 */
 void trace_perror(libtrace_t *trace, const char *msg,...);

/** Checks (and clears) the current error state for an output trace
 * @param trace		The output trace to check the error state on
 * @return The current error status and message
 * 
 * This reads and returns the current error state and sets the current error 
 * to "no error".
 */
 libtrace_err_t trace_get_err_output(libtrace_out_t *trace);

/** Indicates if there is an error on an output trace
 * @param trace		The output trace to check the error state on
 * @return true if an error has occurred, false otherwise.
 *
 * This does not clear the error status, and only returns true or false.
 */
 bool trace_is_err_output(libtrace_out_t *trace);

/** Outputs the error message for an output trace to stderr and clear the error
 * status.
 * @param trace		The output trace with the error to output
 * @param msg		The message to prepend to the error
 * This function does clear the error status.
 */
void trace_perror_output(libtrace_out_t *trace, const char *msg,...);

/** Returns the number of packets observed on an input trace. 
 * Includes the number of packets counted as early as possible, before
 * filtering, and includes dropped packets.
 *
 * @param trace		The input trace to examine
 * @returns The number of packets seen at the capture point before filtering.
 *
 * If the number is not known, this function will return UINT64_MAX
 */

uint64_t trace_get_received_packets(libtrace_t *trace);

/** Returns the number of packets that were captured, but discarded for not
 * matching a provided filter. 
 *
 * @param trace		The input trace to examine
 * @returns The number of packets that were successfully captured, but filtered
 *
 * If the number is not known, this function will return UINT64_MAX
 */

uint64_t trace_get_filtered_packets(libtrace_t *trace);

/** Returns the number of packets that have been dropped on an input trace due 
 * to lack of buffer space on the capturing device.
 *
 * @param trace		The input trace to examine
 * @returns The number of packets captured, but dropped due to buffer overruns
 *
 * If the number is not known, this function will return UINT64_MAX
 */

uint64_t trace_get_dropped_packets(libtrace_t *trace);

/** Returns the number of packets that have been read from the input trace using
 * trace_read_packet().
 *
 * @param trace		The input trace to examine
 * @returns The number of packets that have been read by the libtrace user.
 *
 * If the number is not known, this function will return UINT64_MAX
 */

uint64_t trace_get_accepted_packets(libtrace_t *trace);


/*@}*/

/** @name Reading / Writing packets
 * These members deal with creating, reading and writing packets
 *
 * @{
 */

/** Create a new packet object
 *
 * @return A pointer to an initialised libtrace_packet_t object
 */
 libtrace_packet_t *trace_create_packet(void);

/** Copy a packet object
 * @param packet	The source packet to copy
 * @return A new packet which has the same content as the source packet
 *
 * @note This always involves a copy, which can be slow.  Use of this 
 * function should be avoided where possible.
 * 
 * @par The reason you would want to use this function is that a zerocopied
 * packet from a device will be stored using memory owned by the device which
 * may be a limited resource. Copying the packet will ensure that the packet
 * is now stored in memory owned and managed by libtrace.
 */
 libtrace_packet_t *trace_copy_packet(const libtrace_packet_t *packet);

/** Destroy a packet object
 * @param packet 	The packet to be destroyed
 *
 */
 void trace_destroy_packet(libtrace_packet_t *packet);


/** Read the next packet from an input trace 
 *
 * @param trace 	The libtrace opaque pointer for the input trace
 * @param packet  	The packet opaque pointer
 * @return 0 on EOF, negative value on error, number of bytes read when successful.
 *
 * @note The number of bytes read is usually (but not always) the same as
 * trace_get_framing_length()+trace_get_capture_length() depending on the
 * trace format.
 *
 * @note The trace must have been started with trace_start before calling
 * this function
 *
 * @note When reading from a live capture, this function will block until a
 * packet is observed on the capture interface. The libtrace event API 
 * (e.g. trace_event()) should be used if non-blocking operation is required.
 */
 int trace_read_packet(libtrace_t *trace, libtrace_packet_t *packet);

/** Event types 
 * see \ref libtrace_eventobj_t and \ref trace_event
 */
typedef enum {
	TRACE_EVENT_IOWAIT,	/**< Wait on the given file descriptor */
	TRACE_EVENT_SLEEP,	/**< Sleep for the given amount of time */
	TRACE_EVENT_PACKET,	/**< Packet has been read from input trace */
	TRACE_EVENT_TERMINATE	/**< End of input trace */
} libtrace_event_t;

/** Structure returned by libtrace_event explaining what the current event is */
typedef struct libtrace_eventobj_t {
	libtrace_event_t type; /**< Event type (iowait,sleep,packet) */
	
	/** If the event is IOWAIT, the file descriptor to wait on */
	int fd;		       
	/** If the event is SLEEP, the amount of time to sleep for in seconds */
	double seconds;	       
	/** If the event is PACKET, the value returned by trace_read_packet() */
	int size; 
} libtrace_eventobj_t;

/** Processes the next libtrace event from an input trace.
 * @param trace The libtrace opaque pointer for the input trace
 * @param packet The libtrace_packet opaque pointer to use for reading packets
 * @return A libtrace_event struct containing the event type and details of the event.
 *
 * Type can be:
 *  TRACE_EVENT_IOWAIT	Waiting on I/O on a file descriptor
 *  TRACE_EVENT_SLEEP	Wait a specified amount of time for the next event
 *  TRACE_EVENT_PACKET	Packet was read from the trace
 *  TRACE_EVENT_TERMINATE Trace terminated (perhaps with an error condition)
 */
 libtrace_eventobj_t trace_event(libtrace_t *trace,
		libtrace_packet_t *packet);


/** Write one packet out to the output trace
 *
 * @param trace		The libtrace_out opaque pointer for the output trace
 * @param packet	The packet opaque pointer of the packet to be written
 * @return The number of bytes written out, if zero or negative then an error has occured.
 */
 int trace_write_packet(libtrace_out_t *trace, libtrace_packet_t *packet);

/** Gets the capture format for a given packet.
 * @param packet	The packet to get the capture format for.
 * @return The capture format of the packet
 *
 * @note Due to ability to convert packets between formats relatively easily
 * in Libtrace, the format of the packet right now may not be the format that
 * the packet was originally captured with.
 */
 
enum base_format_t trace_get_format(struct libtrace_packet_t *packet);

/** Construct a libtrace packet from a buffer containing the packet payload.
 * @param[in,out] packet	Libtrace Packet object to update with the new 
 * 				data.
 * @param linktype		The linktype of the packet data.
 * @param[in] data		The packet data (including linklayer).
 * @param len			Length of packet data provided in the buffer.
 *
 * @note The constructed packet will be in the PCAP format.
 *
 * @note To be useful, the provided buffer must start with the layer 2 header
 * (or a metadata header, if desired).
 */

void trace_construct_packet(libtrace_packet_t *packet,
		libtrace_linktype_t linktype, const void *data, uint16_t len);

/*@}*/

/** @name Protocol decodes
 * These functions locate and return a pointer to various headers inside a
 * packet
 * 
 * A packet is divided up into several "layers.":
 *
 * @li Framing header -- This is the header provided by the capture format 
 * itself rather than anything that was sent over the network. This provides
 * basic details about the packet record including capture lengths, wire 
 * lengths, timestamps, direction information and any other metadata that is 
 * part of the capture format.  
 * 
 * @li Metadata header (optional) -- A header containing metadata about a 
 * packet that was captured, but the metadata was not transmitted over the 
 * wire.  Some examples include RadioTap and Linux_sll headers.  This can be 
 * retrieved by trace_get_packet_meta(), or skipped using 
 * trace_get_payload_from_meta(). There may be multiple "metadata" headers on 
 * a packet.
 *
 * @li Layer 2/Link layer/Datalink header -- This can be retrieved by 
 * trace_get_layer2(), or skipped using trace_get_payload_from_layer2().
 *
 * @li Layer 3/IP/IPv6 -- This can be retrieved by trace_get_layer3().  As a 
 * convenience trace_get_ip()/trace_get_ip6() can be used to find an IPv4/IPv6 
 * header.
 *
 * @li Layer 5/transport -- These are protocols carried in IPv4/IPv6 frames. 
 * These can be retrieved using trace_get_transport().
 * 
 * @{
 */


/** Gets a pointer to the first byte of the packet as it was captured and
 * returns its corresponding linktype and capture length.
 *
 * Use this function instead of the deprecated trace_get_link().
 *
 * @param packet The packet to get the buffer from
 * @param[out] linktype The linktype of the returned pointer
 * @param[out] remaining The capture length (the number of captured bytes from
 * the returned pointer)
 * @return A pointer to the first byte of the packet
 */
 void *trace_get_packet_buffer(const libtrace_packet_t *packet,
                libtrace_linktype_t *linktype, uint32_t *remaining);


/** Get a pointer to the IPv4 header (if any) for a given packet
 * @param packet  	The packet to get the IPv4 header for
 *
 * @return A pointer to the IPv4 header, or NULL if there is no IPv4 header
 *
 * If a partial IP header is present, i.e. the packet has been truncated before
 * the end of the IP header, this function will return NULL.
 *
 * You should consider using \ref trace_get_layer3 instead of this function.
 */
 
libtrace_ip_t *trace_get_ip(libtrace_packet_t *packet);

/** get a pointer to the IPv6 header (if any)
 * @param packet  	The packet to get the IPv6 header for
 *
 * @return A pointer to the IPv6 header, or NULL if there is no IPv6 header
 *
 * If a partial IPv6 header is present, i.e. the packet has been truncated
 * before the end of the IP header, this function will return NULL.
 *
 * You should consider using \ref trace_get_layer3 instead of this function.
 */
 
libtrace_ip6_t *trace_get_ip6(libtrace_packet_t *packet);

/** Return a pointer to the first metadata header in a packet, if present.
 *
 * This function takes a pointer to a libtrace packet and if any metadata
 * headers exist, returns a pointer to the first one, along with its
 * corresponding linktype. 
 *
 * If no metadata headers exist in the packet, NULL is returned.
 *
 * A metadata header is a header that was prepended by the capturing device,
 * such as a Linux SLL header, or a Radiotap wireless monitoring header.
 * Subsequent metadata headers may be accessed with the
 * trace_get_payload_from_meta(...) function. 
 *
 * @param packet The libtrace packet
 * @param[out] linktype The linktype of the returned metadata header
 * @param[out] remaining The number of bytes captured after the returned
 * pointer.
 * @return A pointer to the first metadata header, or NULL if there are no
 * metadata headers present.
 *
 * remaining may be NULL, however linktype must be provided.
 */
 void *trace_get_packet_meta(const libtrace_packet_t *packet,
                libtrace_linktype_t *linktype,
                uint32_t *remaining);

/** Returns the payload of a metadata header.
 * 
 * This function takes a pointer to the start of a metadata header (either
 * obtained via trace_get_packet_meta(...) or by a previous call to
 * trace_get_payload_from_meta(...)) along with its corresponding linktype and
 * returns the payload, i.e. the next header. It will also update the linktype
 * parameter to indicate the type of payload.
 *  
 * If the linktype indicates that the header passed in is not a metadata
 * header, the function returns NULL to indicate this. The linktype remains
 * unchanged in this case.
 *
 * This function allows the user to iterate through metadata headers which are
 * sometimes present before the actual packet as it was received on the wire.
 * Examples of metadata headers include the Linux SLL header and the Radiotap
 * wireless monitoring header.
 *
 * If the metadata header passed into this function is truncated, this 
 * function will return NULL, and remaining will be set to 0.
 *
 * If there are 0 bytes of payload following the provided metadata header, the 
 * function will return a pointer to where the header would otherwise be and 
 * remaining will be 0.
 *
 * Therefore, be sure to check the value of remaining after calling this
 * function!
 *
 * @param[in] meta A pointer to a metadata header
 * @param[in,out] linktype The linktype of meta (updated to indicate the
 * linktype of the returned header if applicable).
 * @param[in,out] remaining The number of bytes after the meta pointer.
 * @return A pointer to the payload of the metadata header. If meta is not a
 * pointer to a metadata header, NULL is returned and linktype remains
 * unchanged.
 *
 * All parameters are mandatory. 
 */
 void *trace_get_payload_from_meta(const void *meta,
                libtrace_linktype_t *linktype,
                uint32_t *remaining);


/** Get a pointer to the layer 2 header. Generally this is the first byte of the
 * packet as it was seen on the wire.
 * 
 * This function takes a libtrace packet and skips over any metadata headers if
 * present (such as Linux SLL or Radiotap) and returns a pointer to the first
 * byte of the packet that was actually received by the network interface.
 *
 * @param packet The libtrace packet to find the layer 2 header for
 * @param[out] linktype The linktype of the returned layer 2 header
 * @param[out] remaining The number of bytes left in the packet after the
 * returned pointer.
 * @return A pointer to the first byte of the packet as it was seen on the
 * wire. If no layer 2 header is present, this function will return NULL.
 *
 * remaining may be NULL, otherwise it will be filled in by the function.
 */
 void *trace_get_layer2(const libtrace_packet_t *packet,
                libtrace_linktype_t *linktype,
                uint32_t *remaining);

/** Gets a pointer to the next header following a layer 2 header
 *
 * @param l2            	The pointer to the current layer 2 header
 * @param linktype		The type of the layer 2 header
 * @param[out] ethertype 	An optional output variable of the ethernet type of the new header
 * @param[in,out] remaining 	Updated with the number of captured bytes 
				remaining
 *
 * @return A pointer to the header following the provided layer 2 header, or 
 * NULL if no subsequent header is present.
 *
 * Remaining must point to the number of bytes captured from the layer 2 header
 * and beyond.  It will be decremented by the number of bytes skipped to find
 * the payload.
 *
 * If the layer 2 header is complete but there are zero bytes of payload after 
 * the end of the header, a pointer to where the payload would be is returned 
 * and remaining will be set to 0.  If the layer 2 header is incomplete 
 * (truncated), then NULL is returned and remaining will be set to 0. 
 * Therefore, it is very important to check the value of remaining after
 * calling this function. 
 *
 */
 void *trace_get_payload_from_layer2(void *l2,
		libtrace_linktype_t linktype,
		uint16_t *ethertype,
		uint32_t *remaining);


/** Get a pointer to the layer 3 (e.g. IP) header.
 * @param packet  The libtrace packet to find the layer 3 header for
 * @param[out] ethertype The ethertype of the layer 3 header
 * @param[out] remaining The amount of captured data remaining in the packet starting from the returned pointer, i.e. including the layer 3 header.
 *
 * @return A pointer to the layer 3 header. If no layer 3 header is present in
 * the packet, NULL is returned. If the layer 3 header is truncated, a valid 
 * pointer will still be returned so be sure to check the value of remaining
 * before attempting to process the returned header.
 *
 * remaining may be NULL, otherwise it will be set to the number of captured
 * bytes after the pointer returned.
 */
 
void *trace_get_layer3(const libtrace_packet_t *packet,
		uint16_t *ethertype, uint32_t *remaining);

/** Calculates the expected IP checksum for a packet.
 * @param packet	The libtrace packet to calculate the checksum for
 * @param[out] csum	The checksum that is calculated by this function. This
 * 			may not be NULL.
 * 
 * @return A pointer to the original checksum field within the IP
 * header. If the checksum field is not present in the packet, NULL is returned.
 *
 * @note The return value points to the checksum that exists within the current
 * packet. The value in csum is the value that the checksum should be, given
 * the current packet contents.  
 *
 * @note This function involves the use of a memcpy, so be careful about
 * calling it excessively if performance is a concern for you.
 * 
 * New in libtrace 3.0.17
 */
 uint16_t *trace_checksum_layer3(libtrace_packet_t *packet, 
		uint16_t *csum);

/** Calculates the expected checksum for the transport header in a packet.
 * @param packet	The libtrace packet to calculate the checksum for
 * @param[out] csum	The checksum that is calculated by this function. This
 * 			may not be NULL.
 * 
 * @return A pointer to the original checksum field within the transport 
 * header. If the checksum field is not present in the packet, NULL is returned.
 *
 * @note The return value points to the checksum that exists within the current
 * packet. The value in csum is the value that the checksum should be, given
 * the current packet contents.  
 *
 * @note This function involves the use of a memcpy, so be careful about
 * calling it excessively if performance is a concern for you.
 * 
 * @note Because transport checksums are calculated across the entire payload,
 * truncated packets will result in NULL being returned.
 *
 * This function will determine the appropriate checksum for whatever transport
 * layer header is present in the provided packet. At this stage, this only
 * currently works for TCP, UDP and ICMP packets.
 *
 * Be wary of TCP checksum offloading if you are examining the checksum of
 * packets captured on the same host that generated them!
 *
 * New in libtrace 3.0.17
 */
 uint16_t *trace_checksum_transport(libtrace_packet_t *packet,
                uint16_t *csum);

/** Calculates the fragment offset in bytes for an IP packet
 * @param packet        The libtrace packet to calculate the offset for
 * @param[out] more     A boolean flag to indicate whether there are more
 *                      fragments after the current packet.
 * @return The fragment offset for the packet in bytes. If the packet is not
 * an IP packet or the fragment offset is not present in packet, the return
 * value will be 0.
 *
 * @note The value returned is in bytes, not 8-octet units as it is stored
 * in the fragment offset field in the headers. In other words, libtrace
 * automatically does the multiplication for you.
 *
 * The value passed in for 'more' does not matter; it will be overwritten
 * with the value of the More Fragments flag from the IP header.
 *
 * New in libtrace 3.0.20
 */
 uint16_t trace_get_fragment_offset(const libtrace_packet_t *packet,
                uint8_t *more); 

/** Gets a pointer to the transport layer header (if any)
 * @param packet   The libtrace packet to find the transport header for
 * @param[out] proto	The protocol present at the transport layer.
 * @param[out] remaining The amount of captured data remaining in the packet 
 * starting from the returned pointer, i.e. including the transport header.
 *
 * @return A pointer to the transport layer header. If no transport header is
 * present in the packet, NULL is returned. If the transport header is 
 * truncated, a valid pointer will still be returned so be sure to check the
 * value of remaining before attempting to process the returned header.
 *
 * remaining may be NULL, otherwise it will be set to the number of captured
 * bytes after the returned pointer.
 *
 * @note proto may be NULL if proto is unneeded.
 */
 void *trace_get_transport(const libtrace_packet_t *packet, 
		uint8_t *proto, uint32_t *remaining);

/** Gets a pointer to the payload following an IPv4 header
 * @param ip            The IPv4 Header
 * @param[out] proto	The protocol of the header following the IPv4 header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the transport layer header, or NULL if no subsequent 
 * header is present.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the IPv4 header (including the
 * IPv4 header itself).
 *
 * remaining will be decremented by the size of the IPv4 header (including any 
 * options). If the IPv4 header is complete but there are zero bytes of payload
 * after the IPv4 header, a pointer to where the payload would be is returned 
 * and remaining will be set to 0.  If the IPv4 header is incomplete, NULL will
 * be returned and remaining will be set to 0. Therefore, it is very important
 * to check the value of remaining after calling this function.
 *
 * proto may be NULL, in which case it won't be updated.
 *
 * @note This is similar to trace_get_transport_from_ip in libtrace2
 */
 void *trace_get_payload_from_ip(libtrace_ip_t *ip, uint8_t *proto,
		uint32_t *remaining);

/** Gets a pointer to the payload following an IPv6 header
 * @param ipptr         The IPv6 Header
 * @param[out] proto	The protocol of the header following the IPv6 header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the transport layer header, or NULL if no subsequent 
 * header is present.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the IPv6 header (including the
 * IPv6 header itself).
 *
 * remaining will be decremented by the size of the IPv6 header (including any 
 * options). If the IPv6 header is complete but there are zero bytes of payload
 * after the IPv6 header, a pointer to where the payload would be is returned 
 * and remaining will be set to 0.  If the IPv6 header is incomplete, NULL will
 * be returned and remaining will be set to 0. Therefore, it is very important
 * to check the value of remaining after calling this function.
 *
 * proto may be NULL, in which case it won't be updated.
 *
 */
 void *trace_get_payload_from_ip6(libtrace_ip6_t *ipptr,
                uint8_t *proto, uint32_t *remaining);

/** Gets a pointer to the payload following a link header
 * @param linkptr       A pointer to the link layer header
 * @param linktype	The linktype of the link header being examined
 * @param[out] type	An output variable for the ethernet type
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the header following the link header, or NULL if no
 * subsequent header is present.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the link header (including the
 * link header itself). remaining will be updated to contain the number of 
 * bytes remaining after the link header has been skipped.
 *
 * @deprecated This function is deprecated: you should be using 
 * trace_get_payload_from_layer2() or trace_get_payload_from_meta() instead.
 *
 */
 void *trace_get_payload_from_link(void *linkptr,
		libtrace_linktype_t linktype, 
		uint16_t *type, uint32_t *remaining);

/** Gets a pointer to the payload following an 802.1q (VLAN) header.
 * @param vlan      A pointer to the VLAN header
 * @param[out] type  The ethernet type, replaced with the VLAN ether type
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the header beyond the VLAN header, if one is present.
 * Otherwise, returns NULL.  
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the VLAN header (including the
 * VLAN header itself). remaining will be updated to contain the number of 
 * bytes remaining after the VLAN header has been skipped.
 *
 * If the VLAN header is complete but there are zero bytes of payload after 
 * the VLAN header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the VLAN header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * type will be set to the ethertype of the VLAN payload. This parameter is not
 * mandatory, but is highly recommended.
 *
 */
 void *trace_get_payload_from_vlan(
                void *vlan, uint16_t *type, uint32_t *remaining);

/** Gets a pointer to the payload following an MPLS header.
 * @param mpls      A pointer to the MPLS header
 * @param[out] type  The ethernet type, replaced by the ether type of the
 * returned header - 0x0000 if an Ethernet header is returned
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the header beyond the MPLS label, if one is present. 
 * Will return NULL if there is not enough bytes remaining to skip past the 
 * MPLS label.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the MPLS header (including the
 * MPLS header itself). remaining will be updated to contain the number of 
 * bytes remaining after the MPLS header has been skipped.
 *
 * If the MPLS header is complete but there are zero bytes of payload after 
 * the MPLS header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the MPLS header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * type will be set to the ethertype of the MPLS payload. This parameter is
 * mandatory - it may not be NULL.
 *
 * @note This function will only remove one MPLS label at a time - the type
 * will be set to 0x8847 if there is another MPLS label following the one
 * skipped by this function.
 *
 */
 void *trace_get_payload_from_mpls(
                void *mpls, uint16_t *type, uint32_t *remaining);

/** Gets a pointer to the payload following a PPPoE header
 * @param pppoe      A pointer to the PPPoE header
 * @param[out] type  The ethernet type, replaced by the ether type of the
 * returned header - 0x0000 if an Ethernet header is returned
 * @param[in,out] remaining  Updated with the number of captured bytes remaining
 *
 * @return A pointer to the header beyond the PPPoE header. NOTE that this
 * function will also skip over the PPP header that will immediately follow
 * the PPPoE header. This function will return NULL if there are not enough
 * bytes remaining to skip past both the PPPoE and PPP headers.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the PPPoE header (including the
 * PPPoE header itself). remaining will be updated to contain the number of 
 * bytes remaining after the PPPoE and PPP headers have been removed.
 *
 * If the PPPoE and PPP headers are complete but there are zero bytes of 
 * payload after the PPP header, a pointer to where the payload would be is 
 * returned and remaining will be set to 0.  If the PPPoE or PPP header is 
 * incomplete, NULL will be returned and remaining will be set to 0. Therefore, 
 * it is important to check the value of remaining after calling this function.
 *
 * type will be set to the ether type of the PPP payload. This parameter is
 * mandatory - it may not be NULL.
 *
 */
 void *trace_get_payload_from_pppoe(
		void *pppoe, uint16_t *type, uint32_t *remaining);

/** Gets a pointer to the payload following a TCP header
 * @param tcp           A pointer to the TCP header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the TCP payload, or NULL if the TCP header is truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the TCP header (including the
 * TCP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the TCP header has been skipped.
 *
 * If the TCP header is complete but there are zero bytes of payload after 
 * the TCP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the TCP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 */
 void *trace_get_payload_from_tcp(libtrace_tcp_t *tcp, 
		uint32_t *remaining);

/** Gets a pointer to the payload following a UDP header
 * @param udp           A pointer to the UDP header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the UDP payload, or NULL if the UDP header is truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the UDP header (including the
 * UDP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the UDP header has been skipped.
 *
 * If the UDP header is complete but there are zero bytes of payload after 
 * the UDP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the UDP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 */
 void *trace_get_payload_from_udp(libtrace_udp_t *udp, uint32_t *remaining);

/** Gets a pointer to the payload following a ICMP header
 * @param icmp           A pointer to the ICMP header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the ICMP payload, or NULL if the ICMP header is 
 * truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the ICMP header (including the
 * ICMP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the ICMP header has been skipped.
 *
 * If the ICMP header is complete but there are zero bytes of payload after 
 * the ICMP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the ICMP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * @note In the case of some ICMP messages, the payload may be the IP header
 * from the packet that triggered the ICMP message. 
 *
 */
 void *trace_get_payload_from_icmp(libtrace_icmp_t *icmp, 
		uint32_t *remaining);

/** Gets a pointer to the payload following a ICMPv6 header
 * @param icmp           A pointer to the ICMPv6 header
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the ICMPv6 payload, or NULL if the ICMPv6 header is 
 * truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the ICMPv6 header (including the
 * ICMP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the ICMPv6 header has been skipped.
 *
 * If the ICMPv6 header is complete but there are zero bytes of payload after 
 * the header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the ICMPv6 header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * @note In the case of some ICMPv6 messages, the payload may be the IP header
 * from the packet that triggered the ICMP message. 
 *
 */
 void *trace_get_payload_from_icmp6(libtrace_icmp6_t *icmp, 
		uint32_t *remaining);

/** Gets a pointer to the payload following a GRE header
 * @param         gre       A pointer to the beginning of the GRE header.
 * @param[in,out] remaining Updated with the number of captured bytes remaining.
 *
 * @return A pointer to the GRE payload, or NULL if the GRE header is truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the GRE header (including the
 * GRE header itself). remaining will be updated to contain the number of
 * bytes remaining after the GRE header has been skipped.
 *
 * If the GRE header is complete but there are zero bytes of payload after 
 * the header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the GRE header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 */
 void *trace_get_payload_from_gre(libtrace_gre_t *gre,
                uint32_t *remaining);

/** Gets a pointer to the payload following a VXLAN header
 * @param         udp       A pointer to the beginning of the UDP header.
 * @param[in,out] remaining Updated with the number of captured bytes remaining.
 *
 * @return A pointer to the beginning of the VXLAN header, or NULL if the UDP
 * header is truncated, or this is not a VXLAN packet.
 *
 */
 libtrace_vxlan_t *trace_get_vxlan_from_udp(libtrace_udp_t *udp,
                uint32_t *remaining);

/** Gets a pointer to the payload following a VXLAN header
 * @param         vxlan       A pointer to the beginning of the VXLAN header.
 * @param[in,out] remaining Updated with the number of captured bytes remaining.
 *
 * @return A pointer to the VXLAN payload, or NULL if the VXLAN header is
 * truncated.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the VXLAN header (including the
 * VXLAN header itself). remaining will be updated to contain the number of
 * bytes remaining after the VXLAN header has been skipped.
 *
 * If the VXLAN header is complete but there are zero bytes of payload after
 * the header, a pointer to where the payload would be is returned and
 * remaining will be set to 0.  If the VXLAN header is incomplete, NULL will be
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 */
 void *trace_get_payload_from_vxlan(libtrace_vxlan_t *vxlan,
                uint32_t *remaining);

/** Get a pointer to the TCP header (if present)
 * @param packet  	The packet to get the TCP header from
 *
 * @return A pointer to the TCP header, or NULL if there is not a complete TCP
 * header present in the packet.
 *
 * This is a short-cut function enabling quick and easy access to the TCP
 * header if that is all you care about. However, we recommend the use of the
 * more generic trace_get_transport() function instead.
 *
 * @note Unlike trace_get_transport(), this function will return NULL if the
 * TCP header is incomplete or truncated.
 */
 
libtrace_tcp_t *trace_get_tcp(libtrace_packet_t *packet);

/** Get a pointer to the TCP header following an IPv4 header (if present)
 * @param ip		The IP header to find the subsequent TCP header for
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the TCP header, or NULL if no TCP header is present in 
 * the packet.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the IP header (including the
 * IP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the IP header has been skipped.
 *
 * If the IP header is complete but there are zero bytes of payload after 
 * the IP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the IP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * @note This function is rather redundant now that the layer 3 header is
 * cached. There should be no performance advantage for the user to call this
 * function over just calling trace_get_transport().
 *
 * @note The last parameter has changed from libtrace2
 */
 
libtrace_tcp_t *trace_get_tcp_from_ip(libtrace_ip_t *ip, uint32_t *remaining);

/** Get a pointer to the UDP header (if present)
 * @param packet  	The packet to get the UDP header from
 *
 * @return A pointer to the UDP header, or NULL if there is not a complete UDP
 * header present in the packet.
 *
 * This is a short-cut function enabling quick and easy access to the UDP
 * header if that is all you care about. However, we recommend the use of the
 * more generic trace_get_transport() function instead.
 *
 * @note Unlike trace_get_transport(), this function will return NULL if the
 * UDP header is incomplete or truncated.
 */
 
libtrace_udp_t *trace_get_udp(libtrace_packet_t *packet);

/** Get a pointer to the UDP header following an IPv4 header (if present)
 * @param ip		The IP header to find the subsequent UDP header for
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the UDP header, or NULL if no UDP header is present in 
 * the packet.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the IP header (including the
 * IP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the IP header has been skipped.
 *
 * If the IP header is complete but there are zero bytes of payload after 
 * the IP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the IP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * @note This function is rather redundant now that the layer 3 header is
 * cached. There should be no performance advantage for the user to call this
 * function over just calling trace_get_transport().
 *
 * @note The last parameter has changed from libtrace2
 */
 
libtrace_udp_t *trace_get_udp_from_ip(libtrace_ip_t *ip,uint32_t *remaining);

/** Get a pointer to the ICMP header (if present)
 * @param packet  	The packet to get the ICMP header from
 *
 * @return A pointer to the ICMP header, or NULL if there is not a complete 
 * ICMP header present in the packet.
 *
 * This is a short-cut function enabling quick and easy access to the ICMP
 * header if that is all you care about. However, we recommend the use of the
 * more generic trace_get_transport() function instead.
 *
 * @note Unlike trace_get_transport(), this function will return NULL if the
 * ICMP header is incomplete or truncated.
 */
 
libtrace_icmp_t *trace_get_icmp(libtrace_packet_t *packet);

/** Get a pointer to the ICMPv6 header (if present)
 * @param packet  	The packet to get the ICMPv6 header from
 *
 * @return A pointer to the ICMPv6 header, or NULL if there is not a complete 
 * ICMP header present in the packet.
 *
 * This is a short-cut function enabling quick and easy access to the ICMPv6
 * header if that is all you care about. However, we recommend the use of the
 * more generic trace_get_transport() function instead.
 *
 * @note Unlike trace_get_transport(), this function will return NULL if the
 * ICMPv6 header is incomplete or truncated.
 */
 
libtrace_icmp6_t *trace_get_icmp6(libtrace_packet_t *packet);

/** Get a pointer to the ICMP header following an IPv4 header (if present)
 * @param ip		The IP header to find the subsequent ICMP header for
 * @param[in,out] remaining Updated with the number of captured bytes remaining
 *
 * @return A pointer to the ICMP header, or NULL if no UDP header is present in 
 * the packet.
 *
 * When calling this function, remaining must contain the number of captured
 * bytes remaining in the packet starting from the IP header (including the
 * IP header itself). remaining will be updated to contain the number of 
 * bytes remaining after the IP header has been skipped.
 *
 * If the IP header is complete but there are zero bytes of payload after 
 * the IP header, a pointer to where the payload would be is returned and 
 * remaining will be set to 0.  If the IP header is incomplete, NULL will be 
 * returned and remaining will be set to 0. Therefore, it is important to check
 * the value of remaining after calling this function.
 *
 * @note This function is rather redundant now that the layer 3 header is
 * cached. There should be no performance advantage for the user to call this
 * function over just calling trace_get_transport().
 *
 * @note The last parameter has changed from libtrace2
 */
 
libtrace_icmp_t *trace_get_icmp_from_ip(libtrace_ip_t *ip,uint32_t *remaining);

/** Get a pointer to the OSPF header (if present)
 * @param packet	The packet to get the OSPF header from
 * @param[out] version	The OSPF version number
 * @param[out] remaining	Updated with the number of captured bytes remaining
 * @return A pointer to the start of the OSPF header or NULL if there is no 
 * complete OSPF header present in the packet
 *
 * This is a short-cut function enabling quick and easy access to the OSPF
 * header. If you only care about the OSPF header, this function may be useful
 * but we otherwise recommend the use of the more generic trace_get_transport()
 * function instead.
 *
 * Upon return, 'version' is updated to contain the OSPF version number for the
 * packet so that the returned pointer may be cast to the correct type.
 * The version parameter MUST contain a valid pointer; it MUST NOT be NULL.
 *
 * 'remaining' is also set to contain the number of captured bytes remaining
 * starting from the pointer returned by this function. 
 *
 * @note Unlike trace_get_transport(), this function will return NULL if the
 * OSPF header is incomplete or truncated.
 */
 
void *trace_get_ospf_header(libtrace_packet_t *packet, uint8_t *version,
		uint32_t *remaining);

/** Get a pointer to the contents of the OSPF packet *after* the OSPF header
 * @param header	The OSPF header to get the OSPF contents from
 * @param[out] ospf_type	The OSPF packet type
 * @param[in, out] remaining	Updated with the number of captured bytes remaining
 * @return A pointer to the first byte after the OSPF header.
 *
 * This function returns a void pointer that can be cast appropriately
 * based on the ospf_type. For example, if the ospf_type is TRACE_OSPF_HELLO 
 * then the return pointer should be cast as a libtrace_ospf_hello_v2_t 
 * structure.
 *
 * If the OSPF header is truncated, then NULL will be returned. If the OSPF 
 * contents are missing or truncated, the pointer to the start of the content 
 * will still be returned so be careful to check the value of remaining.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the OSPF header. It will be updated
 * to contain the number of bytes remaining from the start of the OSPF contents.
 *
 * @note This function only works for OSPF version 2 packets.
 * @note Use trace_get_first_ospf_lsa_v2_from_X() and trace_get_next_ospf_lsa_v2() to read the LSAs from Link State Update packets.
 * @note Use trace_get_first_ospf_lsa_v2_from_X() and trace_get_next_ospf_lsa_header_v2() to read the LSA headers from Link State Ack packets.
 * 
 */ 
 
void *trace_get_ospf_contents_v2(libtrace_ospf_v2_t *header,
		uint8_t *ospf_type, uint32_t *remaining);

/** Get a pointer to the start of the first LSA contained within an LS Update packet
 * @param ls_update	Pointer to the LS Update header
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @return A pointer to the first LSA in the packet.
 *
 * This function simply skips past the LS Update header to provide a suitable
 * pointer to pass into trace_get_next_ospf_lsa_v2.
 *
 * If the OSPF packet is truncated, then NULL will be returned.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the LS Update header. It will be 
 * updated to contain the number of bytes remaining from the start of the 
 * first LSA.
 *
 * @note This function only works for OSPF version 2 packets.
 * @note trace_get_next_ospf_lsa_v2() should be subequently used to process the LSAs
 */
  
unsigned char *trace_get_first_ospf_lsa_from_update_v2(
                libtrace_ospf_ls_update_t *ls_update,
                uint32_t *remaining);

/** Get a pointer to the start of the first LSA contained within an Database Description packet
 * @param db_desc	Pointer to the Database Description header
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @return A pointer to the first LSA in the packet.
 *
 * This function simply skips past the Database Description header to provide a 
 * suitable pointer to pass into trace_get_next_ospf_lsa_header_v2.
 *
 * If the OSPF packet is truncated, then NULL will be returned.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the Database Description header. It 
 * will be updated to contain the number of bytes remaining from the start of 
 * the first LSA.
 *
 * @note This function only works for OSPF version 2 packets.
 * @note trace_get_next_ospf_lsa_header_v2() should be subequently used to process the LSA headers
 */
  
unsigned char *trace_get_first_ospf_lsa_from_db_desc_v2(
                libtrace_ospf_db_desc_v2_t *db_desc,
                uint32_t *remaining);

/** Get a pointer to the start of the first link contained within a Router LSA
 * @param lsa	Pointer to the Router LSA
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @return A pointer to the first link in the packet.
 *
 * This function simply skips past the Router LSA header to provide a 
 * suitable pointer to pass into trace_get_next_ospf_link_v2.
 *
 * If the OSPF packet is truncated, then NULL will be returned.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the Router LSA (not including the LSA 
 * header) header. It will be updated to contain the number of bytes remaining 
 * from the start of the first Link.
 *
 * @note This function only works for OSPF version 2 packets.
 * @note trace_get_next_ospf_link_v2() should be subequently used to process
 * the links
 */
 
unsigned char *trace_get_first_ospf_link_from_router_lsa_v2(
		libtrace_ospf_router_lsa_v2_t *lsa,
		uint32_t *remaining);

/** Parses an OSPF Router LSA Link and finds the next Link (if there is one)
 * @param[in,out] current	Pointer to the next Link (updated by this function)
 * @param[out] link		Set to point to the Link located at the original value of current
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @param[out] link_len		Set to the size of the Link pointed to by 'link'
 * @return 0 if there are no more links after the current one, 1 otherwise.
 *
 * When called, 'current' MUST point to an OSPF Router LSA link. Ideally, this
 * would come from either a call to 
 * trace_get_first_ospf_link_from_router_lsa_v2() or a previous call of this
 * function. 
 *
 * 'link' will be set to the value of 'current', so that the caller may then
 * do any processing they wish on that particular link. 'current' is advanced
 * to point to the next link and 'link_len' is updated to report the size of
 * the original link.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the Link pointed to by 'current'.
 * It will be updated to contain the number of bytes remaining from the start 
 * of the next link.
 *
 * If this function returns 0 but 'link' is NOT NULL, that link is still valid
 * but there are no more links after this one.
 * If this function returns 0 AND link is NULL, the link is obviously not 
 * suitable for processing. 
 *
 * @note This function only works for OSPF version 2 packets.
 */
 
int trace_get_next_ospf_link_v2(unsigned char **current,
		libtrace_ospf_link_v2_t **link,
		uint32_t *remaining,
		uint32_t *link_len);

/** Parses an OSPF LSA and finds the next LSA (if there is one)
 * @param[in,out] current 	Pointer to the next LSA (updated by this function)
 * @param[out] lsa_hdr		Set to point to the header of the LSA located at the original value of current
 * @param[out] lsa_body		Set to point to the body of the LSA located at the original value of current
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @param[out] lsa_type		Set to the type of the LSA located at the original value of current
 * @param[out] lsa_length	Set to the size of the LSA located at the original value of current
 * @return 1 if there are more LSAs after the current one, 0 if there are no more LSAs to come, and -1 if the current LSA is incomplete, truncated or invalid.
 *
 * When called, 'current' MUST point to an OSPF LSA. Ideally, this would come 
 * from either a call to trace_get_first_ospf_lsa_from_update_v2() or a 
 * previous call of this function. 
 *
 * This function should only be used to access COMPLETE LSAs, i.e. LSAs that
 * have both a header and a body. In OSPFv2, only the LSAs contained within
 * LSA Update packets meet this requirement. trace_get_next_ospf_lsa_header_v2
 * should be used to read header-only LSAs, e.g. those present in LS Acks.
 *
 * 'lsa_hdr' will be set to the value of 'current', so that the caller may then
 * do any processing they wish on that particular LSA. 'lsa_body' will be set
 * to point to the first byte after the LSA header. 'current' is advanced
 * to point to the next LSA. 'lsa_length' is updated to contain the size of
 * the parsed LSA, while 'lsa_type' is set to indicate the LSA type.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the LSA pointed to by 'current'.
 * It will be updated to contain the number of bytes remaining from the start 
 * of the next LSA.
 *
 * If this function returns 0 but 'lsa_hdr' is NOT NULL, that LSA is still 
 * valid but there are no more LSAs after this one.
 * If this function returns 0 AND 'lsa_hdr' is NULL, the LSA is obviously not 
 * suitable for processing. 
 *
 * It is also recommended to check the value of 'lsa_body' before 
 * de-referencing it. 'lsa_body' will be set to NULL if there is only an LSA
 * header present.
 * 
 * @note This function only works for OSPF version 2 packets.
 *
 */
 
int trace_get_next_ospf_lsa_v2(unsigned char **current,
                libtrace_ospf_lsa_v2_t **lsa_hdr,
                unsigned char **lsa_body,
                uint32_t *remaining,
                uint8_t *lsa_type,
                uint16_t *lsa_length);

/** Parses an OSPF LSA header and finds the next LSA (if there is one)
 * @param[in,out] current 	Pointer to the next LSA (updated by this function)
 * @param[out] lsa_hdr		Set to point to the header of the LSA located at the original value of current
 * @param[in,out] remaining	Updated with the number of captured bytes remaining
 * @param[out] lsa_length	Set to the size of the LSA located at the original value of current
 * @return 1 if there are more LSAs after the current one, 0 if there are no more LSAs to come, and -1 if the current LSA is incomplete, truncated or invalid.
 *
 * When called, 'current' MUST point to an OSPF LSA. Ideally, this would come 
 * from either a call to trace_get_first_ospf_lsa_from_db_desc_v2() or a 
 * previous call of this function. 
 *
 * This function should only be used to access LSA headers, i.e. LSAs that
 * have a header only. In OSPFv2, the LSAs contained within LSA Ack and 
 * Database Description packets meet this requirement. 
 * trace_get_next_ospf_lsa_v2 should be used to read full LSAs, e.g. those 
 * present in LS Updates.
 *
 * 'lsa_hdr' will be set to the value of 'current', so that the caller may then
 * do any processing they wish on that particular LSA header. 'current' is 
 * advanced to point to the next LSA header. 'lsa_length' is updated to contain 
 * the size of the parsed LSA header.
 *
 * 'remaining' MUST be set to the amount of bytes remaining in the captured
 * packet starting from the beginning of the LSA pointed to by 'current'.
 * It will be updated to contain the number of bytes remaining from the start 
 * of the next LSA.
 *
 * If this function returns 0 but 'lsa_hdr' is NOT NULL, that LSA is still 
 * valid but there are no more LSAs after this one.
 * If this function returns 0 AND 'lsa_hdr' is NULL, the LSA is obviously not 
 * suitable for processing. 
 *
 * @note This function only works for OSPF version 2 packets.
 *
 */
 
int trace_get_next_ospf_lsa_header_v2(unsigned char **current,
                libtrace_ospf_lsa_v2_t **lsa_hdr,
                uint32_t *remaining,
                uint8_t *lsa_type,
                uint16_t *lsa_length); 

/** Extracts the metric field from an AS External LSA packet
 *
 * @param as_lsa	The AS External LSA
 * @returns The value of the metric field
 *
 * The metric field in the AS External LSA packet is a 24-bit value, which
 * is difficult to extract correctly. To avoid byte-ordering issues, use this
 * function which will extract the correct value for you.
 */
 
uint32_t trace_get_ospf_metric_from_as_external_lsa_v2(
		libtrace_ospf_as_external_lsa_v2_t *as_lsa);

/** Extracts the metric field from a Summary LSA packet
 *
 * @param sum_lsa	The Summary LSA
 * @returns The value of the metric field
 *
 * The metric field in the Summary LSA packet is a 24-bit value, which
 * is difficult to extract correctly. To avoid byte-ordering issues, use this
 * function which will extract the correct value for you.
 */
 
uint32_t trace_get_ospf_metric_from_summary_lsa_v2(
		libtrace_ospf_summary_lsa_v2_t *sum_lsa);


/** Gets the destination MAC address for a given packet
 * @param packet  	The packet to extract the destination MAC address from
 *
 * @return A pointer to the destination MAC address field in the layer 2 
 * header, (or NULL if there is no destination MAC address or layer 2 header
 * available)
 *
 * @note This is a zero-copy function, so the memory that the returned pointer
 * points to is part of the packet itself. 
 */
 
uint8_t *trace_get_destination_mac(libtrace_packet_t *packet);

/** Gets the source MAC address for a given packet
 * @param packet  	The packet to extract the source MAC address from
 *
 * @return A pointer to the source MAC address field in the layer 2 
 * header, (or NULL if there is no source MAC address or layer 2 header
 * available)
 *
 * @note This is a zero-copy function, so the memory that the returned pointer
 * points to is part of the packet itself. 
 */
 
uint8_t *trace_get_source_mac(libtrace_packet_t *packet);

/** Get the source IP address for a given packet
 * @param packet  	The packet to extract the source IP address from
 * @param addr		A pointer to a sockaddr structure to store the address 
 * 			in. If NULL, static storage is used instead.
 * @return A pointer to a sockaddr holding a v4 or v6 IP address or on some 
 * platforms a sockaddr holding a MAC address. Returns NULL if no source IP
 * address was available.
 *
 * @note The best way to use this function is to pass in a pointer to the
 * struct sockaddr_storage for the addr parameter. This will avoid problems
 * with trying to shoe-horn an IPv6 address into a sockaddr that only supports
 * IPv4.
 */
 
struct sockaddr *trace_get_source_address(const libtrace_packet_t *packet,
		struct sockaddr *addr);

/** Get the source IP address for a packet and convert it into a string
 * @param packet	The packet to extract the source IP address from
 * @param space		A pointer to a character buffer to store the address 
 *			in. If NULL, static storage is used instead.
 * @param spacelen	The size of the buffer passed in via 'space'. Set this
 *			to zero if you are going to pass in a NULL buffer.
 * @return A pointer to a character buffer containing the string representation
 * of the source IP address. For packets where there is no suitable IP address,
 * the source MAC will be returned instead. Returns NULL if no valid address
 * is available.
 *
 * @note Be wary of the possibility of the address being an IPv6 address - make
 * sure your buffer is large enough!
 *
 * New in libtrace 3.0.17.
 */
 
char *trace_get_source_address_string(const libtrace_packet_t *packet,
		char *space, int spacelen);

/** Get the destination IP address for a given packet
 * @param packet  	The packet to extract the destination IP address from
 * @param addr		A pointer to a sockaddr structure to store the address 
 * 			in. If NULL, static storage is used instead.
 * @return A pointer to a sockaddr holding a v4 or v6 IP address or on some 
 * platforms a sockaddr holding a MAC address. Returns NULL if no destination 
 * IP address was available.
 *
 * @note The best way to use this function is to pass in a pointer to the
 * struct sockaddr_storage for the addr parameter. This will avoid problems
 * with trying to shoe-horn an IPv6 address into a sockaddr that only supports
 * IPv4.
 */
 
struct sockaddr *trace_get_destination_address(const libtrace_packet_t *packet,
		struct sockaddr *addr);

/** Get the destination IP address for a packet and convert it into a string
 * @param packet	The packet to extract the destination IP address from
 * @param space		A pointer to a character buffer to store the address 
 *			in. If NULL, static storage is used instead.
 * @param spacelen	The size of the buffer passed in via 'space'. Set this
 *			to zero if you are going to pass in a NULL buffer.
 * @return A pointer to a character buffer containing the string representation
 * of the destination IP address. For packets where there is no suitable IP 
 * address, the destination MAC will be returned instead. Returns NULL if no 
 * valid address is available.
 *
 * @note Be wary of the possibility of the address being an IPv6 address - make
 * sure your buffer is large enough!
 *
 * New in libtrace 3.0.17.
 */
 
char *trace_get_destination_address_string(const libtrace_packet_t *packet,
		char *space, int spacelen);

/** Parses an IP or TCP option
 * @param[in,out] ptr	The pointer to the current option
 * @param[in,out] len	The total length of all the remaining options
 * @param[out] type	The type of the option
 * @param[out] optlen 	The length of the option
 * @param[out] data	The data of the option
 *
 * @return bool true if there is another option (and the fields are filled in)
 *               or false if this was the last option.
 *
 * This updates ptr to point to the next option after this one, and updates
 * len to be the number of bytes remaining in the options area.  Type is updated
 * to be the code of this option, and data points to the data of this option,
 * with optlen saying how many bytes there are.
 *
 * @note Beware of fragmented packets.
 */
 int trace_get_next_option(unsigned char **ptr,int *len,
			unsigned char *type,
			unsigned char *optlen,
			unsigned char **data);

/*@}*/

/** @name Time
 * These functions deal with the timestamp describing when a packet was 
 * captured and can convert it into various formats
 * @{
 */

/** Get the packet timestamp in the DAG time format 
 * @param packet  	The packet to extract the timestamp from
 *
 * @return a 64 bit timestamp in DAG ERF format (upper 32 bits are the seconds
 * past 1970-01-01, the lower 32bits are partial seconds)
 */
 
uint64_t trace_get_erf_timestamp(const libtrace_packet_t *packet);

/** Get the packet timestamp as a struct timeval
 * @param packet  	The packet to extract the timestamp from
 *
 * @return The time that this packet was captured in a struct timeval
 */ 
 
//struct timeval trace_get_timeval(const libtrace_packet_t *packet);

/** Get the packet timestamp as a struct timespec
 * @param packet  	The packet to extract the timestamp from
 *
 * @return The time that this packet was captured in a struct timespec
 */ 
 
//struct timespec trace_get_timespec(const libtrace_packet_t *packet);

/** Get the packet timestamp in floating point seconds
 * @param packet  	The packet to extract the timestamp from
 *
 * @return time that this packet was seen in 64-bit floating point seconds from
 * the UNIX epoch (1970-01-01 00:00:00 UTC).
 */
 
double trace_get_seconds(const libtrace_packet_t *packet);

/** Seek within an input trace to a time specified in floating point seconds
 * @param trace		The input trace to seek within
 * @param seconds	The time to seek to, in floating point seconds
 * @return 0 on success, -1 if the seek fails. Use trace_perror() to determine
 * the error that occurred.
 *
 * This will make the next packet read to be the first packet to occur at or 
 * after the specified time.  This must be called in the configuration state 
 * (i.e. before trace_start() or after trace_pause()).
 *
 * The time format accepted by this function is 64-bit floating point seconds
 * since the UNIX epoch (1970-01-01 00:00:00 UTC), i.e. the same format as
 * trace_get_seconds().
 *
 * @note This function may be extremely slow.
 */
 int trace_seek_seconds(libtrace_t *trace, double seconds);

/** Seek within an input trace to a time specified as a timeval
 * @param trace		The input trace to seek within
 * @param tv		The time to seek to, as a timeval
 *
 * @return 0 on success, -1 if the seek fails. Use trace_perror() to determine
 * the error that occurred.
 *
 * This will make the next packet read to be the first packet to occur at or 
 * after the specified time.  This must be called in the configuration state 
 * (i.e. before trace_start() or after trace_pause()).
 *
 * @note This function may be extremely slow.
 */
 int trace_seek_timeval(libtrace_t *trace, struct timeval tv);

/** Seek within an input trace to a time specified as an ERF timestamp
 * @param trace		The input trace to seek within
 * @param ts		The time to seek to, as an ERF timestamp
 *
 * @return 0 on success, -1 if the seek fails. Use trace_perror() to determine
 * the error that occurred.
 *
 * This will make the next packet read to be the first packet to occur at or 
 * after the specified time.  This must be called in the configuration state 
 * (i.e. before trace_start() or after trace_pause()).
 *
 * The time format accepted by this function is the ERF timestamp, which is a
 * 64-bit value where the upper 32 bits are seconds since the UNIX epoch and
 * the lower 32 bits are partial seconds.
 *
 * @note This function may be extremely slow.
 */
 int trace_seek_erf_timestamp(libtrace_t *trace, uint64_t ts);

/*@}*/

/** @name Sizes
 * This section deals with finding or setting the various different lengths
 * that a packet can have, e.g. capture lengths, wire lengths, etc.
 * @{
 */
/** Get the current size of the packet (in bytes), taking into account any 
 * truncation or snapping that may have previously been performed. 
 *
 * @param packet  	The packet to determine the capture length for
 * @return The size of the packet read from the input trace, i.e. what is
 * actually available to libtrace at the moment.
 *
 * @note Most traces are header captures, so this value may not be the same
 * as the size of the packet when it was captured. Use trace_get_wire_length()
 * to get the original size of the packet.
 
 * @note This can (and often is) different for different packets in a trace!
 
 * @note This is sometimes called the "snaplen".
 *
 * @note The return size refers to the network-level payload of the packet and
 * does not include any capture framing headers. For example, an Ethernet 
 * packet with an empty TCP packet will return sizeof(ethernet_header) + 
 * sizeof(ip_header) + sizeof(tcp_header), but not the capture format
 * (pcap/erf/etc) header.
 */
 
size_t trace_get_capture_length(const libtrace_packet_t *packet);

/** Get the size of the packet as it was originally seen on the wire (in bytes).
 * @param packet  	The packet to determine the wire length for
 * @return The size of the packet as it was on the wire.
 *
 * @note This value may not be the same as the capture length, due to 
 * truncation.
 *
 * @note trace_get_wire_length \em includes  the Frame Check Sequence. This is 
 * different behaviour compared to most PCAP-based tools.
 *
 * @note The return size refers to the network-level payload of the packet and
 * does not include any capture framing headers. For example, an Ethernet 
 * packet with an empty TCP packet will return sizeof(ethernet_header) + 
 * sizeof(ip_header) + sizeof(tcp_header), but not the capture format
 * (pcap/erf/etc) header.
 */ 
 
size_t trace_get_wire_length(const libtrace_packet_t *packet);

/** Get the length of the capture framing headers (in bytes).
 * @param packet  	The packet to determine the framing length for
 * @return The size of the capture format header encapsulating the packet.
 *
 * @note This length corresponds to the difference between the amount of
 * memory required to store a captured packet and the capture length reported
 * by trace_capture_length()
 */ 
 
size_t trace_get_framing_length(const libtrace_packet_t *packet);

/** Get the length of the original payload content of the packet (in bytes).
 * @param packet	The packet to determine the payload length for
 * @return The size of the packet payload (without headers). Returns 0 if
 * unable to calculate payload length correctly.
 *
 * This function reports the amount of data that followed the transport header
 * when the packet was originally captured, i.e. prior to any snapping. Best
 * described as the wire length minus the packet headers. 
 *
 * Currently only supports some protocols and will return 0 if an unsupported
 * protocol header is encountered, or if one of the headers is truncated. 
 *
 * @note Supports IPv4, IPv6, TCP, UDP and ICMP.
 */ 
 
size_t trace_get_payload_length(const libtrace_packet_t *packet);

/** Truncate ("snap") the packet to the suggested length
 * @param packet	The packet to truncate
 * @param size		The new length of the packet (in bytes)
 *
 * @return The new capture length of the packet or the original capture
 * length of the packet if unchanged.
 *
 * This function will modify the capture length of the given packet. The wire
 * length will not be changed, so you can always determine what the original
 * packet size was, prior to the truncation.
 *
 * @note You can only use this function to decrease the capture length. Any
 * attempt to increase capture length will have no effect.
 */
 size_t trace_set_capture_length(libtrace_packet_t *packet, size_t size);

/*@}*/


/** Gets the link layer type for a packet
 * @param packet  	The packet to extract the link layer type for
 * @return A libtrace_linktype_t describing the link layer protocol being used
 * by this packet.
 */
 
libtrace_linktype_t trace_get_link_type(const libtrace_packet_t *packet);

/** Set the direction flag for a packet, if the capture format supports
 * direction tagging.
 *
 * @param packet  	The packet to set the direction for
 * @param direction	The new direction 
 * @returns -1 on error, or the direction that was set.
 *
 * @note Few capture formats actually support direction tagging. Most notably,
 * we cannot set the direction on PCAP packets.
 */
 libtrace_direction_t trace_set_direction(libtrace_packet_t *packet, libtrace_direction_t direction);

/** Get the direction flag for a packet, if it has one.
 * @param packet  The packet to get the direction for
 *
 * @return A value representing the direction flag, or -1 if this is not 
 * supported by the capture format.
 * 
 * The direction is defined as 0 for packets originating locally (ie, outbound)
 * and 1 for packets originating remotely (ie, inbound). Other values are 
 * possible, which might be overloaded to mean special things for certain 
 * traces, e.g. in the Waikato traces, 2 is used to represent an "Unknown"
 * direction.
 *
 * For DAG/ERF traces, the direction is extracted from the "Interface" bits in 
 * the ERF header, which can range from 0 - 3.
 */
 
libtrace_direction_t trace_get_direction(const libtrace_packet_t *packet);

/** @name BPF
 * This section deals with using Berkley Packet Filters to filter input traces
 * @{
 */
/** Creates a BPF filter
 * @param filterstring The filter string describing the BPF filter to create
 * @return An opaque pointer to a libtrace_filter_t object
 *
 * @note The filter is not actually compiled at this point, so no correctness
 * tests are performed here. trace_create_filter() will always return ok, but
 * if the filter is poorly constructed an error will be generated when the 
 * filter is actually used.
 */
 
libtrace_filter_t *trace_create_filter(const char *filterstring);

/** Create a BPF filter based on pre-compiled byte-code.
 * @param bf_insns	A pointer to the start of the byte-code
 * @param bf_len	The number of BPF instructions	
 * @return		An opaque pointer to a libtrace_filter_t object
 * @note		The supplied byte-code is not checked for correctness.
 * 			Instead, incorrect byte-code will generate an error
 * 			once the filter is actually used.
 * @author		Scott Raynel
 */
 libtrace_filter_t *
trace_create_filter_from_bytecode(void *bf_insns, unsigned int bf_len);

/** Apply a BPF filter to a packet
 * @param filter 	The filter to be applied	
 * @param packet	The packet to be matched against the filter
 * @return >0 if the filter matches, 0 if it doesn't, -1 on error.
 * 
 * @note Due to the way BPF filters are built, the filter is not actually
 * compiled until the first time trace_create_filter is called. If your filter
 * is incorrect, it will generate an error message and assert, exiting the
 * program. This behaviour may change to a more graceful handling of this error
 * in the future.
 */
 int trace_apply_filter(libtrace_filter_t *filter,
		const libtrace_packet_t *packet);

/** Destroy a BPF filter
 * @param filter 	The filter to be destroyed
 * 
 * Deallocates all the resources associated with a BPF filter.
 */
 void trace_destroy_filter(libtrace_filter_t *filter);
/*@}*/

/** @name Portability
 * This section contains functions that deal with portability issues, e.g. byte
 * ordering.
 * @{
 */

/** Converts an ethernet address to a printable string 
 * @param addr 	Ethernet address in network byte order
 * @param buf	Buffer to store the ascii representation, or NULL to indicate
 * that static storage should be used.
 * @return buf, or if buf is NULL then a statically allocated buffer.
 *
 * This function is similar to the GNU ether_ntoa_r function, with a few
 * minor differences.  If NULL is passed as buf, then the function will
 * use an internal static buffer. If NULL isn't passed then the function
 * will use that buffer instead.
 *
 * The address pointers returned by trace_get_source_mac() and 
 * trace_get_destination_mac() can be passed directly into this function.
 *
 * @note The type of addr isn't struct ether_addr as it is with ether_ntoa_r,
 * however it is bit compatible so that a cast will work.
 */ 
 char *trace_ether_ntoa(const uint8_t *addr, char *buf);

/** Convert a string to an ethernet address
 * @param buf	A string containing an Ethernet address in hex format 
 * delimited with :'s.
 * @param addr	Buffer to store the binary representation, or NULL to indicate
 * that static storage should be used.
 * @return addr, or if addr is NULL then a statically allocated buffer.
 *
 * This function is similar to the GNU ether_aton_r function, with a few
 * minor differences.  If NULL is passed as addr, then the function will
 * use an internal static buffer. If NULL isn't passed then the function will 
 * use that buffer instead.
 * 
 * The address returned by this function will be in network byte order.
 *
 * @note the type of addr isn't struct ether_addr as it is with ether_aton_r,
 * however it is bit compatible so that a cast will work.
 */
 uint8_t *trace_ether_aton(const char *buf, uint8_t *addr);

/*@}*/

/** @name Ports
 * This section contains functions for dealing with port numbers at the 
 * transport layer.
 *
 * @{
 */

/** An indication of which port is the "server" port for a given port pair */
typedef enum {
	USE_DEST, 	/**< Destination port is the server port */
	USE_SOURCE	/**< Source port is the server port */
} serverport_t;

/** Gets the source port for a given packet
 * @param packet	The packet to get the source port from
 * @return The source port in HOST byte order or 0 if no suitable port number
 * can be extracted from the packet.
 *
 * This function will return 0 if the transport protocol is known not to
 * use port numbers, e.g. ICMP. 0 is also returned if no transport header is
 * present in the packet or the transport header has been truncated such that
 * the port fields are not readable.
 *
 * @note If the transport protocol is not known by libtrace, the first two
 * bytes of the transport header will be treated as the source port field.
 */
 
uint16_t trace_get_source_port(const libtrace_packet_t *packet);

/** Gets the destination port for a given packet
 * @param packet	The packet to get the destination port from
 * @return The destination port in HOST byte order or 0 if no suitable port
 * number can be extracted from the packet.
 *
 * This function will return 0 if the transport protocol is known not to
 * use port numbers, e.g. ICMP. 0 is also returned if no transport header is
 * present in the packet or the transport header has been truncated such that
 * the port fields are not readable.
 *
 * @note If the transport protocol is not known by libtrace, the third and
 * fourth bytes of the transport header will be treated as the destination 
 * port field.
 *
 */
 
uint16_t trace_get_destination_port(const libtrace_packet_t *packet);

/** Hint at which of the two provided ports is the server port.
 *
 * @param protocol	The IP layer protocol, eg 6 (tcp), 17 (udp)
 * @param source	The source port from the packet
 * @param dest		The destination port from the packet
 *
 * @return one of USE_SOURCE or USE_DEST describing on which of the two ports
 * is most likely to be the server port.
 *
 * @note Ports must be provided in HOST byte order!
 *
 * This function is based almost entirely on heuristics and should not be
 * treated as a definitive means of identifying the server port. However, it
 * is deterministic, so it is very handy for identifying both halves of the
 * same flow. 
 */
 
int8_t trace_get_server_port(uint8_t protocol, uint16_t source, uint16_t dest);

/*@}*/

/** @name Wireless trace support
 * Functions to access wireless information from packets that have wireless
 * monitoring headers such as Radiotap or Prism.
 * 
 * The trace_get_wireless_* functions provide an abstract interface for
 * retrieving information from wireless traces. They take a pointer to the
 * wireless monitoring header (usually found with trace_get_packet_meta()) and
 * the linktype of the header passed in.
 * 
 * All of the trace_get_wireless_* functions return false if the requested
 * information was unavailable, or true if it was. The actual data is stored
 * in an output variable supplied by the caller. Values returned into the 
 * output variable will always be returned in host byte order.
 * @{
 */


/** Get the wireless Timer Synchronisation Function
 *
 * Gets the value of the timer synchronisation function for this frame, which
 * is a value in microseconds indicating the time that the first bit of the
 * MPDU was received by the MAC.
 *
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in 
 * @param[out] tsft The value of the timer synchronisation function. 
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_tsft(void *linkptr,
        libtrace_linktype_t linktype, uint64_t *tsft);

/** Get the wireless data rate 
 *
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] rate The data-rate of the frame in units of 500kbps
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_rate(void *linkptr,
        libtrace_linktype_t linktype, uint8_t *rate);

/** Get the wireless channel frequency
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] freq The frequency in MHz of the channel the frame was 
 * transmitted or received on.
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_freq(void *linkptr,
        libtrace_linktype_t linktype, uint16_t *freq);

/** Get the wireless signal strength in dBm 
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] strength The RF signal power at the antenna, in dB difference
 * from 1mW.
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_signal_strength_dbm(void *linkptr,
        libtrace_linktype_t linktype, int8_t *strength);

/** Get the wireless noise strength in dBm
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] strength The RF noise power at the antenna, in dB difference 
 * from 1mW. 
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_noise_strength_dbm(void *linkptr,
        libtrace_linktype_t linktype, int8_t *strength);

/** Get the wireless signal strength in dB
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] strength The RF signal power at the antenna, in dB difference 
 * from a fixed reference. 
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_signal_strength_db(void *linkptr,
        libtrace_linktype_t linktype, uint8_t *strength);

/** Get the wireless noise strength in dB 
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] strength The RF noise power at the antenna, in dB difference 
 * from a fixed reference. 
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_noise_strength_db(void *linkptr,
        libtrace_linktype_t linktype, uint8_t *strength);

/** Get the wireless transmit attenuation 
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] attenuation The transmit power as a unitless distance from 
 * maximum power set at factory calibration. 0 indicates maximum transmission 
 * power.
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_tx_attenuation(void *linkptr,
        libtrace_linktype_t linktype, uint16_t *attenuation);

/** Get the wireless transmit attenuation in dB
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] attenuation The transmit power as dB difference from maximum 
 * power set at factory calibration. 0 indicates maximum power.
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_tx_attenuation_db(void *linkptr,
        libtrace_linktype_t linktype, uint16_t *attenuation);

/** Get the wireless transmit power in dBm 
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in 
 * @param[out] txpower The transmit power as dB from a 1mW reference. This is 
 * the absolute power level measured at the antenna port.  
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_tx_power_dbm(void *linkptr, 
		libtrace_linktype_t linktype, int8_t *txpower);

/** Get the wireless antenna 
 * @param linkptr The wireless meta header
 * @param linktype The linktype of the wireless meta header passed in
 * @param[out] antenna The antenna that was used to transmit or receive the 
 * frame.
 * @return true if the field was available, false if not.
 */
 bool trace_get_wireless_antenna(void *linkptr,
        libtrace_linktype_t linktype, uint8_t *antenna);

/*@}*/

