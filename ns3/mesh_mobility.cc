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

    // Internet stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // Point-to-point helper
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    // IP address helper
    Ipv4AddressHelper address;
    address.SetBase("10.3.1.0", "255.255.255.0");

    // ---------------- MESH CONNECTIONS ----------------
    for (uint32_t i = 0; i < n; i++)
    {
        for (uint32_t j = i + 1; j < n; j++)
        {
            NodeContainer pair(nodes.Get(i), nodes.Get(j));
            NetDeviceContainer devices = p2p.Install(pair);
            address.Assign(devices);
            address.NewNetwork();
        }
    }

    // ---------------- MOBILITY ----------------
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();

    // Place nodes in grid-like layout
    uint32_t gridSize = 5;
    double spacing = 80.0;
    uint32_t index = 0;

    for (uint32_t x = 0; x < gridSize && index < n; x++)
    {
        for (uint32_t y = 0; y < gridSize && index < n; y++)
        {
            positionAlloc->Add(Vector(100 + x * spacing, 100 + y * spacing, 0));
            index++;
        }
    }

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes);

    for (uint32_t i = 0; i < n; i++)
    {
        Ptr<ConstantVelocityMobilityModel> model =
            nodes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
        model->SetVelocity(Vector(2.0, 2.0, 0.0));
    }

    // ---------------- NETANIM ----------------
    AnimationInterface anim("mesh_mobility.xml");

    // Highlight nodes
    for (uint32_t i = 0; i < n; i++)
    {
        anim.UpdateNodeColor(nodes.Get(i)->GetId(), 0, 0, 255); // BLUE
        anim.UpdateNodeSize(nodes.Get(i)->GetId(), 15.0, 15.0);
    }

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
