#include "base/base.cpp"

#include <filesystem>
#include <chrono>

using namespace std;

class TimeCounter
{
	public:
	
	chrono::high_resolution_clock::time_point timePoint;
	
	void start()
	{
		timePoint=chrono::high_resolution_clock::now();
	}
	double end()
	{
		chrono::high_resolution_clock::time_point endTimePoint=chrono::high_resolution_clock::now();
		
		return double(chrono::duration_cast<chrono::microseconds>(endTimePoint-timePoint).count())/1000000;
	}
};

class Emulator
{
	public:
	
	Pos resolution=Pos(180,180);
	size_t memorySize=0x10000000;
	uint32_t memorySizeMask=0xfffffff;
	uint32_t videoMemoryAddress=0xff00000;
	uint32_t audioMemoryAddress=0xfe00000;
	
	uint8_t stackRegisterIndex=127;
	uint8_t numberOfNonVolatileRegisters=64;
	
	Image screen;
	uint32_t buttonInput=0;
	
	uint32_t programCounter=0;
	vector<uint32_t> generalPurposeRegisters=vector<uint32_t>(128,0);
	Binary memory;
	
	TimeCounter emulationTimeCounter;
	
	Emulator()
	{
		screen=Image(resolution);
		screen.clear(0x000000);
	}
	
