#include "NetCtrl.hpp"

#include <cstdlib>
#include <cstdint>
#include <sstream>

#include "Device.hpp"
#include "MotorHdl.hpp"
#include "SensorHdl.hpp"
#include "ConfigFile.hpp"

#include "StabCtrl.hpp"



#define NET_INFOREQUEST (uint8_t)1
#define NET_INFODATA (uint8_t)2
#define NET_CHANGEMOTORSPEED (uint8_t)3

NetCtrl::NetCtrl(ConfigFile* cfg)
{
	m_nSockPort = cfg->GetValue<int>("NET_Port");
}


void NetCtrl::OnThreadStart()
{
    //Init server socket
    m_sockServer = socket(AF_INET, SOCK_STREAM, 0);
    if(m_sockServer < 0)
    {
        std::cerr<<"\e[31mError while opening server socket ("<<strerror(errno)<<")\e[m"<<std::endl;
        int nSeq[] = {200, 0, 200, 0, 200, 0, 200, 0, 200};
        Device::GetDevice()->BipRoutine(nSeq, 9);
        sleep(5);
    }

    //Init server address
    bzero((char *) &m_addrServer, sizeof(m_addrServer));
    m_addrServer.sin_family = AF_INET;
    m_addrServer.sin_addr.s_addr = INADDR_ANY;
    m_addrServer.sin_port = htons(m_nSockPort);

	//Bind server address & server socket
    if(bind(m_sockServer, (struct sockaddr *) &m_addrServer, sizeof(m_addrServer)) < 0)
    {
        std::cerr<<"\e[31mError while binding server socket ("<<strerror(errno)<<")\e[m"<<std::endl;
        int nSeq[] = {200, 0, 200, 0, 200, 0, 200, 0, 200};
        Device::GetDevice()->BipRoutine(nSeq, 9);
        sleep(5);
    }

    //Tell the device to watch the socket
    listen(m_sockServer,5);
}



void NetCtrl::ThreadProcess()
{
    std::clog<<"\e[33mWaiting for client connection...\e[m"<<std::endl;
    struct sockaddr_in m_addrClient;
    socklen_t addrlenClient = sizeof(m_addrClient);
    m_sockClient = accept(m_sockServer, (struct sockaddr *) &m_addrClient, &addrlenClient);
    if (m_sockClient < 0)
    {
        std::cerr<<"\e[31mError while accepting client connexion ("<<strerror(errno)<<")\e[m"<<std::endl;
        sleep(5);
    }

	//Convert bytes to readable IPv4 address to print it into the console
    short addrA = m_addrClient.sin_addr.s_addr/16777216;// /2^24
    short addrB = (m_addrClient.sin_addr.s_addr-addrA*16777216)/65536;// /2^16
    short addrC = (m_addrClient.sin_addr.s_addr-addrA*16777216-addrB*65536)/256;// /2^8
    short addrD = m_addrClient.sin_addr.s_addr-addrA*16777216-addrB*65536-addrC*256;
    std::clog<<"\e[33mConnection from client: "<<addrD<<"."<<addrC<<"."<<addrB<<"."<<addrA<<"\e[m"<<std::endl;

	//Read & process net data loop
    while(!GetIsThreadQuitting() && m_sockClient>=0)
    {
		//Read the socket and gets the Packet Size
        uint16_t nPacketSize;
        int n = read(m_sockClient,(char*)(&nPacketSize),2);
        if (n <= 0)break;//If connection is closed, break

		//Extract the data to be processed
        char cData[nPacketSize];
        int m = read(m_sockClient,(char*)(&cData),nPacketSize);
        if (m <= 0)break;//If connection is closed, break

		//Process the data
        ProcessNetData(cData);
    }

    close(m_sockClient);
    std::clog<<"\e[33mClient connection closed\e[m"<<std::endl;
}

void NetCtrl::OnThreadStop()
{
	close(m_sockClient);
}

//Handy functions
void WriteSSTream16(std::stringstream& ss, uint16_t i)
{
    uint16_t nTmp;
    if(i==0)
    {
        ss.write((char*)(&(nTmp=0)), 2);
    }
    else if(i<256)
    {
        ss.write((char*)(&(nTmp=0)), 1);
        ss.write((char*)(&(nTmp=i)), 1);
    }
    else
    {
        ss.write((char*)(&(nTmp=i/256)), 1);
        ss.write((char*)(&(nTmp=i-i/256)), 1);
    }
}
void WriteSSTream8(std::stringstream& ss, uint8_t i)
{
    ss.write((char*)(&(i)), 1);
}
//

void NetCtrl::ProcessNetData(const char* data)
{
    std::istringstream sData(data);

    uint8_t nAction;
    sData.read((char*)(&nAction), 1);

    if(nAction == NET_INFOREQUEST)
    {
    	#ifdef DEBUG
        std::cout<<"#IR#";
		#endif

        std::stringstream sToSend;

        //Reserve space for packet size
        WriteSSTream16(sToSend, 0);

        WriteSSTream8(sToSend, NET_INFODATA);

        //Motors
        // The sensors values are in [0, +100] G
        WriteSSTream16(sToSend, Device::GetMotors()->GetSpeed(1)*655.36);
        WriteSSTream16(sToSend, Device::GetMotors()->GetSpeed(2)*655.36);
        WriteSSTream16(sToSend, Device::GetMotors()->GetSpeed(3)*655.36);
        WriteSSTream16(sToSend, Device::GetMotors()->GetSpeed(4)*655.36);

        //Sensors
        // The sensors values are in [-3, +3] G
        //TODO Optimize the values for moving in m.s-2
        WriteSSTream16(sToSend, Device::GetSensors()->GetAcceleration().x);
        WriteSSTream16(sToSend, Device::GetSensors()->GetAcceleration().y);
        WriteSSTream16(sToSend, Device::GetSensors()->GetAcceleration().z);
        WriteSSTream16(sToSend, Device::GetSensors()->GetAngularSpeed());

        //Write packet size
        uint16_t nPacketSize = sToSend.tellp();
        sToSend.seekp(std::ios_base::beg);
        WriteSSTream16(sToSend, nPacketSize-2);

        //Sending packet
        int m = write(m_sockClient, sToSend.str().c_str(), sToSend.str().length());
        if (m <= 0)std::cerr<<__FILE__<<" @ "<<__LINE__<<" : Error while sending InfoData ("/*<<strerror(errno)*/<<")"<<std::endl;
    }
    else if(nAction == NET_INFODATA)
    {
    	#ifdef DEBUG
        std::cout<<"#ID#";
		#endif

        //Should not happen
        std::cerr<<"Received NET_INFODATA, and it should not happen"<<std::endl;
    }
    else if(nAction == NET_CHANGEMOTORSPEED)
    {
    	#ifdef DEBUG
        std::cout<<"#CMS#";
		#endif

        uint16_t nValue;
        uint8_t nTmp;
        sData.read((char*)(&nTmp), 1);
        nValue = 256*nTmp;
        sData.read((char*)(&nTmp), 1);
        nValue+=nTmp;

        Device::GetStabCtrl()->ChangeMotorSpeed(nValue/327.68 - 100.0);
    }


}
