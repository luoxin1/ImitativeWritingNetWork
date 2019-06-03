/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on May 21, 2019, 3:55 PM
 */

#include <cstdlib>
//#include"./tests/NioEventLoopTest.h"
//#include"./tests/ByteBufeTest.h"
#include"./tests/echoServer.h"
#include"./tests/echoClient.h"

using namespace std;

int main(int argc, char** argv) 
{
//    NioEventLoopTest test;
//    test.nioEventLoopTestMain();
//    ByteBufeTest test;
//    test.byteBufeTestMain();
    echoServerMain();
//    echoClientMain();
    
    
    return 0;
}

