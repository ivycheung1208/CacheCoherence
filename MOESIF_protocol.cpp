#include "MOESIF_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESIF_protocol::MOESIF_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
this->state = MOESIF_CACHE_I;
}

MOESIF_protocol::~MOESIF_protocol ()
{    
}

void MOESIF_protocol::dump (void)
{
    const char *block_states[11] = {"X","I","IS","S","E","F","IM","SM","M","O","OM"};
    fprintf (stderr, "MOESIF_protocol - state: %s\n", block_states[state]);
}

void MOESIF_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
                case MOESIF_CACHE_I:  do_cache_I(request);  break;
                case MOESIF_CACHE_IS: do_cache_IS(request); break;
                case MOESIF_CACHE_S:  do_cache_S(request);  break;
                case MOESIF_CACHE_E:  do_cache_E(request);  break;
                case MOESIF_CACHE_F:  do_cache_F(request);  break;
                case MOESIF_CACHE_IM: do_cache_IM(request); break;
                case MOESIF_CACHE_SM: do_cache_SM(request); break;
                case MOESIF_CACHE_M:  do_cache_M(request);  break;
                case MOESIF_CACHE_O:  do_cache_O(request);  break;
                case MOESIF_CACHE_OM: do_cache_OM(request); break;
    default:
        fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

void MOESIF_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
                case MOESIF_CACHE_I:  do_snoop_I(request);  break;
                case MOESIF_CACHE_IS: do_snoop_IS(request); break;
                case MOESIF_CACHE_S:  do_snoop_S(request);  break;
                case MOESIF_CACHE_E:  do_snoop_E(request);  break;
                case MOESIF_CACHE_F:  do_snoop_F(request);  break;
                case MOESIF_CACHE_IM: do_snoop_IM(request); break;
                case MOESIF_CACHE_SM: do_snoop_SM(request); break;
                case MOESIF_CACHE_M:  do_snoop_M(request);  break;
                case MOESIF_CACHE_O:  do_snoop_O(request);  break;
                case MOESIF_CACHE_OM: do_snoop_OM(request); break;
    default:
    	fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

inline void MOESIF_protocol::do_cache_I (Mreq *request)
{
        switch (request->msg) {
                case LOAD: // upgrade to E or S state
                        send_GETS(request->addr);
                        state = MOESIF_CACHE_IS;
                        Sim->cache_misses++;
                        break;
                case STORE: // upgrade to M state
                        send_GETM(request->addr);
                        state = MOESIF_CACHE_IM;
                        Sim->cache_misses++;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: I state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_S (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                        send_DATA_to_proc(request->addr);
                        break;
                case STORE:
                        send_GETM(request->addr);
                        state = MOESIF_CACHE_SM;
                        Sim->cache_misses++;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: S state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_E (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                        send_DATA_to_proc(request->addr);
                        break;
                case STORE: // silent upgrade
                        send_DATA_to_proc(request->addr);
                        state = MOESIF_CACHE_M;
                        Sim->silent_upgrades++;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: E state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_F (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                        send_DATA_to_proc(request->addr);
                        break;
                case STORE:
                        send_GETM(request->addr);
                        state = MOESIF_CACHE_OM;
                        Sim->cache_misses++;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: O state shouldn't see this message\n");
        }
}


inline void MOESIF_protocol::do_cache_M (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                case STORE:
                        send_DATA_to_proc(request->addr);
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: M state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_O (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                        send_DATA_to_proc(request->addr);
                        break;
                case STORE:
                        send_GETM(request->addr);
                        state = MOESIF_CACHE_OM;
                        Sim->cache_misses++;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: O state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_IS (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                case STORE:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Should only have one outstanding request per processor!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: IS state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_IM (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                case STORE:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Should only have one outstanding request per processor!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: IM state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_SM (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                case STORE:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Should only have one outstanding request per processor!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: SM state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_cache_OM (Mreq *request)
{
        switch (request->msg) {
                case LOAD:
                case STORE:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Should only have one outstanding request per processor!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERRIR");
                        fatal_error ("Client: OM state shouldn't see this message\n");
        }
}

////////////////////////////////////////////////////////////

inline void MOESIF_protocol::do_snoop_I (Mreq *request)
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

inline void MOESIF_protocol::do_snoop_S (Mreq *request)
{
        switch (request->msg) {
                case GETS: // set shared line
                        set_shared_line();
                        break;
                case GETM:
                        state = MOESIF_CACHE_I;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: S state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_E (Mreq *request)
{
        switch (request->msg) {
                case GETS: // upgrade to F state, send data on bus, set shared line
                        set_shared_line();
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_F;
                        break;
                case GETM: // send data on bus and invalidate
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_I;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: E state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_F (Mreq *request)
{
        switch (request->msg) {
                case GETS: // stay in F state, send data on bus, set shared line
                        set_shared_line();
                        send_DATA_on_bus(request->addr, request->src_mid);
                        break;
                case GETM: // send data on bus and invalidate
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_I;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: F state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_M (Mreq *request)
{
        switch (request->msg) {
                case GETS: // upgrade to O state, send data on bus, set shared line
                        set_shared_line();
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_O;
                        break;
                case GETM: // send data on bus and invalidate
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_I;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: M state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_O (Mreq *request)
{
        switch (request->msg) {
                case GETS: // set shared line, send data on bus, stay in O state
                        set_shared_line();
                        send_DATA_on_bus(request->addr, request->src_mid);
                        break;
                case GETM: // send data on bus and invalidate
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_I;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: O state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_IS (Mreq *request)
{
        switch (request->msg) {
                case GETS:
                case GETM:
                        break; // wait for data
                case DATA:
                        send_DATA_to_proc(request->addr);
                        if (get_shared_line())
                                state = MOESIF_CACHE_S;
                        else
                                state = MOESIF_CACHE_E;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: IS state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_IM (Mreq *request)
{
        switch (request->msg) {
                case GETS:
                case GETM:
                        break; // wait for data
                case DATA:
                        send_DATA_to_proc(request->addr);
                        state = MOESIF_CACHE_M;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: IM state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_SM (Mreq *request)
{
        switch (request->msg) {
                case GETS: // set shared line
                        set_shared_line();
                        break;
                case GETM: // invalidate if GetM came from another proc
                        if (request->src_mid != my_table->moduleID)
                                state = MOESIF_CACHE_IM;
                        break;
                case DATA:
                        send_DATA_to_proc(request->addr);
                        state = MOESIF_CACHE_M;
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: SM state shouldn't see this message\n");
        }
}

inline void MOESIF_protocol::do_snoop_OM (Mreq *request)
{
        switch (request->msg) {
                case GETS: // set shared line and supply data
                        set_shared_line();
                        send_DATA_on_bus(request->addr, request->src_mid);
                        break;
                case GETM: // send data on bus and switch to IM state and wait for data
                        send_DATA_on_bus(request->addr, request->src_mid);
                        state = MOESIF_CACHE_IM;
                        break;
                case DATA:
                        fatal_error ("Should not see data for this line! I have the line!");
                        break;
                default:
                        request->print_msg (my_table->moduleID, "ERROR");
                        fatal_error ("Client: OM state shouldn't see this message\n");
        }
}

