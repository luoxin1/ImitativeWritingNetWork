#ifndef __ENDIAN_H__ 
#define __ENDIAN_H__
namespace sockets
{
	inline uint16_t hostToNetwork16(uint16_t host16)
	{
		return htob16(host16);
	}

	inline uint32_t hostToNetwork16(uint32_t host32)
	{
		return htob32(host32);
	}

	inline uint64_t hostToNetwork16(uint64_t host64)
	{
		return htob64(host64);
	}

	inline uint16_t networkToHost16(uint16_t net16)
	{
		return be16toh(net16);
	}

	inline uint32_t networkToHost32(uint32_t net32)
	{
		return be32toh(net32);
	}

	inline uint64_t networkToHost16(uint64_t net64)
	{
		return be64toh(net64);
	}

	inline bool isMachineNetworkEndian()
	{
		uint16_t number = 0x01;
		return reinterpret_cast<unint8*>(&number) == 0x00;
	}
}
#endif // !

