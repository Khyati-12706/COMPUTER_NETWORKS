#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LabTaskApp");

// --- 1. SERVER: RECEIVES ARGS, CALCULATES, SENDS RESULT ---
class CalcServerApp : public Application {
public:
    CalcServerApp () : m_socket (0), m_port (9000) {}
    
    void Setup (uint16_t port, std::string protocol) { 
        m_port = port; 
        m_protocol = protocol;
    }

private:
    virtual void StartApplication (void) override {
        TypeId tid = (m_protocol == "TCP") ? TcpSocketFactory::GetTypeId () : UdpSocketFactory::GetTypeId ();
        m_socket = Socket::CreateSocket (GetNode (), tid);
        m_socket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_port));

        if (m_protocol == "TCP") {
            m_socket->Listen ();
            m_socket->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&CalcServerApp::HandleAccept, this)
            );
            NS_LOG_UNCOND ("Server initialized with TCP on port " << m_port);
        } else {
            m_socket->SetRecvCallback (MakeCallback (&CalcServerApp::HandleRead, this));
            NS_LOG_UNCOND ("Server initialized with UDP on port " << m_port);
        }
    }

    void HandleAccept (Ptr<Socket> s, const Address& from) {
        s->SetRecvCallback (MakeCallback (&CalcServerApp::HandleRead, this));
    }

    void HandleRead (Ptr<Socket> socket) {
        Ptr<Packet> packet;
        Address from;
        
        while ((packet = socket->RecvFrom (from))) {
            std::vector<uint8_t> buf(packet->GetSize() + 1);
            packet->CopyData (buf.data(), packet->GetSize ());
            buf[packet->GetSize()] = '\0';
            std::string msg((char*)buf.data());

            // 1. GET ARGUMENTS FROM CLIENT
            int num1 = 0, num2 = 0, result = 0;
            if (sscanf(msg.c_str(), "%d,%d", &num1, &num2) == 2) {
                // 2. PERFORM CALCULATION ON SERVER
                result = num1 + num2; 
            }

            NS_LOG_UNCOND ("Server [Processing]: Received '" << msg << "' -> Calculated Sum: " << result);

            // 3. SEND RESULT BACK
            std::string replyMsg = "Calculated Result = " + std::to_string(result);
            Ptr<Packet> responsePkt = Create<Packet> ((uint8_t*)replyMsg.c_str (), replyMsg.length ());
            
            socket->SendTo (responsePkt, 0, from);
        }
    }

    Ptr<Socket> m_socket;
    uint16_t m_port;
    std::string m_protocol;
};

// --- 2. CLIENT: SENDS ARGS & DISPLAYS SERVER'S RESULT ---
class CalcClientApp : public Application {
public:
    CalcClientApp () : m_socket (0) {}
    
    void Setup (Address address, std::string protocol, std::string args, std::string clientName) {
        m_peer = address;
        m_protocol = protocol;
        m_args = args;
        m_name = clientName;
    }

private:
    virtual void StartApplication (void) override {
        TypeId tid = (m_protocol == "TCP") ? TcpSocketFactory::GetTypeId () : UdpSocketFactory::GetTypeId ();
        m_socket = Socket::CreateSocket (GetNode (), tid);
        m_socket->Connect (m_peer);
        m_socket->SetRecvCallback (MakeCallback (&CalcClientApp::HandleResponse, this));
        
        Simulator::Schedule (MilliSeconds(100), &CalcClientApp::SendArgs, this);
    }

    void SendArgs () {
        Ptr<Packet> p = Create<Packet> ((uint8_t*)m_args.c_str (), m_args.length ());
        m_socket->Send (p);
        NS_LOG_UNCOND ("[" << m_name << "] Sent arguments to Server: " << m_args);
    }

    void HandleResponse (Ptr<Socket> socket) {
        Ptr<Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom (from))) {
            std::vector<uint8_t> buf(packet->GetSize() + 1);
            packet->CopyData (buf.data(), packet->GetSize ());
            buf[packet->GetSize()] = '\0';
            std::string data((char*)buf.data());
            
            // 4. DISPLAY RESULTS IN CLIENT SIDE
            NS_LOG_UNCOND ("[" << m_name << "] Recv From Server -> " << data);
        }
    }

    Ptr<Socket> m_socket;
    Address m_peer;
    std::string m_protocol;
    std::string m_args;
    std::string m_name;
};

// --- 3. TOPOLOGY & NETWORK PERFORMANCE SETUP ---
int main (int argc, char *argv[]) {
    std::string protocol = "UDP"; // Default protocol

    // Allow changing protocol via command line
    CommandLine cmd;
    cmd.AddValue ("protocol", "Set to TCP or UDP", protocol);
    cmd.Parse (argc, argv);

    Time::SetResolution (Time::NS);

    NodeContainer nodes;
    nodes.Create (3); // Node 0: Server, Node 1: Client A, Node 2: Client B

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

    NetDeviceContainer d10 = p2p.Install (nodes.Get (1), nodes.Get (0));
    NetDeviceContainer d20 = p2p.Install (nodes.Get (2), nodes.Get (0));

    InternetStackHelper stack;
    stack.Install (nodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i10 = address.Assign (d10);
    
    address.SetBase ("10.2.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i20 = address.Assign (d20);

    // Install Server
    Ptr<CalcServerApp> server = CreateObject<CalcServerApp> ();
    server->Setup (9000, protocol);
    nodes.Get (0)->AddApplication (server);
    server->SetStartTime (Seconds (1.0));
    server->SetStopTime (Seconds (10.0));

    // Client A (Sends arguments "15,25")
    Ptr<CalcClientApp> clientA = CreateObject<CalcClientApp> ();
    clientA->Setup (InetSocketAddress (i10.GetAddress (1), 9000), protocol, "15,25", "CLIENT_A");
    nodes.Get (1)->AddApplication (clientA);
    clientA->SetStartTime (Seconds (2.0));
    clientA->SetStopTime (Seconds (10.0));

    // Client B (Sends arguments "100,200")
    Ptr<CalcClientApp> clientB = CreateObject<CalcClientApp> ();
    clientB->Setup (InetSocketAddress (i20.GetAddress (1), 9000), protocol, "100,200", "CLIENT_B");
    nodes.Get (2)->AddApplication (clientB);
    clientB->SetStartTime (Seconds (3.0));
    clientB->SetStopTime (Seconds (10.0));

    // Setup Network Performance Measurements
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll();

    NS_LOG_UNCOND ("\n--- Starting Simulation (" << protocol << ") ---");
    Simulator::Stop (Seconds (11.0));
    Simulator::Run ();
    NS_LOG_UNCOND ("--- Simulation Finished ---\n");

    // Perform Network Performance Measurements
    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    NS_LOG_UNCOND ("--- Network Performance Measurements ---");
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
        std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
        std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
        std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
        std::cout << "  Lost Packets: " << i->second.lostPackets << "\n";
        std::cout << "  Delay Sum: " << i->second.delaySum.GetSeconds() << " s\n";
    }

    Simulator::Destroy ();
    return 0;
}
