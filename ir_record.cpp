#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <limits>
#include <memory>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h>

constexpr unsigned short INPUT_NO_CHANGE_LIMIT_TIME=10000;//frame interval threhold
constexpr int INPUT_TEST_INTERVAL=10;

double getTimevalDiffInMicrosec(const timeval &after,const timeval &before)
{
	return (after.tv_sec-before.tv_sec)*1000000.0+(after.tv_usec-before.tv_usec);
}

int ir_record(int pin_num,int frame_count,std::ostream &out)
{
	constexpr int NO_CHANGE_LIMIT_COUNT=INPUT_NO_CHANGE_LIMIT_TIME/INPUT_TEST_INTERVAL;
	bool state=false;
	timeval start,now;
	std::vector<unsigned short> record;
	record.reserve(500);

	pinMode(pin_num,INPUT);

	std::clog << "Ready to record..." << std::endl;

	piHiPri(99);
	while(digitalRead(pin_num))
	{
		delayMicroseconds(INPUT_TEST_INTERVAL);
	}

	gettimeofday(&start,NULL);
	for(int i=0;;i++)
	{
		if(i>=NO_CHANGE_LIMIT_COUNT)
		{
			if(!state)
			{
				std::cerr << "Receiver's on state (GPIO's low state) continued too long time. Something may be wrong with a circuit." << std::endl;
				return 1;
			}else{
				frame_count--;
				i=std::numeric_limits<int>::min();
				if(frame_count<=0)
				{
					std::clog << "Finished!" << std::endl;
					for(std::vector<unsigned short>::iterator itr=record.begin();itr!=record.end();++itr)
					{
						out<<*itr<<",";
					}
					out<<std::endl;
					return 0;
				}
			}
		}

		if(digitalRead(pin_num)!=state)
		{
			gettimeofday(&now,NULL);
			record.push_back((unsigned short)getTimevalDiffInMicrosec(now,start));
			start=now;
			state=!state;
			i=0;
		}
		delayMicroseconds(INPUT_TEST_INTERVAL);
	}
}

void printUsage(const char *program_name)
{
	std::cout << "Usage: ";
	std::cout << program_name << " [-g] [-f <filename>] [-c <frame-count>] <pin_num>" << std::endl;
	std::cout << "  Record IR signal from <pin_num>" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -g                     <pin_num> is Broadcom GPIO pin number (Default is wiringPi pin number)" << std::endl;
	std::cout << "  -f <filename>          Output recorded data to <filename> (Default is stdout)" << std::endl;
	std::cout << "  -c <frame-count>       Record <frame-count> frames (Default is 1)" << std::endl;
}

int main(int argc,char *argv[])
{
	int opt;
	bool useGPIONumbering=false;
	long pin_num=-1;
	std::ofstream out;
	long numberOfFrames=1;
	char *endptr;

	while((opt=getopt(argc,argv,"gf:c:"))!=-1)
	{
		switch(opt)
		{
		case 'g':
			useGPIONumbering=true;
			break;
		case 'f':
			out.open(optarg);
			if(out.fail())
			{
				std::cout << "opening file failed" << std::endl;
				printUsage(argv[0]);
				return 1;
			}
			break;
		case 'c':
			numberOfFrames=strtol(optarg,&endptr,10);
			if(*endptr!='\0' || numberOfFrames<0 || std::numeric_limits<int>::max()<numberOfFrames)
			{
				printUsage(argv[0]);
				return 1;
			}
			break;
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

	if(out.is_open())
		return ir_record((int)pin_num,(int)numberOfFrames,out);
	else
		return ir_record((int)pin_num,(int)numberOfFrames,std::cout);
}

