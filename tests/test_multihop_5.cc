#include "broker/broker.hh"
#include "broker/endpoint.hh"
#include "broker/message_queue.hh"
#include "broker/report.hh"
#include "testsuite.h"
#include <vector>
#include <string>
#include <iostream>

// A test of "event" style messages -- vectors of data w/ first element
// being the event name.

using namespace broker;

int main(int argc, char** argv)
	{
	init();

	// init debugging/reporting 
	//broker::report::init();

	/* Overlay configuration
	 * n0 [a,b]
	 * |
	 * n1 [a,b]
	 * |  
	 * n2 [a,b]
	 */

	int flags = AUTO_PUBLISH | AUTO_ADVERTISE | AUTO_ROUTING;
	// Node 0
	endpoint node0("node0", flags);
	message_queue q0a("a", node0, MULTI_HOP);
	message_queue q0b("b", node0, MULTI_HOP);

	// Node 1
	endpoint node1("node1", flags);
	message_queue q1a("a", node1, MULTI_HOP);
	message_queue q1b("b", node1, MULTI_HOP);
	// connecting
	node1.peer(node0);

	if ( node1.outgoing_connection_status().need_pop().front().status !=
	     outgoing_connection_status::tag::established)
		{
		BROKER_TEST(false);
		return 1;
		}

	// Node 2
	endpoint node2("node2", flags);
	message_queue q2a("a", node2, MULTI_HOP);
	message_queue q2b("b", node2, MULTI_HOP);
	// connecting
	node2.peer(node1);

	if ( node2.outgoing_connection_status().need_pop().front().status !=
	     outgoing_connection_status::tag::established)
		{
		BROKER_TEST(false);
		return 1;
		}


	std::vector<message> msg_a;
	msg_a.push_back(message{"command", "yo"});
	// sending request
	node0.send("a", msg_a[0], 0x02);

	int counter = 0;
	for ( auto& msg : q1a.need_pop() )
		{
		counter++;
		//std::cout << "node1 received msg " << msg[0] << std::endl;
		}

	for ( auto& msg : q2a.need_pop() )
		{
		counter++;
		//std::cout << "node2 received msg " << msg[0] << std::endl;
		}

	//BROKER_TEST(msg_1a == msg_2a);

	//sending reply
	std::vector<message> msg_b;
	msg_b.push_back(message{"result", "yo"});
	node2.send("b", msg_b[0], 0x02);

	for ( auto& msg : q1b.need_pop() )
		{
		counter++;
		//std::cout << "node1 received msg " << msg[0] << std::endl;
		}

	for ( auto& msg : q0b.need_pop() )
		{
		counter++;
		//std::cout << "node0 received msg " << msg[0] << std::endl;
		}

	BROKER_TEST(counter == 4);

	return BROKER_TEST_RESULT();
	}
