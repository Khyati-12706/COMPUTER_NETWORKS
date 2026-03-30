#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main ()
{
    uint32_t n = 15;   // 15–25 allowed

    // Create nodes
    NodeContainer nodes;
    nodes.Create(n);

    // Install Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Point-to-point helper
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // IP addressing
    Ipv4AddressHelper address;
    address.SetBase("10.4.1.0", "255.255.255.0");

    // ---------------- TREE CONNECTIONS ----------------
    // Parent at index i → children at 2i+1, 2i+2
    for (uint32_t i = 0; i < n; i++)
    {
        uint32_t left = 2 * i + 1;
        uint32_t right = 2 * i + 2;

        if (left < n)
        {
            NodeContainer pair(nodes.Get(i), nodes.Get(left));
            NetDeviceContainer dev = p2p.Install(pair);
            address.Assign(dev);
            address.NewNetwork();
        }

        if (right < n)
        {
            NodeContainer pair(nodes.Get(i), nodes.Get(right));
            NetDeviceContainer dev = p2p.Install(pair);
            address.Assign(dev);
            address.NewNetwork();
        }
    }

    // ---------------- MOBILITY ----------------
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    // Manual tree layout (levels)
    positionAlloc->Add(Vector(300, 50, 0));   // Root (0)

    // Level 1
    positionAlloc->Add(Vector(150, 150, 0));
    positionAlloc->Add(Vector(450, 150, 0));

    // Level 2
    positionAlloc->Add(Vector(75, 250, 0));
    positionAlloc->Add(Vector(225, 250, 0));
    positionAlloc->Add(Vector(375, 250, 0));
    positionAlloc->Add(Vector(525, 250, 0));

    // Level 3 (remaining nodes)
    double x = 40;
    for (uint32_t i = 7; i < n; i++)
    {
        positionAlloc->Add(Vector(x, 350, 0));
        x += 80;
    }

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    // Small downward movement
    for (uint32_t i = 0; i < n; i++)
    {
        Ptr<ConstantVelocityMobilityModel> model =
            nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        model->SetVelocity(Vector(0.0, 1.0, 0.0));
    }

    // ---------------- NETANIM ----------------
    AnimationInterface anim("tree_mobility.xml");

    // Highlight nodes
    for (uint32_t i = 0; i < n; i++)
    {
        anim.UpdateNodeColor(nodes.Get(i)->GetId(), 0, 150, 0); // GREEN
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 15.0, 15.0);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
