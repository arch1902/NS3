#ifndef TCPCONGESTIONOPS_H
#define TCPCONGESTIONOPS_H

#include "ns3/tcp-socket-state.h"

namespace ns3 {

class TcpCongestionOps : public Object
{
public:

  static TypeId GetTypeId (void);

  TcpCongestionOps ();

  TcpCongestionOps (const TcpCongestionOps &other);

  virtual ~TcpCongestionOps ();

  virtual std::string GetName () const = 0;

  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight) = 0;


  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked) = 0;


  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                          const Time& rtt)
  {
    NS_UNUSED (tcb);
    NS_UNUSED (segmentsAcked);
    NS_UNUSED (rtt);
  }


  virtual void CongestionStateSet (Ptr<TcpSocketState> tcb,
                                   const TcpSocketState::TcpCongState_t newState)
  {
    NS_UNUSED (tcb);
    NS_UNUSED (newState);
  }


  virtual void CwndEvent (Ptr<TcpSocketState> tcb,
                          const TcpSocketState::TcpCAEvent_t event)
  {
    NS_UNUSED (tcb);
    NS_UNUSED (event);
  }


  virtual Ptr<TcpCongestionOps> Fork () = 0;
};

class TcpNewRenoCSE : public TcpCongestionOps
{
public:

  static TypeId GetTypeId (void);

  TcpNewRenoCSE ();

  TcpNewRenoCSE (const TcpNewRenoCSE& sock);

  ~TcpNewRenoCSE ();

  std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual Ptr<TcpCongestionOps> Fork ();

protected:
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
};

} // namespace ns3

#endif // TCPCONGESTIONOPS_H
