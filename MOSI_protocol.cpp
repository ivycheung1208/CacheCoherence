#include "MOSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
	MOSI_protocol::MOSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
: Protocol (my_table, my_entry)
{
	this->state = MOSI_CACHE_I;
}

MOSI_protocol::~MOSI_protocol ()
{    
}

void MOSI_protocol::dump (void)
{
	const char *block_states[8] = {"X","I","S","M","O","IS","IM","OM"};
	fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

void MOSI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
		case MOSI_CACHE_I:  do_cache_I(request); break;
		case MOSI_CACHE_S:  do_cache_S(request); break;
		case MOSI_CACHE_M:  do_cache_M(request); break;
		case MOSI_CACHE_O:  do_cache_O(request); break;
		case MOSI_CACHE_IS:
		case MOSI_CACHE_IM:
		case MOSI_CACHE_OM: do_cache_X(request); break;
		default:
				    fatal_error ("Invalid Cache State for MOSI Protocol\n");
	}
}

void MOSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
		case MOSI_CACHE_I:  do_snoop_I(request);  break;
		case MOSI_CACHE_S:  do_snoop_S(request);  break;
		case MOSI_CACHE_M:  do_snoop_M(request);  break;
		case MOSI_CACHE_O:  do_snoop_O(request);  break;
		case MOSI_CACHE_IS: do_snoop_IS(request); break;
		case MOSI_CACHE_IM: do_snoop_IM(request); break;
		case MOSI_CACHE_OM: do_snoop_OM(request); break;
		default:
				    fatal_error ("Invalid Cache State for MOSI Protocol\n");
	}
}

inline void MOSI_protocol::do_cache_I (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
			send_GETS(request->addr);
			state = MOSI_CACHE_IS;
			Sim->cache_misses++;
			break;
		case STORE:
			send_GETM(request->addr);
			state = MOSI_CACHE_IM;
			Sim->cache_misses++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_S (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
			send_DATA_to_proc(request->addr);
			break;
		case STORE:
			send_GETM(request->addr);
			state = MOSI_CACHE_IM;
			Sim->cache_misses++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: S state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_M (Mreq *request)
{

	switch (request->msg) {
		case LOAD:
		case STORE:
			send_DATA_to_proc(request->addr);
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: M state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_O (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
			send_DATA_to_proc(request->addr);
			break;
		case STORE:
			send_GETM(request->addr);
			state = MOSI_CACHE_OM;
			Sim->cache_misses++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: O state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_cache_X (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
		case STORE:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Should only have one outstanding request per processor!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: X state shouldn't see this message\n");
	}
}

////////////////////////////////////////////////////////////

inline void MOSI_protocol::do_snoop_I (Mreq *request)
{
	switch (request->msg) {
		case GETS:
		case GETM:
		case DATA:
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: I state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_S (Mreq *request)
{
	switch (request->msg) {
		case GETS:
			break;
		case GETM:
			state = MOSI_CACHE_I;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: S state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_M (Mreq *request)
{
	switch (request->msg) {
		case GETS: // supply data and upgrade to O state
			// set_shared_line();
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MOSI_CACHE_O;
			break;
		case GETM:
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MOSI_CACHE_I;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: M state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_O (Mreq *request)
{
	switch (request->msg) {
		case GETS: // supply data
			send_DATA_on_bus(request->addr, request->src_mid);
			break;
		case GETM: // send data on bus then invalidate
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MOSI_CACHE_I;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: O state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_IS (Mreq *request)
{
	switch (request->msg) {
		case GETS:
		case GETM:
			break; // wait for DATA
		case DATA:
			send_DATA_to_proc(request->addr);
			state = MOSI_CACHE_S;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IS state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_IM (Mreq *request)
{
	switch (request->msg) {
		case GETS:
		case GETM:
			break; // wait for DATA
		case DATA:
			send_DATA_to_proc(request->addr);
			state = MOSI_CACHE_M;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void MOSI_protocol::do_snoop_OM (Mreq *request)
{
	switch (request->msg) {
		case GETS: // supply data
			send_DATA_on_bus(request->addr, request->src_mid);
			break;
		case GETM: // send data on bus then switch to IM state and wait for data
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MOSI_CACHE_IM;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: OM state shouldn't see this message\n");
	}
}

