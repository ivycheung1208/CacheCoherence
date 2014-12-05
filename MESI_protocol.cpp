#include "MESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
	MESI_protocol::MESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
: Protocol (my_table, my_entry)
{
	this->state = MESI_CACHE_I;
}

MESI_protocol::~MESI_protocol ()
{    
}

void MESI_protocol::dump (void)
{
	const char *block_states[8] = {"X","I","IS","S","E","IM","SM","M"};
	fprintf (stderr, "MESI_protocol - state: %s\n", block_states[state]);
}

void MESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
		case MESI_CACHE_I:  do_cache_I(request);  break;
		case MESI_CACHE_IS: do_cache_IS(request); break;
		case MESI_CACHE_S:  do_cache_S(request);  break;
		case MESI_CACHE_E:  do_cache_E(request);  break;
		case MESI_CACHE_IM: do_cache_IM(request); break;
		case MESI_CACHE_SM: do_cache_SM(request); break;
		case MESI_CACHE_M:  do_cache_M(request);  break;
		default:
				    fatal_error ("Invalid Cache State for MESI Protocol\n");
	}
}

void MESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
		case MESI_CACHE_I:  do_snoop_I(request);  break;
		case MESI_CACHE_IS: do_snoop_IS(request); break;
		case MESI_CACHE_S:  do_snoop_S(request);  break;
		case MESI_CACHE_E:  do_snoop_E(request);  break;
		case MESI_CACHE_IM: do_snoop_IM(request); break;
		case MESI_CACHE_SM: do_snoop_SM(request); break;
		case MESI_CACHE_M:  do_snoop_M(request);  break;
		default:
				    fatal_error ("Invalid Cache State for MESI Protocol\n");
	}
}

inline void MESI_protocol::do_cache_I (Mreq *request)
{
	switch (request->msg) {
		case LOAD: // upgrade to E or S state
			send_GETS(request->addr);
			state = MESI_CACHE_IS;
			Sim->cache_misses++;
			break;
		case STORE: // upgrade to M state
			send_GETM(request->addr);
			state = MESI_CACHE_IM;
			Sim->cache_misses++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: I state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_IS (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
		case STORE:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Should only have one outstanding request per processor!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IS state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_S (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
			send_DATA_to_proc(request->addr);
			break;
		case STORE:
			send_GETM(request->addr);
			state = MESI_CACHE_SM;
			Sim->cache_misses++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: S state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_E (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
			send_DATA_to_proc(request->addr);
			break;
		case STORE: // silent upgrade
			send_DATA_to_proc(request->addr);
			state = MESI_CACHE_M;
			Sim->silent_upgrades++;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: E state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_IM (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
		case STORE:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Should only have one outstanding request per processor!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_SM (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
		case STORE:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Should only have one outstanding request per processor!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: SM state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_cache_M (Mreq *request)
{
	switch (request->msg) {
		case LOAD:
		case STORE:
			send_DATA_to_proc(request->addr);
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: M state shouldn't see this meesage\n");
	}
}

inline void MESI_protocol::do_snoop_I (Mreq *request)
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

inline void MESI_protocol::do_snoop_IS (Mreq *request)
{
	switch (request->msg) {
		case GETS:
		case GETM:
			break;
		case DATA:
			send_DATA_to_proc(request->addr);
			if (get_shared_line()) { // another cache has a copy of this line
				state = MESI_CACHE_S;
			}
			else // nobody else has any copy of this line
				state = MESI_CACHE_E;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IS state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_snoop_S (Mreq *request)
{
	switch (request->msg) {
		case GETS: // set shared line
			set_shared_line();
			break;
		case GETM:
			state = MESI_CACHE_I;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: S state shouldn't see this message \n");
	}
}

inline void MESI_protocol::do_snoop_E (Mreq *request)
{
	switch (request->msg) {
		case GETS: // downgrade to S state, send data on bus, set shared line
			set_shared_line();
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MESI_CACHE_S;
			break;
		case GETM:
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MESI_CACHE_I;
			break;
		case DATA:
			fatal_error ("Should not see data for this line! I have the line!");
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: E state shouldn't see this message \n");
	}
}

inline void MESI_protocol::do_snoop_IM (Mreq *request)
{
	switch (request->msg) {
		case GETS:
		case GETM:
			break; // wait for data
		case DATA:
			send_DATA_to_proc(request->addr);
			state = MESI_CACHE_M;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_snoop_SM (Mreq *request)
{
	switch (request->msg) {
		case GETS: // notify that a copy of this line exists
			set_shared_line();
			break;
		case GETM: // invalidate if GetM came from another proc
			if (request->src_mid != my_table->moduleID) 
				state = MESI_CACHE_IM;
			break;
		case DATA:
			send_DATA_to_proc(request->addr);
			state = MESI_CACHE_M;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: SM state shouldn't see this message\n");
	}
}

inline void MESI_protocol::do_snoop_M (Mreq *request)
{
	switch (request->msg) {
		case GETS: // write back and downgrade to S state, set shared line
			set_shared_line();
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MESI_CACHE_S;
			break;
		case GETM: // write back and invalidate
			send_DATA_on_bus(request->addr, request->src_mid);
			state = MESI_CACHE_I;
			break;
		default:
			request->print_msg (my_table->moduleID, "ERROR");
			fatal_error ("Client: M state shouldn't see this message\n");
	}
}