	void start()
	{
		emulationTimeCounter.start();
	}
	void loadProgramFromFile(const string& filePath)
	{
		memory=Binary();
		memory.loadFromFileRaw(filePath);
		if(memory.size()>memorySize)
		{
			throw string("The program is too big");
		}
		
		memory.content.resize(memorySize,0);
	}
	uint32_t getValueOfRegMem(uint8_t regMem)
	{
		if(regMem&0x80)
		{
			uint32_t memoryAddress=generalPurposeRegisters[regMem&0x7f];
			uint32_t value=memory.content[(memoryAddress+0)&memorySizeMask];
			value|=(((uint32_t)memory.content[(memoryAddress+1)&memorySizeMask])<<8);
			value|=(((uint32_t)memory.content[(memoryAddress+2)&memorySizeMask])<<16);
			value|=(((uint32_t)memory.content[(memoryAddress+3)&memorySizeMask])<<24);
			return value;
		}
		else
		{
			return generalPurposeRegisters[regMem];
		}
	}
	void setValueOfRegMem(uint8_t regMem,uint32_t value)
	{
		if(regMem&0x80)
		{
			uint32_t memoryAddress=generalPurposeRegisters[regMem&0x7f];
			memory.content[(memoryAddress+0)&memorySizeMask]=(value&0xff);
			memory.content[(memoryAddress+1)&memorySizeMask]=((value>>8)&0xff);
			memory.content[(memoryAddress+2)&memorySizeMask]=((value>>16)&0xff);
			memory.content[(memoryAddress+3)&memorySizeMask]=((value>>24)&0xff);
		}
		else
		{
			generalPurposeRegisters[regMem]=value;
		}
	}
	uint32_t readInstructionUint32FromMemory()
	{
		uint32_t value=memory.read<uint8_t>();
		memory.cursor&=memorySizeMask;
		
		value|=(((uint32_t)memory.read<uint8_t>())<<8);
		memory.cursor&=memorySizeMask;
		
		value|=(((uint32_t)memory.read<uint8_t>())<<16);
		memory.cursor&=memorySizeMask;
		
		value|=(((uint32_t)memory.read<uint8_t>())<<24);
		memory.cursor&=memorySizeMask;
		
		return value;
	}
	void emulateInstruction()
	{
		memory.cursor=programCounter;
		
		bool jump=false;
		uint32_t nextProgramCounter;
		
		uint8_t opcode=memory.read<uint8_t>();
		memory.cursor&=memorySizeMask;
		
		if(opcode&0x80)
		{
			if(opcode&0x40)
			{
				if(opcode&0x20)
				{
					if(opcode&0x10){}
					else
					{
						if(opcode&0x08)
						{
							for(int pixel=0;pixel<resolution.x*resolution.y;pixel++)
							{
								screen.data()[pixel]=
									((uint32_t)memory.content[videoMemoryAddress+pixel*4])
									|(((uint32_t)memory.content[videoMemoryAddress+pixel*4+1])<<8)
									|(((uint32_t)memory.content[videoMemoryAddress+pixel*4+2])<<16)
									|(((uint32_t)memory.content[videoMemoryAddress+pixel*4+3])<<24);
							}
						}
						else
						{
							Binary stackFrame;
							stackFrame.content.resize((128+1)*4);
							
							for(uint32_t i=0;i<stackFrame.content.size();i++)
							{
								uint32_t position=(generalPurposeRegisters[stackRegisterIndex]+i)&memorySizeMask;
								stackFrame.content[i]=memory.content[position];
							}
							
							for(int r=0;r<numberOfNonVolatileRegisters;r++)
							{
								generalPurposeRegisters[r]=stackFrame.read<uint32_t>();
							}
							
							jump=true;
							nextProgramCounter=stackFrame.read<uint32_t>();
							
							generalPurposeRegisters[stackRegisterIndex]+=(128+1)*4;
						}
					}
				}
				else
				{
					if(opcode&0x10)
					{
						uint8_t regMem=memory.read<uint8_t>();
						memory.cursor&=memorySizeMask;
						
						uint32_t value;
						
						if(opcode&0x08)
						{
							if(opcode&0x04)
							{
								value=0;//rdauds
								//--------
							}
							else
							{
								double time=emulationTimeCounter.end();
								value=uint32_t(uint64_t(time*1000));
							}
						}
						else
						{
							value=buttonInput;
						}
						
						setValueOfRegMem(regMem,value);
					}
					else
					{
						uint8_t regMem=memory.read<uint8_t>();
						memory.cursor&=memorySizeMask;
						
						uint32_t value=getValueOfRegMem(regMem);
						
						if(opcode&0x08) value=-value;
						else value=~value;
						
						setValueOfRegMem(regMem,value);
					}
				}
			}
			else
			{
				if(opcode&0x20)
				{
					uint32_t valueA=getValueOfRegMem(memory.read<uint8_t>());
					memory.cursor&=memorySizeMask;
					
					uint32_t valueB=getValueOfRegMem(memory.read<uint8_t>());
					memory.cursor&=memorySizeMask;
					
					uint32_t positionToJump;
					if(opcode&0x10)
					{
						positionToJump=readInstructionUint32FromMemory();
					}
					else
					{
						positionToJump=getValueOfRegMem(memory.read<uint8_t>());
						memory.cursor&=memorySizeMask;
					}
					
					nextProgramCounter=positionToJump;
					
					if(opcode&0x08)
					{
						if(opcode&0x04) jump= ((int32_t)valueA)>=((int32_t)valueB);
						else jump= ((int32_t)valueA)>((int32_t)valueB);
					}
					else
					{
						if(opcode&0x04)
						{
							if(opcode&0x02) jump= valueA>=valueB;
							else jump= valueA>valueB;
						}
						else
						{
							if(opcode&0x02) jump= valueA!=valueB;
							else jump= valueA==valueB;
						}
					}
				}
				else
				{
					uint32_t value;
					if(opcode&0x10)
					{
						value=readInstructionUint32FromMemory();
					}
					else
					{
						value=getValueOfRegMem(memory.read<uint8_t>());
						memory.cursor&=memorySizeMask;
					}
					
					if(opcode&0x08)
					{
						jump=true;
						nextProgramCounter=value;
					}
					else
					{
						jump=true;
						nextProgramCounter=value;
						
						generalPurposeRegisters[stackRegisterIndex]-=(128+1)*4;
						
						Binary stackFrame;
						for(int r=0;r<numberOfNonVolatileRegisters;r++)
						{
							stackFrame.write<uint32_t>(generalPurposeRegisters[r]);
						}
						stackFrame.write<uint32_t>(memory.cursor);
						
						for(uint32_t i=0;i<stackFrame.content.size();i++)
						{
							uint32_t position=(generalPurposeRegisters[stackRegisterIndex]+i)&memorySizeMask;
							memory.content[position]=stackFrame.content[i];
						}
					}
				}
			}
		}
		else
		{
			uint8_t regMemA=memory.read<uint8_t>();
			memory.cursor&=memorySizeMask;
			
			uint32_t valueA=getValueOfRegMem(regMemA);
			uint32_t valueB;
			if(opcode&0x40)
			{
				valueB=readInstructionUint32FromMemory();
			}
			else
			{
				valueB=getValueOfRegMem(memory.read<uint8_t>());
				memory.cursor&=memorySizeMask;
			}
			
			if(opcode&0x20)
			{
				if(opcode&0x10)
				{
					if(opcode&0x08)
					{
						if(valueB<32)
						{
							valueA=(valueA>>valueB)|((valueA&0x80000000) ? (~(0xffffffff>>valueB)) : 0);
						}
						else valueA=((valueA&0x80000000) ? 0xffffffff : 0);
					}
					else
					{
						if(valueB!=0) valueA=((int32_t)valueA)%((int32_t)valueB);
					}
				}
				else
				{
					if(valueB!=0) valueA=((int32_t)valueA)/((int32_t)valueB);
					else valueA=0;
				}
			}
			else
			{
				if(opcode&0x10)
				{
					if(opcode&0x08)
					{
						if(opcode&0x04)
						{
							if(opcode&0x02)
							{
								if(valueB!=0) valueA%=valueB;
							}
							else
							{
								if(valueB!=0) valueA/=valueB;
								else valueA=0;
							}
						}
						else
						{
							if(opcode&0x02) valueA*=valueB;
							else valueA>>=valueB;
						}
					}
					else
					{
						if(opcode&0x04)
						{
							if(opcode&0x02)
							{
								if(valueB<32) valueA<<=valueB;
								else valueA=0;
							}
							else valueA=valueA^valueB;
						}
						else
						{
							if(opcode&0x02) valueA|=valueB;
							else valueA&=valueB;
						}
					}
				}
				else
				{
					if(opcode&0x08)
					{
						if(opcode&0x04) valueA-=valueB;
						else valueA+=valueB;
					}
					else valueA=valueB;
				}
			}
			
			setValueOfRegMem(regMemA,valueA);
		}
		
		programCounter=memory.cursor;
		if(jump)
		{
			programCounter=nextProgramCounter&memorySizeMask;
		}
	}
	void emulate(double time)
	{
		TimeCounter timeCounter;
		timeCounter.start();
		
		while(true)
		{
			emulateInstruction();
			if(timeCounter.end()>time) break;
		}
	}
	void setButtonInput(uint32_t _buttonInput)
	{
		buttonInput=_buttonInput;
	}
};

