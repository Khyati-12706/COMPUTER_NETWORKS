#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include <cmath>

using namespace ns3;

int main ()
{
    uint32_t n = 15;   // 15–25 allowed

    // Create nodes
    NodeContainer nodes;
    nodes.Create(n);

    // Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Point-to-point helper
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // IP addressing
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    // Ring connections
    for (uint32_t i = 0; i < n; i++)
    {
        NodeContainer pair(nodes.Get(i), nodes.Get((i + 1) % n));
        NetDeviceContainer devices = p2p.Install(pair);
        address.Assign(devices);
        address.NewNetwork();
    }

    // ---------------- MOBILITY ----------------
    MobilityHelper mobility;

    // Initial circular position allocation
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    double radius = 150.0;

    for (uint32_t i = 0; i < n; i++)
    {
        double angle = (2 * M_PI * i) / n;
        double x = radius * cos(angle) + 250;
        double y = radius * sin(angle) + 250;
        positionAlloc->Add(Vector(x, y, 0.0));
    }

    mobility.SetPositionAllocator(positionAlloc);

    // Mobility model
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    // Assign velocity to each node
    for (uint32_t i = 0; i < n; i++)
    {
        Ptr<ConstantVelocityMobilityModel> model =
            nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        model->SetVelocity(Vector(5.0, 0.0, 0.0));  // slow movement
    }

    // NetAnim
    AnimationInterface anim("ring_mobility.xml");

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}

