#ifndef __BYTEBUFETEST_H__
#define __BYTEBUFETEST_H__
#include "../ByteBuf.h"
#include <iostream>
#include <string>
#include "../Buffer.h"
#include "boost/shared_ptr.hpp"

class ByteBufeTest
{
public:
	void byteBufeTestMain()
	{
//            testBytezbufRead();
            testByteBufWrite();
//            testByteBufMove();
	}

private:
	void testBytezbufRead()
	{
		Bytebuf buf;
		if (buf.readalbeBytes() == 0)
		{
			std::cout << "buf.readalbeBytes() is 0" << std::endl;
		}
		else
		{
			std::cout << "buf.readalbeBytes() is not 0" << std::endl;
		}

		if (buf.empty())
		{
			std::cout << "buf is empty" << std::endl;
		}
		else
		{
			std::cout << "buf is not empty " << std::endl;
		}
	}

	void testByteBufWrite()
	{
		std::string s(100, 'a');

		Bytebuf buf;
		buf.append(s.data(), s.size());
		if (buf.readalbeBytes() == 100)
		{
			std::cout << "buf.readalbeBytes() is 100" << std::endl;
		}
		else
		{
			std::cout << "buf.readalbeBytes() is not 100" << std::endl;
		}

		if (buf.empty())
		{
			std::cout << "buf is empty" << std::endl;
		}
		else
		{
			std::cout << "buf is not empty " << std::endl;
		}


		buf.append(s);
		if (buf.readalbeBytes() == 200)
		{
			std::cout << "buf.readalbeBytes() is 200" << std::endl;
		}
		else
		{
			std::cout << "buf.readalbeBytes() is not 200" << std::endl;
		}

		if (buf.empty())
		{
			std::cout << "buf is empty" << std::endl;
		}
		else
		{
			std::cout << "buf is not empty " << std::endl;
		}

		buf.discardBytest(10);
		if (buf.readalbeBytes() == 190)
		{
			std::cout << "buf.readalbeBytes() is 190" << std::endl;
		}
		else
		{
			std::cout << "buf.readalbeBytes() is not 190" << std::endl;
		}

		if (buf.empty())
		{
			std::cout << "buf is empty" << std::endl;
		}
		else
		{
			std::cout << "buf is not empty " << std::endl;
		}

		char cache[100] = { 0 };
		buf.peekBytes(cache, sizeof(cache));
		std::cout <<"cache:"<< cache <<" readalbeBytes: "<<buf.readalbeBytes()<< std::endl;

		Buffer b;
		buf.readBytes(b, 50);
		std::cout <<"buf.readBytes(b, 50) "<< buf.readalbeBytes() << std::endl;

		Bytebuf tmp;
		tmp.append(buf);
		std::cout << "Bytebuf tmp.append(buf) " << buf.readalbeBytes() << std::endl;
		
		std::cout << "Bytebuf tmp.append(buf) " << tmp.readalbeBytes() << std::endl;

		{
			Bytebuf localBf;
			localBf.appendReference(std::move(b));
			std::cout << "localBf " << localBf.readalbeBytes() << std::endl;

			char abc[50] = { 0 };
			localBf.peekBytes(abc, 20);
			std::cout << "localBf " << localBf.readalbeBytes() << std::endl;

			std::cout << "after peek bytes "<<std::endl;

			localBf.readBytes(abc, 50);
			std::cout << "localBf " << localBf.readalbeBytes() << std::endl;

			std::cout << "after read bytes " << std::endl;
		}
                                

		std::cout << "exit local..." << std::endl;
            
	}

	void testByteBufMove()
	{
		std::string s(100, 'a');
		Bytebuf buf;

		buf.append(s.data(), s.size());

		Bytebuf buf1(std::move(buf));
		std::cout << "localBf " << buf1.readalbeBytes() << std::endl;


		Bytebuf buf2;
		buf2 = std::move(buf1);
		std::cout << "localBf " << buf2.readalbeBytes() << std::endl;
	}
};

#endif
