
#include <iostream>
#include <string>
#include <random>
#include <cmath>
#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
#include <vector>

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"

using namespace ns3;
using namespace std;

/**
 * \ingroup aodv-examples
 * \ingroup examples
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then ping last node from the first one:
 * 
 * [10.0.0.1] <-- step --> [10.0.0.2] <-- step --> [10.0.0.3] <-- step --> [10.0.0.4]
 * 
 * ping 10.0.0.4
 *
 * When 1/3 of simulation time has elapsed, one of the nodes is moved out of
 * range, thereby breaking the topology.  By default, this will result in
 * only 34 of 100 pings being received.  If the step size is reduced
 * to cover the gap, then all pings can be received.
 */
class AodvExample 
{
public:
  AodvExample ();
  /**
   * \brief Configure script parameters
   * \param argc is the command line argument count
   * \param argv is the command line arguments
   * \return true on successful configuration
  */
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  /**
   * Report results
   * \param os the output stream
   */
  void Report (std::ostream & os);

private:

  // parameters
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  /// Print routes if true
  bool printRoutes;

  // network
  /// nodes used in the example
  NodeContainer nodes;
  /// devices used in the example
  NetDeviceContainer devices;
  /// interfaces used in the example
  Ipv4InterfaceContainer interfaces;

private:
  /// Create the nodes
  void CreateNodes ();
  /// Create the devices
  void CreateDevices ();
  /// Create the network
  void InstallInternetStack ();
  /// Create the simulation applications
  void InstallApplications ();
};

int main (int argc, char **argv)
{
  AodvExample test;
  if (!test.Configure (argc, argv))
    NS_FATAL_ERROR ("Configuration failed. Aborted.");

  test.Run ();
  test.Report (std::cout);
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (21),
  step (4),
  totalTime (100),
  pcap (true),
  printRoutes (true)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd (__FILE__);

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);

  cmd.Parse (argc, argv);
  return true;
}

void
AodvExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void
AodvExample::Report (std::ostream &)
{ 
}

void
AodvExample::CreateNodes ()
{
float x[21] = {27.4,217.5,515.1,841.70,36.7,386.6,442.3,603.3,726.5,863.4,130.9,336.3,600.2,890.2,986.2,255.5,533.6,613.2,697.7,747.7,917.2};
float dy [4]={2,6,10,14};

  std::cout << "Creating " << (unsigned)size << " nodes in 4 lanes and " << step << " m apart.\n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
  // Create static grid
int j=0;
for(uint32_t i=0;i<size;i++){
if(i==4)
j=1;
else if(i==10)
j=2;
else if(i==15)
j=3;
  MobilityHelper mobility;
        if(i==0){
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (x[i]),
                                 "DeltaY", DoubleValue (dy[j]),
                                 "GridWidth", UintegerValue (size),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes.Get(i));
}
else{
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (x[i]-x[i-1]),
                                 "DeltaY", DoubleValue (dy[j]),
                                 "GridWidth", UintegerValue (size),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes.Get(i));

}
}
}

void
AodvExample::CreateDevices ()
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes); 



  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}void
AodvExample::InstallInternetStack ()
{
  AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
  //aodv.Set("size",size);
  if (printRoutes)
    {

      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);
    }

}

void
AodvExample::InstallApplications ()
{

  // UDP connfection from N0 to N2
 //std::string server[10] = {"10.0.0.1","10.0.0.2","10.0.0.3","10.0.0.4","10.0.0.5","10.0.0.6","10.0.0.7","10.0.0.8","10.0.0.9","10.0.0.10"};
  //std::string clients[11] =  {"10.0.0.11","10.0.0.12","10.0.0.13","10.0.0.14","10.0.0.15","10.0.0.16","10.0.0.17","10.0.0.18","10.0.0.19","10.0.0.20","10.0.0.21"};
  uint16_t sinkPort = 50000;
  for(uint32_t i=0;i<11;i++){
          Address sinkLocalAddress (InetSocketAddress (interfaces.GetAddress(i), sinkPort)); // interface of n #i

         PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
          std::vector<ApplicationContainer>sinkApps;
          sinkApps[i] = sinkHelper.Install (nodes.Get (5)); //n #i as sink
          sinkApps[i].Start (Seconds (0.));
          sinkApps[i].Stop (Seconds (100.));
}

// Create the OnOff Applications to use UDP
  for(uint32_t i=0; i<10;i++){
          OnOffHelper clientHelper ("ns3::UdpSocketFactory", InetSocketAddress (interfaces.GetAddress(i), sinkPort));
     // clientHelper.SetAttribute("Remote", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
          std::vector<ApplicationContainer>clientApps;
          clientApps[i] = clientHelper.Install (nodes.Get(i+2)); 
          clientApps[i].Start(Seconds(0.0));
          clientApps[i].Stop(Seconds(100));
 }
 for(uint32_t i =0;i<size;i++){      
          V4PingHelper ping (interfaces.GetAddress (size-i-1));
          ping.SetAttribute ("Verbose", BooleanValue (true));
          if(i>10){
                ApplicationContainer p2 = ping.Install (nodes.Get (i));
                p2.Start (Seconds (0));
                p2.Stop (Seconds (totalTime) - Seconds (0.001));
          }
        else{
          ApplicationContainer p1 = ping.Install (nodes.Get (i));
          p1.Start (Seconds (0));
          p1.Stop (Seconds (totalTime) - Seconds (0.001));
        }
   }


}




