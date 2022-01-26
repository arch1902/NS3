#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Part3");



class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
  *stream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

int num_dropped = 0;

static void
RxDrop (Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
  NS_LOG_UNCOND ("RxDrop at " << Simulator::Now ().GetSeconds ());
  file->Write (Simulator::Now (), p);
  num_dropped += 1;
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  std::string protocol;
  cmd.Parse (argc, argv);
  
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewRenoCSE"));
  
  NodeContainer nodes;
  nodes.Create (3);

  PointToPointHelper pointToPoint_n1n3;
  pointToPoint_n1n3.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint_n1n3.SetChannelAttribute ("Delay", StringValue ("3ms"));

  PointToPointHelper pointToPoint_n2n3;
  pointToPoint_n2n3.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint_n2n3.SetChannelAttribute ("Delay", StringValue ("3ms"));

  NetDeviceContainer device_n1n3;
  device_n1n3 = pointToPoint_n1n3.Install (nodes.Get(0),nodes.Get(2));

  NetDeviceContainer device_n2n3;
  device_n2n3 = pointToPoint_n2n3.Install (nodes.Get(1),nodes.Get(2));

  Ptr<RateErrorModel> em_n1n3 = CreateObject<RateErrorModel> ();
  em_n1n3->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  device_n1n3.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em_n1n3));

  Ptr<RateErrorModel> em_n2n3 = CreateObject<RateErrorModel> ();
  em_n2n3->SetAttribute ("ErrorRate", DoubleValue (0.00001));
  device_n2n3.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em_n2n3));

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address_n1n3;
  address_n1n3.SetBase ("10.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interface_n1n3 = address_n1n3.Assign (device_n1n3);

  Ipv4AddressHelper address_n2n3;
  address_n2n3.SetBase ("12.1.1.0", "255.255.255.252");
  Ipv4InterfaceContainer interface_n2n3 = address_n2n3.Assign (device_n2n3);

  uint16_t sinkPort1 = 8080;
  uint16_t sinkPort2 = 8081;
  uint16_t sinkPort3 = 8082;

  // Address sinkAddress1,sinkAddress2,sinkAddress3;
  // Address anyAddress1,anyAddress2,anyAddress3;

  Address sinkAddress1 (InetSocketAddress (interface_n1n3.GetAddress (1), sinkPort1));
  Address sinkAddress2 (InetSocketAddress (interface_n1n3.GetAddress (1), sinkPort2));
  Address sinkAddress3 (InetSocketAddress (interface_n2n3.GetAddress (1), sinkPort3));

  Address anyAddress1 (InetSocketAddress (Ipv4Address::GetAny (),sinkPort1));
  Address anyAddress2 (InetSocketAddress (Ipv4Address::GetAny (),sinkPort2));
  Address anyAddress3 (InetSocketAddress (Ipv4Address::GetAny (),sinkPort3));

  std::string probeType;
  std::string tracePath;

  probeType = "ns3::Ipv4PacketProbe";
  tracePath = "/NodeList/*/$ns3::Ipv4L3Protocol/Tx";

  

  PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", anyAddress1);
  PacketSinkHelper packetSinkHelper2 ("ns3::TcpSocketFactory", anyAddress2);
  PacketSinkHelper packetSinkHelper3 ("ns3::TcpSocketFactory", anyAddress3);

  ApplicationContainer sinkApps1 = packetSinkHelper1.Install (nodes.Get (2));
  ApplicationContainer sinkApps2 = packetSinkHelper2.Install (nodes.Get (2));
  ApplicationContainer sinkApps3 = packetSinkHelper3.Install (nodes.Get (2));

  sinkApps1.Start (Seconds (0.));
  sinkApps1.Stop (Seconds (30.));
  sinkApps2.Start (Seconds (0.));
  sinkApps2.Stop (Seconds (30.));
  sinkApps3.Start (Seconds (0.));
  sinkApps3.Stop (Seconds (30.));

  Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
  Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket (nodes.Get (1), TcpSocketFactory::GetTypeId ());

  Ptr<MyApp> app1 = CreateObject<MyApp> ();
  app1->Setup (ns3TcpSocket1, sinkAddress1, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app1);
  app1->SetStartTime (Seconds (1.));
  app1->SetStopTime (Seconds (20.));

  Ptr<MyApp> app2 = CreateObject<MyApp> ();
  app2->Setup (ns3TcpSocket2, sinkAddress2, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (0)->AddApplication (app2);
  app2->SetStartTime (Seconds (5.));
  app2->SetStopTime (Seconds (25.));

  Ptr<MyApp> app3 = CreateObject<MyApp> ();
  app3->Setup (ns3TcpSocket3, sinkAddress3, 3000, 100000, DataRate ("1.5Mbps"));
  nodes.Get (1)->AddApplication (app3);
  app3->SetStartTime (Seconds (15.));
  app3->SetStopTime (Seconds (30.));

  AsciiTraceHelper asciiTraceHelper1;
  AsciiTraceHelper asciiTraceHelper2;
  AsciiTraceHelper asciiTraceHelper3;

  Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper1.CreateFileStream ("node1.cwnd");
  ns3TcpSocket1->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream1));
  
  Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper2.CreateFileStream ("node2.cwnd");
  ns3TcpSocket2->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream2));
  
  Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper3.CreateFileStream ("node3.cwnd");
  ns3TcpSocket3->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream3));

  PcapHelper pcapHelper_n1n3;
  Ptr<PcapFileWrapper> file1 = pcapHelper_n1n3.CreateFile ("n1n3.pcap", std::ios::out, PcapHelper::DLT_PPP);
  device_n1n3.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file1));

  PcapHelper pcapHelper_n2n3;
  Ptr<PcapFileWrapper> file2 = pcapHelper_n2n3.CreateFile ("n2n3.pcap", std::ios::out, PcapHelper::DLT_PPP);
  device_n2n3.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeBoundCallback (&RxDrop, file2));

  Simulator::Stop (Seconds (30));
  Simulator::Run ();
  Simulator::Destroy ();
  
  std::cout<<"Total Number of packets dropped = "<<num_dropped<<std::endl;

  return 0;
}
