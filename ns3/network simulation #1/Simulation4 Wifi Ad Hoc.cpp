#MAIN CODE 
#========



/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
 *
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include <iostream>
#include "experiment.h"
#include "ns3/gnuplot.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include <memory>
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wifi-Adhoc");


int main (int argc, char *argv[])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);
    
int size = 14;
int i =0;
        string input [size] = {"54", "48", "36", "24","18","12","9","6","arf","aarf","aarf-cd","cara", "rraa", "ideal"};
  Gnuplot gnuplot = Gnuplot ("reference-rates.png");

  Experiment experiment;
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211a);
  WifiMacHelper wifiMac;
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Gnuplot2dDataset dataset;
  wifiMac.SetType ("ns3::AdhocWifiMac");
  while (i<size){
        string temp;
  gnuplot.GenerateOutput (std::cout);
if (i==8){

  gnuplot.GenerateOutput (std::cout);
}
if (i<7){
std::cout<<"Process("<<i+1<< ") in execution ....|\n|\n"<<std::endl;
        NS_LOG_DEBUG (input[i]);
        temp = input[i];
        experiment = Experiment (temp.append("mb"));
        temp = "OfdmRate";
        temp = temp.append(input[i]);

        wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue (temp.append("Mbps")));
        dataset = experiment.Run (wifi, wifiPhy, wifiMac, wifiChannel);
        gnuplot.AddDataset (dataset);}
if (i>7){
        NS_LOG_DEBUG (input[i]);
  experiment = Experiment (input[i]);
temp = "ns3::s";
temp = temp.append(input[i]);
  wifi.SetRemoteStationManager (temp.append("WifiManager"));
  dataset = experiment.Run (wifi, wifiPhy, wifiMac, wifiChannel);
  gnuplot.AddDataset (dataset);

}
if(i==size-1){

 gnuplot.GenerateOutput (std::cout);
}

std::cout<<"Process("<<i+1<< ") done! ....\n\n"<<std::endl;
i++;

}




#EXPERIMENT.H
#==============


#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "ns3/gnuplot.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"

#include <vector>
#include <iostream>
#include "experiment.h"
#include "ns3/gnuplot.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/mobility-model.h"
#include <memory>

using namespace std;
class Experiment
{
public:
  Experiment ();
  Experiment (std::string name) ;
ns3::Gnuplot2dDataset Run (const ns3::WifiHelper &wifi, const ns3::YansWifiPhyHelper &wifiPhy,
                        const ns3::WifiMacHelper &wifiMac, const ns3::YansWifiChannelHelper &wifiChannel);

  
private:
  void ReceivePacket (ns3::Ptr<ns3::Socket> socket);
  void SetPosition (ns3::Ptr<ns3::Node> node, ns3::Vector position);
  ns3::Vector GetPosition (ns3::Ptr<ns3::Node> node);
  void AdvancePosition (ns3::Ptr<ns3::Node> node);
  ns3::Ptr<ns3::Socket> SetupPacketReceive (ns3::Ptr<ns3::Node> node);

  uint32_t m_bytesTotal;
  ns3::Gnuplot2dDataset m_output;


};

#endif


#EXPERIMENT.CC
#==============

#include <iostream>
#include "experiment.h"
#include "ns3/gnuplot.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include <memory>
#include "ns3/packet-socket-helper.h"
#include "ns3/packet-socket-address.h"


using namespace ns3;

using namespace std;
//NS_LOG_COMPONENT_DEFINE ("Wifi-Adhoc");



Experiment::Experiment ()
{
}

Experiment::Experiment (std::string name): m_output (name)
{
  m_output.SetStyle (Gnuplot2dDataset::LINES);
}

Gnuplot2dDataset Experiment::Run (const WifiHelper &wifi, const YansWifiPhyHelper &wifiPhy,
                 const WifiMacHelper &wifiMac, const YansWifiChannelHelper &wifiChannel)
{
  m_bytesTotal = 0;

  NodeContainer c;
  c.Create (2);

  PacketSocketHelper packetSocket;
  packetSocket.Install (c);

  YansWifiPhyHelper phy = wifiPhy;
  phy.SetChannel (wifiChannel.Create ());

  WifiMacHelper mac = wifiMac;
  NetDeviceContainer devices = wifi.Install (phy, mac, c);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (c);

  PacketSocketAddress socket;
  socket.SetSingleDevice (devices.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (devices.Get (1)->GetAddress ());
  socket.SetProtocol (1);

  OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
  onoff.SetConstantRate (DataRate (60000000));
  onoff.SetAttribute ("PacketSize", UintegerValue (2000));

  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (0.5));
  apps.Stop (Seconds (250.0));

  Simulator::Schedule (Seconds (1.5), &Experiment::AdvancePosition, this, c.Get (1));
  Ptr<Socket> recvSink = SetupPacketReceive (c.Get (1));

  Simulator::Run ();

  Simulator::Destroy ();

  return m_output;
}


void Experiment::ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  while ((packet = socket->Recv ()))
    {
      m_bytesTotal += packet->GetSize ();
    }
}


void Experiment::SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

Vector Experiment::GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  return mobility->GetPosition ();
}

void Experiment::AdvancePosition (Ptr<Node> node)
{
  Vector pos = GetPosition (node);
  double mbs = ((m_bytesTotal * 8.0) / 1000000);
  m_bytesTotal = 0;
  m_output.Add (pos.x, mbs);
  pos.x += 1.0;
  if (pos.x >= 210.0)
    {
      return;
    }
  SetPosition (node, pos);
  Simulator::Schedule (Seconds (1.0), &Experiment::AdvancePosition, this, node);
}


Ptr<Socket> Experiment::SetupPacketReceive (Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  sink->Bind ();
  sink->SetRecvCallback (MakeCallback (&Experiment::ReceivePacket, this));
  return sink;
}


