//#include "WebServer.h"
//#include "StringUtility.h"
//#include <iostream>
//#include <string>
//#include <iomanip>
//#include <sstream>
//using namespace std;
//
//void test_sha1();
//void test_encodeBase64();
//
// int main() {
// 	std::string ip = "127.0.0.1";
// 	int port = 8080;
// 	//// 執行ioc.run()，系統會被block住，直到所有用到此ioc的thread結束才會往下執行。
// 	//ioc.run();
// 	WebServer server(ip, port);
// 	server.run();
// 	return 0;
// }
//
//void test_sha1() {
//	cout << "test_sha1" << endl;
//	stringstream ss;
//	string src = StringUtility::sha1("I am string.");
//	cout << src << endl;
//	string y = "5bdf667b1dfbf56cdaeeb9aff97cb3c95deb3241";
//	for (int i = 0; i < src.length(); i++) {
//		int tmp = int(src[i]);
//		ss << hex << ((tmp & 240) >> 4);
//		ss << hex << (tmp & 15);
//	}
//	string x = ss.str();
//	cout << x << endl;
//	cout << (x == y) << endl;
//	cout << endl;
//}
//
//void test_encodeBase64() {
//	cout << "test_encodeBase64" << endl;
//	stringstream ss;
//	string x = StringUtility::encodeBase64("I am string.");
//	string y = "SSBhbSBzdHJpbmcu";
//	cout << x << endl;
//	cout << (x == y) << endl;
//	cout << endl;
//}
