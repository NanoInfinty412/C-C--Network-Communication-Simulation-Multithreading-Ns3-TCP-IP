/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include <vector>
#include <thread>
#include <chrono>

// Default Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;
// Defines a new log component, which can be enable or disabled using ns3::LogComponentEnable and ns3::LogComponentDisable

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");
int main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3; //number of servers
  uint32_t nWifi = 3; //number of clientz
  bool tracing = false;
 

// Generate doxygen documentation of the arguments and options...This helps setting variables' values from the cmd line.
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {// Take logs - Log informational messages about program progress. Also applies to lower levels
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
//Take n number of computer
  NodeContainer p2pNodes; //Define a Node object

  p2pNodes.Create (2); // Class member "create" creates 2 nodes - One for the Point to point node and the other for n1(First lan device)
 
//choose your technology to communicate
  PointToPointHelper pointToPoint; //Define a point to point communication type (object)

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps")); //Set Data rate as 5 mbps transmitter
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms")); //Set delay
// Install communication technology on computers
  NetDeviceContainer p2pDevices; //Creates a recipient that accepts the communication point to point we just built
  p2pDevices = pointToPoint.Install (p2pNodes); // Install the technology on the computer

  NodeContainer csmaNodes; //Nodes for LAN connection
  csmaNodes.Add (p2pNodes.Get (1)); // Attach p2p device with CSMA(LAN) devices
  csmaNodes.Create (nCsma); //Add extra 3 nodes for LAN devices connected to node 0

  CsmaHelper csma; // Just like with PointToPointHelper, we create a NetDeviceContainer to hold the devices with CSmaHelper - Creating a Lan connection
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps")); //SET speed of LAN connectivity
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560))); //Set Delay

  NetDeviceContainer csmaDevices; // Create container for communication applications. (Network card?)
  csmaDevices = csma.Install (csmaNodes); //Install network bus on computers/nodes

  NodeContainer wifiStaNodes; // Create recipient for wifi connectivity
  wifiStaNodes.Create (nWifi); // Creates n computers/nodes for wifi (3 in this case)
  NodeContainer wifiApNode = p2pNodes.Get (0); //Make node 0 (point to point's linked list) the access point.

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default (); // Creates default wifi communication channel 
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ()); //Set channel to phy

  WifiHelper wifi; //Using mac parameters on 802.11 service
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid"); //Set name of ssid/ access point
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false)); //Probe requests won't be sent by MACs created by this helper

  NetDeviceContainer staDevices; // Create a container for wifi connectivity. Network card to be inserted in nodes 
  staDevices = wifi.Install (phy, mac, wifiStaNodes); //Install wifi card in computers

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));       //Set mac configurations

  NetDeviceContainer apDevices; //Create access point devices
  apDevices = wifi.Install (phy, mac, wifiApNode); //Install them in computers

  MobilityHelper mobility; //Make STA nodes mobile and AP stationary. 
 
//Set position
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes); //Install location and mobility

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);  //Install location and mobility

// Install protocol stacks on each set of nodes - Asking to follow rules

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

// Assign IP address to communicate

  Ipv4AddressHelper address; //Use ipv4
       
// Use the following address and port for p2p devices and all Csma devices
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices); // Apply to p2p devices

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices); // Apply to csma devices

// Use the following address and port for both access point and STA

  
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices); //Apply settings  to STA
  address.Assign (apDevices);    //Apply settings  to AP



// CODE MODIFIED 

std::vector<UdpEchoServerHelper> echoServer(nCsma,UdpEchoServerHelper(9));   //Create vectors and arrays  to store echo servers, echo clients,  and application containers
ApplicationContainer serverApps[nCsma];   
std::vector<UdpEchoClientHelper> echoClient(nWifi,UdpEchoClientHelper(csmaInterfaces.GetAddress (nCsma), 9));
ApplicationContainer clientApps[nWifi*nCsma];
for(uint32_t i=0; i<nWifi;i++){
          echoClient[i]= UdpEchoClientHelper(csmaInterfaces.GetAddress (nCsma-i), ((nCsma)-i)); //Generate both echo servers and echo clients with right ports (Set by constructor)
          if(i == nWifi-1){
                for(uint32_t j = 0; j<nCsma;j++){
                    echoServer[j]=UdpEchoServerHelper((nCsma)-j);
        
                }
        }
}
for(uint32_t  i=0;i<nCsma ;i++){
        serverApps[i] = echoServer[i].Install (csmaNodes.Get (nCsma-i));  
        serverApps[i].Start (Seconds (2*i+1)); //Start
        serverApps[i].Stop (Seconds (100.0)); //Stops after 100   
        if(i==nCsma-1){
                  for(uint32_t  j=0;j<nWifi ;j++){
                          echoClient[j].SetAttribute ("MaxPackets", UintegerValue (1));  //Set attributes of each entry
                          echoClient[j].SetAttribute ("Interval", TimeValue (Seconds (1.0)));
                          echoClient[j].SetAttribute ("PacketSize", UintegerValue (12));
                                
                }
                for(uint32_t k=0, m = 1; k<(nWifi*nCsma);k++,m++){
                       if(m==nWifi+1){
                                m = 1;
                }
                        clientApps[k] = echoClient[m-1].Install (wifiStaNodes.Get (nWifi - m)); // cient is the last leftmost node on the STA design
                        clientApps[k].Start (Seconds (2*k+1));
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                        clientApps[k].Stop (Seconds (100)); //Stops after 99sec      
                }
        }
}
Ipv4GlobalRoutingHelper::PopulateRoutingTables (); //Enable internet network

  Simulator::Stop (Seconds (100.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }
// Run simulation
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

