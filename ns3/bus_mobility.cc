#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main ()
{
    uint32_t n = 15;   // 15–25 nodes allowed

    // Create nodes
    NodeContainer nodes;
    nodes.Create(n);

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // CSMA helper (Bus topology)
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

    NetDeviceContainer devices = csma.Install(nodes);

    // IP addressing
    Ipv4AddressHelper address;
    address.SetBase("10.2.1.0", "255.255.255.0");
    address.Assign(devices);

    // ---------------- MOBILITY ----------------
    MobilityHelper mobility;

    // Place nodes in a straight line (bus layout)
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint32_t i = 0; i < n; i++)
    {
        positionAlloc->Add(Vector(50 + i * 40, 200, 0));
    }

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    // Same velocity for all nodes
    for (uint32_t i = 0; i < n; i++)
    {
        Ptr<ConstantVelocityMobilityModel> model =
            nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        model->SetVelocity(Vector(5.0, 0.0, 0.0));
    }

    // ---------------- NETANIM ----------------
    AnimationInterface anim("bus_mobility.xml");

    // Highlight nodes (RED + bigger size)
    for (uint32_t i = 0; i < n; i++)
    {
        anim.UpdateNodeColor(nodes.Get(i)->GetId(), 255, 0, 0);
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 15.0, 15.0);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