int main(int argc,char*argv[])
{
	try
	{
	if(argc<1)
	{
		throw string("Cannot get the path of the executable");
	}
	
	string argv0=string(argv[0]);
	
	if(argv0.find_first_of("/\\")!=string::npos)
	{
		filesystem::path path=argv0;
		path.remove_filename();
		filesystem::current_path(path);
	}
	
	vector<string> args;
	for(int i=1;i<argc;i++)
	{
		args.emplace_back(argv[i]);
	}
	
	if(args.size()!=1)
	{
		cout<<"Wrong number of parameters. Use the format: 'emulate code.bin'."<<endl;
		return 1;
	}
	else
	{
		Window window;
		window.create();
		Image windowIcon("application/icon.bmp");
		window.setWindowIcon(windowIcon);
		
		Emulator emulator;
		emulator.loadProgramFromFile(args[0]);
		
		emulator.start();
		
		double emulationTimePerFrame=0.02;
		
		while(window.pollEventsAndUpdateInputAndTime())
		{
			uint32_t buttonInput=0;
			if(window.input.key['W']) buttonInput|=(1<<0);
			if(window.input.key['A']) buttonInput|=(1<<1);
			if(window.input.key['S']) buttonInput|=(1<<2);
			if(window.input.key['D']) buttonInput|=(1<<3);
			if(window.input.key['I']) buttonInput|=(1<<4);
			if(window.input.key['J']) buttonInput|=(1<<5);
			if(window.input.key['K']) buttonInput|=(1<<6);
			if(window.input.key['L']) buttonInput|=(1<<7);
			if(window.input.key['C']) buttonInput|=(1<<8);
			if(window.input.key['N']) buttonInput|=(1<<9);
			
			emulator.setButtonInput(buttonInput);
			
			emulator.emulate(emulationTimePerFrame);
			
			window.screen.clear(0x000000);
			
			Pos windowCenter=window.getWindowSize()/2;
			
			int screenScaleFactor=3;
			Pos screenSize=emulator.screen.size()*screenScaleFactor;
			Pos screenBase=windowCenter-screenSize/2;
			for(int y=0;y<emulator.screen.size().y;y++)
			{
				for(int x=0;x<emulator.screen.size().x;x++)
				{
					window.screen.rectfill(screenBase+Pos(x,y)*screenScaleFactor,screenBase+Pos(x+1,y+1)*screenScaleFactor-Pos(1,1),emulator.screen._getpixel(Pos(x,y)));
				}
			}
			window.screen.rect(screenBase-Pos(2,2),screenBase+screenSize+Pos(1,1),0xffffff);
			
			window.drawScreen();
		}
		
		window.destroy();
	}
	
	}catch(const string& str)
	{
		cout<<str<<endl;
	}
	
	return 0;
}

