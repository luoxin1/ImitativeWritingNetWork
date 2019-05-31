#ifndef __ENDIAN_H__ 
#define __ENDIAN_H__
#include<endian.h>
#include <stdint.h>
#include<iostream>

namespace sockets
{
	inline uint16_t hostToNetwork16(uint16_t host16)
	{
		return htobe16(host16);
	}

	inline uint32_t hostToNetwork32(uint32_t host32)
	{
		return htobe32(host32);
	}

	inline uint64_t hostToNetwork64(uint64_t host64)
	{
		return htobe64(host64);
	}

	inline uint16_t networkToHost16(uint16_t net16)
	{
		return be16toh(net16);
	}

	inline uint32_t networkToHost32(uint32_t net32)
	{
		return be32toh(net32);
	}

	inline uint64_t networkToHost64(uint64_t net64)
	{
		return be64toh(net64);
	}

	inline bool isMachineNetworkEndian()
	{
                std::cout<<"2222222222222"<<std::endl;
		uint16_t number = 0x01;
		return *reinterpret_cast<uint8_t*>(&number) == 0x00;
	}
}
#endif // !

