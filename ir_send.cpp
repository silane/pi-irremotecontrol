#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <limits>
#include <unistd.h>
#include <wiringPi.h>

constexpr unsigned short INPUT_NO_CHANGE_LIMIT_TIME=40000;
constexpr int INPUT_TEST_INTERVAL=10;

double getTimevalDiffInMicrosec(const timeval &after,const timeval &before)
{
	return (after.tv_sec-before.tv_sec)*1000000.0+(after.tv_usec-before.tv_usec);
}

int ir_send(int pin_num,std::istream &in,int count)
{
	std::string::size_type start=0,pos=0;
	std::string str;
	std::vector<int> record;

	std::getline(in,str);
	while((pos=str.find(',',start))!=std::string::npos)
	{
		record.push_back(std::stoi(str.substr(start,pos-start)));
		start=pos+1;
	}

	pinMode(pin_num,PWM_OUTPUT);

	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(252);
	pwmSetClock(2);

	piHiPri(99);
	for(int i=0;i<count;i++)
	{
		bool state=true;
		for(std::vector<int>::iterator itr=record.begin();itr!=record.end();++itr)
		{
			if(state){
				pwmWrite(pin_num,252/3);
			}else{
				pwmWrite(pin_num,0);
			}
			delayMicroseconds(*itr);

			state=!state;
		}
		pwmWrite(pin_num,0);
		delayMicroseconds(100*1000);
	}

	piHiPri(0);
	std::clog << "Send succeed" << std::endl;
	return 0;
}

void printUsage(const char *program_name)
{
	std::cout << "Usage :";
	std::cout << program_name << " [-g] [-c <count>] [-f <filename>] <pin_num> " << std::endl;
	std::cout << "  Send IR signal from pin number <pin_num> (<pin_num> must be PWM writable)" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -g                     <pin_num> is Broadcom GPIO pin number (Default is wiringPi pin number)" << std::endl;
	std::cout << "  -f <filename>          Input send data from <filename> (Default is stdin)" << std::endl;
	std::cout << "  -c <count>             Send <count> times repeatedly (Default is 1)" << std::endl;
}

int main(int argc,char *argv[])
{
	int opt;
	char *endptr;
	bool useGPIONumbering=false;
	long pin_num;
	long count=1; 
	std::ifstream in;
	while((opt=getopt(argc,argv,"gf:c:"))!=-1)
	{
		switch(opt)
		{
		case 'g':
			useGPIONumbering=true;
			break;
		case 'f':
			in.open(optarg);
			if(in.fail())
			{
				std::cerr << "opening file failed" << std::endl;
				return 1;
			}
			break;
		case 'c':
			count=strtol(optarg,&endptr,10);

			if(*endptr!='\0' || count<0 || std::numeric_limits<int>::max()<count)
			{
				printUsage(argv[0]);
				return 1;
			}
		}
	}

	if(argc-optind!=1)
	{
		printUsage(argv[0]);
		return 1;
	}

	pin_num=strtol(argv[optind],&endptr,10);
	if(*endptr!='\0' || pin_num<0 || std::numeric_limits<int>::max()<pin_num)
	{
		printUsage(argv[0]);
		return 1;
	}

	int ret;
	if(useGPIONumbering)
		ret=wiringPiSetupGpio();
	else
		ret=wiringPiSetup();
	if(ret==-1)
	{
		std::cerr << "initialization of wiringPi failed" << std::endl;
		return 1;
	}

	if(in.is_open())
		return ir_send((int)pin_num,in,(int)count);
	else
		return ir_send((int)pin_num,std::cin,(int)count);
}

