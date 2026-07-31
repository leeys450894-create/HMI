#pragma once

//#ifndef _DEBUG
#pragma comment (lib, "AccuraClient")
//#else
//#pragma comment (lib, "AccuraClientDbg")
//#endif

enum ModbusTcpError {
	ModbusTcpErrorOnOpen,
	ModbusTcpErrorOnClose,
	ModbusTcpErrorTransactionTimeout,
	ModbusTcpErrorRemoteConnectionClosed,
	ModbusTcpErrorOnTransaction,
};

class AccuraModbusTcpClient
{
public:
	static AccuraModbusTcpClient* Create(const char* ipAddress);
	static AccuraModbusTcpClient* Create(unsigned long ipAddress);

	virtual ~AccuraModbusTcpClient(void) {};

	virtual void SetTimeout(int timeout) = 0;
	virtual int GetTimeout() = 0;

	virtual void SetConnectionCheckTime(int interval) = 0;
	virtual int GetConnectionCheckTime() = 0;

	virtual void EnableException() = 0;
	virtual void DisableException() = 0;

	virtual bool GetProductId(unsigned short& productId) = 0;
	virtual unsigned short GetProductId() = 0;

	virtual bool ReadHoldingRegisters(unsigned address, unsigned len, unsigned short* buffer) = 0;
};

class AccuraClients
{
public:
	static AccuraClients* Create();

	virtual ~AccuraClients() {};

	virtual void AddDevice(const char* ipAddress) = 0;
	virtual void AddDevice(unsigned long ipAddress) = 0;
	virtual void RemoveDevice(int index) = 0;
	virtual int Count() = 0;

	virtual AccuraModbusTcpClient* At(int index) = 0;
	virtual AccuraModbusTcpClient** GetDevicesAsAllocatedArray(int& size) = 0;
};

class ModbusTcpClient
{
public:
	static ModbusTcpClient* Create(const char* ipAddress);
	static ModbusTcpClient* Create(unsigned long ipAddress);

	virtual ~ModbusTcpClient(void) {};

	virtual void SetTimeout(int timeout) = 0;
	virtual int GetTimeout() = 0;

	virtual void Open() = 0;
	virtual void Close() = 0;
	virtual bool IsOpen() = 0;

	virtual void ReadHoldingRegisters(unsigned address, unsigned len, unsigned short* buffer) = 0;
};


// for explicit link only
AccuraModbusTcpClient* CreateAccuraModbusTcpClientFromIpString(const char* ipAddress);
AccuraModbusTcpClient* CreateAccuraModbusTcpClientFromIpValue(unsigned long ipAddress);
AccuraClients* CreateAccuraClients();
ModbusTcpClient* CreateModbusTcpClientFromIpString(const char* ipAddress);
ModbusTcpClient* CreateModbusTcpClientFromIpValue(unsigned long ipAddress);
