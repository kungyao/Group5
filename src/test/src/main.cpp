#include "WebServer.h"

int main() {
	std::string ip = "127.0.0.1";
	int port = 8080;
	//// 執行ioc.run()，系統會被block住，直到所有用到此ioc的thread結束才會往下執行。
	//ioc.run();
	WebServer server(ip, port);
	server.run();
	return 0;
}