#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <exception>
#include <algorithm>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include <filesystem>

using namespace std;

#include "assembler_binary.cpp"



string fileToString(const string& filepath)
{
	Binary binary;
	binary.loadFromFileRaw(filepath);
	string str;
	str.reserve(binary.size());
	for(uint64_t i=0;i<binary.size();i++)
	{
		str.push_back(binary.content[i]);
	}
	return str;
}

void stringToFile(const string& str,const string& filepath)
{
	Binary binary;
	binary.resize(str.size());
	for(uint64_t i=0;i<binary.size();i++)
	{
		binary.content[i]=str[i];
	}
	binary.saveToFileRaw(filepath);
}

vector<string> fileToLines(const string& filepath)
{
	string file=fileToString(filepath);
	
	vector<string> lines;
	size_t base=0;
	for(size_t p=0;p<file.size();)
	{
		if(file[p]=='\n' || file[p]=='\r')
		{
			lines.push_back(file.substr(base,p-base));
			char c=file[p];
			p++;
			if(p<file.size())
			{
				if((file[p]=='\n' || file[p]=='\r') && file[p]!=c)
				{
					p++;
				}
			}
			base=p;
		}
		else
		{
			p++;
		}
	}
	lines.push_back(file.substr(base,string::npos));
	
	return lines;
}

class LineLocation
{
	public:
	size_t lineNumber=0;
	string fileName;
	
	LineLocation(){}
	LineLocation(size_t _lineNumber,const string& _fileName)
	{
		lineNumber=_lineNumber;
		fileName=_fileName;
	}
};

class AssemblyCodeLine
{
	public:
	string codeStr;
	LineLocation location;
	
	AssemblyCodeLine(){}
	AssemblyCodeLine(const string& _codeStr,const LineLocation& _location)
	{
		codeStr=_codeStr;
		location=_location;
	}
	string makeErrorMessage(const string& message) const
	{
		return string("Error at '")+location.fileName+"':"+to_string(location.lineNumber)+": "+message;
	}
};

class AssemblyCode
{
	public:
	vector<AssemblyCodeLine> lines;
};

class LabelDefinition
{
	public:
	
	string labelName;
	size_t positionInMachineCode=0;
	
	LabelDefinition(){}
	LabelDefinition(const string& _labelName,size_t _positionInMachineCode)
	{
		labelName=_labelName;
		positionInMachineCode=_positionInMachineCode;
	}
};

class LabelUse
{
	public:
	
	string labelName;
	size_t positionInMachineCode=0;
	
	AssemblyCodeLine codeLine;
	
	LabelUse(){}
	LabelUse(const string& _labelName,size_t _positionInMachineCode,const AssemblyCodeLine& _codeLine)
	{
		labelName=_labelName;
		positionInMachineCode=_positionInMachineCode;
		
		codeLine=_codeLine;
	}
};

class Mnemonic
{
	public:
	
	string name;
	string parameterTypes;
	
	uint8_t opcode=0;
	uint8_t opcodeWithImmediate=0;
	
	Mnemonic(){}
	Mnemonic(const string& _name,const string& _parameterTypes,uint8_t _opcode,uint8_t _opcodeWithImmediate=0)
	{
		name=_name;
		parameterTypes=_parameterTypes;
		
		opcode=_opcode;
		opcodeWithImmediate=_opcodeWithImmediate;
	}
};

class InstructionParameter
{
	public:
	
	bool isImmediate=false;
	bool isImmediateLabel=false;
	bool isMemory=false;
	
	int registerIndex=0;
	uint32_t immediateValue=0;
	string immediateLabelName;
};

class Assembler
{
	public:
	
	vector<Mnemonic> mnemonics;
	
	Assembler()
	{
		mnemonics.emplace_back("nop","",0xf0);
		mnemonics.emplace_back("ret","",0xe0);
		mnemonics.emplace_back("draw","",0xe8);
		
		mnemonics.emplace_back("not","r",0xc0);
		mnemonics.emplace_back("neg","r",0xc8);
		mnemonics.emplace_back("rdbtn","r",0xd0);
		mnemonics.emplace_back("rdclk","r",0xd8);
		mnemonics.emplace_back("rdauds","r",0xdc);
		
		mnemonics.emplace_back("call","i",0x80,0x90);
		mnemonics.emplace_back("jmp","i",0x88,0x98);
		
		mnemonics.emplace_back("mov","ri",0x00,0x40);
		mnemonics.emplace_back("and","ri",0x10,0x50);
		mnemonics.emplace_back("or","ri",0x12,0x52);
		mnemonics.emplace_back("xor","ri",0x14,0x54);
		mnemonics.emplace_back("add","ri",0x08,0x48);
		mnemonics.emplace_back("sub","ri",0x0c,0x4c);
		mnemonics.emplace_back("mul","ri",0x1a,0x5a);
		mnemonics.emplace_back("div","ri",0x1c,0x5c);
		mnemonics.emplace_back("divs","ri",0x20,0x60);
		mnemonics.emplace_back("mod","ri",0x1e,0x5e);
		mnemonics.emplace_back("mods","ri",0x30,0x70);
		mnemonics.emplace_back("shl","ri",0x16,0x56);
		mnemonics.emplace_back("shr","ri",0x18,0x58);
		mnemonics.emplace_back("shrs","ri",0x38,0x78);
		
		mnemonics.emplace_back("je","rri",0xa0,0xb0);
		mnemonics.emplace_back("jne","rri",0xa2,0xb2);
		mnemonics.emplace_back("jg","rri",0xa4,0xb4);
		mnemonics.emplace_back("jge","rri",0xa6,0xb6);
		mnemonics.emplace_back("jgs","rri",0xa8,0xb8);
		mnemonics.emplace_back("jges","rri",0xac,0xbc);
	}
	
	int getRegisterIndex(const string& str)
	{
		if(str.size()>0)
		{
			if(str[0]=='r')
			{
				if(str.size()>1)
				{
					if(str[1]=='s' && str.size()==2)
					{
						return 127;
					}
					else if(str[1]=='v' && str.size()>2)
					{
						int number=0;
						try
						{
							number=stoi(str.substr(2));
						}
						catch(...)
						{
							return -1;
						}
						
						if(number>=0 && number<=62)
						{
							return 64+number;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						int number=0;
						try
						{
							number=stoi(str.substr(1));
						}
						catch(...)
						{
							return -1;
						}
						
						if(number>=0 && number<=63)
						{
							return number;
						}
						else
						{
							return -1;
						}
					}
				}
			}
		}
		
		return -1;
	}
	
	string getSpaceCharacters()
	{
		return " \t";
	}
	bool isMnemonicCharacter(char c)
	{
		return c>='a' && c<='z' || c>='A' && c<='Z';
	}
	bool isNumberStartCharacter(char c)
	{
		return c>='0' && c<='9' || c=='-';
	}
	bool parseNumberHex(const string& str,uint32_t& number)
	{
		if(str.size()==0) return false;
		
		number=0;
		for(size_t digit=0;digit<str.size();digit++)
		{
			number*=0x10;
			if(str[digit]>='0' && str[digit]<='9')
			{
				number+=str[digit]-'0';
			}
			else if(str[digit]>='a' && str[digit]<='f')
			{
				number+=str[digit]-'a'+0xa;
			}
			else if(str[digit]>='A' && str[digit]<='F')
			{
				number+=str[digit]-'A'+0xa;
			}
			else return false;
		}
		
		return true;
	}
	bool parseNumberBin(const string& str,uint32_t& number)
	{
		if(str.size()==0) return false;
		
		number=0;
		for(size_t digit=0;digit<str.size();digit++)
		{
			number<<=1;
			if(str[digit]!='0' && str[digit]!='1') return false;
			number+=str[digit]-'0';
		}
		
		return true;
	}
	bool parseNumberDec(const string& str,uint32_t& number)
	{
		if(str.size()==0) return false;
		
		number=0;
		for(size_t digit=0;digit<str.size();digit++)
		{
			number*=10;
			if(str[digit]<'0' || str[digit]>'9') return false;
			number+=str[digit]-'0';
		}
		
		return true;
	}
	bool parseNumber(const string& inputString,uint32_t& number)
	{
		if(inputString.size()==0) return false;
		
		string str=inputString;
		bool negative=false;
		if(inputString[0]=='-')
		{
			negative=true;
			str=inputString.substr(1,string::npos);
		}
		
		if(str.size()==0) return false;
		if(str[0]=='0' && str.size()>=3)
		{
			if(str[1]=='x')
			{
				if(!parseNumberHex(str.substr(2,string::npos),number)) return false;
				if(negative) number=-number;
				
				return true;
			}
			else if(str[1]=='b')
			{
				if(!parseNumberBin(str.substr(2,string::npos),number)) return false;
				if(negative) number=-number;
				
				return true;
			}
		}
		
		if(!parseNumberDec(str,number)) return false;
		if(negative) number=-number;
		
		return true;
	}
	string parseStringLiteral(const string& code,size_t& codePosition)
	{
		string str;
		
		if(codePosition>code.size()) return str;
		
		char delimiter=code[codePosition];
		
		codePosition++;
		
		bool escaping=false;
		
		while(codePosition<code.size())
		{
			char c=code[codePosition];
			
			if(escaping)
			{
				if(c=='n') str.push_back('\n');
				else if(c=='r') str.push_back('\r');
				else if(c=='t') str.push_back('\t');
				else if(c=='0') str.push_back(0);
				else str.push_back(c);
				
				escaping=false;
				codePosition++;
			}
			else
			{
				if(c==delimiter) break;
				else if(c=='\\')
				{
					escaping=true;
					codePosition++;
				}
				else
				{
					str.push_back(c);
					codePosition++;
				}
			}
		}
		
		return str;
	}
	string pathRemoveUntilBarReverse(const string& path)
	{
		size_t lastBar=path.find_last_of("/");
		
		string basePath;
		if(lastBar!=string::npos)
		{
			basePath=path.substr(0,lastBar+1);
		}
		
		return basePath;
	}
	string formPath(const string& startPath,const string& relativePath)
	{
		string path=pathRemoveUntilBarReverse(startPath);
		
		size_t position=0;
		while(position<relativePath.size())
		{
			size_t nextBar=relativePath.find_first_of("/",position);
			
			if(nextBar==string::npos)
			{
				path+=relativePath.substr(position,string::npos);
				break;
			}
			else
			{
				if(relativePath.substr(position,nextBar+1-position)=="../")
				{
					if(path.size()==0)
					{
						path+="../";
					}
					else
					{
						path.resize(path.size()-1);
						path=pathRemoveUntilBarReverse(path);
					}
					position=nextBar+1;
				}
				else
				{
					path+=relativePath.substr(position,nextBar+1-position);
					position=nextBar+1;
				}
			}
		}
		
		return path;
	}
	AssemblyCode getAssemblyCodeFromFile(const string& filePath)
	{
		AssemblyCode code;
		
		vector<string> lines=fileToLines(filePath);
		
		for(size_t l=0;l<lines.size();l++)
		{
			string line=lines[l];
			size_t firstCharacter=line.find_first_not_of(getSpaceCharacters());
			if(firstCharacter!=string::npos)
			{
				if(line[firstCharacter]!='#')
				{
					if(line[firstCharacter]=='.')
					{
						string includeString="include";
						
						if(line.size()>firstCharacter+1+includeString.size())
						{
							if(line.substr(firstCharacter+1,includeString.size())==includeString)
							{
								size_t nextCharacter=line.find_first_not_of(getSpaceCharacters(),firstCharacter+1+includeString.size());
								
								if(nextCharacter!=string::npos)
								{
									if(line[nextCharacter]=='\"' || line[nextCharacter]=='\'')
									{
										char startCharacter=line[nextCharacter];
										
										string startCharacterStr;
										startCharacterStr.push_back(startCharacter);
										
										size_t endCharacterPosition=line.find(startCharacterStr,nextCharacter+1);
										
										if(endCharacterPosition!=string::npos)
										{
											string toInclude=line.substr(nextCharacter+1,endCharacterPosition-(nextCharacter+1));
											
											string path=formPath(filePath,toInclude);
											
											AssemblyCode codeToInclude=getAssemblyCodeFromFile(path);
											
											code.lines.insert(code.lines.end(),codeToInclude.lines.begin(),codeToInclude.lines.end());
											
											continue;
										}
									}
								}
							}
						}
					}
					
					code.lines.emplace_back(line,LineLocation(l+1,filePath));
				}
			}
		}
		
		return code;
	}
	void assembleLine(Binary& machineCode,vector<LabelDefinition>& labelDefinitions,vector<LabelUse>& labelUses,const AssemblyCodeLine& codeLine,string& lastMainLabel,vector<string>& errors)
	{
		string line=codeLine.codeStr;
		
		size_t firstCharacter=line.find_first_not_of(getSpaceCharacters());
		size_t lastCharacter=line.find_last_not_of(getSpaceCharacters());
		
		if(firstCharacter==string::npos || lastCharacter==string::npos) return;
		if(line[firstCharacter]=='#') return;
		
		if(line[lastCharacter]==':')
		{
			string labelName=line.substr(firstCharacter,lastCharacter-firstCharacter);
			
			bool isMainLabel=true;
			
			if(line[firstCharacter]=='.')
			{
				labelName=lastMainLabel+labelName;
				isMainLabel=false;
			}
			
			for(int i=0;i<labelDefinitions.size();i++)
			{
				if(labelDefinitions[i].labelName==labelName)
				{
					errors.push_back(codeLine.makeErrorMessage("Label already defined."));
					return;
				}
			}
			
			labelDefinitions.emplace_back(labelName,machineCode.size());
			
			if(isMainLabel)
			{
				lastMainLabel=labelName;
			}
		}
		else if(isMnemonicCharacter(line[firstCharacter]))
		{
			size_t mnemonicNameEnd=line.find_first_of(getSpaceCharacters(),firstCharacter);
			
			string mnemonicName;
			if(mnemonicNameEnd==string::npos) mnemonicName=line.substr(firstCharacter,string::npos);
			else mnemonicName=line.substr(firstCharacter,mnemonicNameEnd-firstCharacter);
			
			int mnemonicIndex=-1;
			for(int i=0;i<mnemonics.size();i++)
			{
				if(mnemonics[i].name==mnemonicName)
				{
					mnemonicIndex=i;
					break;
				}
			}
			
			if(mnemonicIndex==-1)
			{
				errors.push_back(codeLine.makeErrorMessage("Instruction name not recognized."));
				return;
			}
			
			vector<InstructionParameter> parameters;
			size_t base=line.find_first_not_of(getSpaceCharacters(),mnemonicNameEnd);
			while(base<line.size())
			{
				size_t parameterEnd=line.find_first_of(",",base);
				if(parameterEnd==string::npos) parameterEnd=line.size();
				
				
				string parameterString=line.substr(base,parameterEnd-base);
				if(parameterString.size()==0)
				{
					errors.push_back(codeLine.makeErrorMessage("Expected instruction parameter before ','."));
					return;
				}
				size_t parameterLastCharacter=parameterString.find_last_not_of(getSpaceCharacters());
				if(parameterLastCharacter==string::npos)
				{
					errors.push_back(codeLine.makeErrorMessage("Unable to parse instruction parameter."));
					return;
				}
				parameterString=parameterString.substr(0,parameterLastCharacter+1);
				
				if(parameterString[0]=='[')
				{
					if(parameterString.back()!=']')
					{
						errors.push_back(codeLine.makeErrorMessage("Expected ']'."));
						return;
					}
					
					size_t registerStart=parameterString.find_first_not_of(getSpaceCharacters(),1);
					if(registerStart+1>=parameterString.size())
					{
						errors.push_back(codeLine.makeErrorMessage("Expected register name inside '[' and ']'."));
						return;
					}
					
					size_t registerEnd=parameterString.find_last_not_of(getSpaceCharacters(),parameterString.size()-2);
					if(registerEnd<registerStart)
					{
						errors.push_back(codeLine.makeErrorMessage("Unable to parse instruction parameter."));
						return;
					}
					
					string registerString=parameterString.substr(registerStart,registerEnd-registerStart+1);
					
					int registerIndex=getRegisterIndex(registerString);
					if(registerIndex==-1)
					{
						errors.push_back(codeLine.makeErrorMessage("Register name not recognized."));
						return;
					}
					
					
					InstructionParameter parameter;
					
					parameter.isImmediate=false;
					parameter.isImmediateLabel=false;
					parameter.isMemory=true;
					
					parameter.registerIndex=registerIndex;
					
					parameters.push_back(parameter);
				}
				else if(isNumberStartCharacter(parameterString[0]))
				{
					uint32_t number=0;
					if(!parseNumber(parameterString,number))
					{
						errors.push_back(codeLine.makeErrorMessage("Error parsing number."));
						return;
					}
					
					InstructionParameter parameter;
					
					parameter.isImmediate=true;
					parameter.isImmediateLabel=false;
					parameter.isMemory=false;
					
					parameter.immediateValue=number;
					
					parameters.push_back(parameter);
				}
				else
				{
					int registerIndex=getRegisterIndex(parameterString);
					if(registerIndex!=-1)
					{
						InstructionParameter parameter;
						
						parameter.isImmediate=false;
						parameter.isImmediateLabel=false;
						parameter.isMemory=false;
						
						parameter.registerIndex=registerIndex;
						
						parameters.push_back(parameter);
					}
					else
					{
						string labelName=parameterString;
						
						if(labelName[0]=='.')
						{
							labelName=lastMainLabel+labelName;
						}
						
						
						InstructionParameter parameter;
						
						parameter.isImmediate=true;
						parameter.isImmediateLabel=true;
						parameter.isMemory=false;
						
						parameter.immediateLabelName=labelName;
						
						parameters.push_back(parameter);
					}
				}
				
				
				if(parameterEnd>=line.size()) break;
				if(parameterEnd+1>=line.size()) break;
				base=line.find_first_not_of(getSpaceCharacters(),parameterEnd+1);
			}
			
			if(parameters.size()!=mnemonics[mnemonicIndex].parameterTypes.size())
			{
				errors.push_back(codeLine.makeErrorMessage("Wrong number of parameters for the instruction."));
				return;
			}
			
			bool immediateMode=false;
			for(int i=0;i<parameters.size();i++)
			{
				if(mnemonics[mnemonicIndex].parameterTypes[i]=='r')
				{
					if(parameters[i].isImmediate)
					{
						errors.push_back(codeLine.makeErrorMessage("Immediate value not available for this parameter for this instruction."));
						return;
					}
				}
				else
				{
					if(parameters[i].isImmediate)
					{
						immediateMode=true;
					}
				}
			}
			
			
			if(immediateMode) machineCode.write<uint8_t>(mnemonics[mnemonicIndex].opcodeWithImmediate);
			else machineCode.write<uint8_t>(mnemonics[mnemonicIndex].opcode);
			
			for(int i=0;i<parameters.size();i++)
			{
				if(parameters[i].isImmediate)
				{
					if(parameters[i].isImmediateLabel)
					{
						labelUses.emplace_back(parameters[i].immediateLabelName,machineCode.size(),codeLine);
						
						machineCode.write<uint32_t>(0);
					}
					else
					{
						machineCode.write<uint32_t>(parameters[i].immediateValue);
					}
				}
				else if(parameters[i].isMemory)
				{
					machineCode.write<uint8_t>(128+parameters[i].registerIndex);
				}
				else
				{
					machineCode.write<uint8_t>(parameters[i].registerIndex);
				}
			}
		}
		else if(line[firstCharacter]=='.')
		{
			size_t directiveEnd=line.find_first_of(getSpaceCharacters(),firstCharacter);
			if(directiveEnd==string::npos) directiveEnd=line.size();
			
			string directiveName=line.substr(firstCharacter,directiveEnd-firstCharacter);
			
			if(directiveName==".binary")
			{
				if(directiveEnd>=line.size())
				{
					errors.push_back(codeLine.makeErrorMessage("Expected file path."));
					return;
				}
				size_t stringStart=line.find_first_not_of(getSpaceCharacters(),directiveEnd);
				if(stringStart==string::npos)
				{
					errors.push_back(codeLine.makeErrorMessage("Expected file path."));
					return;
				}
				if(line[stringStart]!='\'' && line[stringStart]!='\"')
				{
					errors.push_back(codeLine.makeErrorMessage("Expected file path."));
					return;
				}
				
				char startCharacter=line[stringStart];
				
				string startCharacterStr;
				startCharacterStr.push_back(startCharacter);
				
				size_t endCharacterPosition=line.find(startCharacterStr,stringStart+1);
				
				if(endCharacterPosition==string::npos)
				{
					errors.push_back(codeLine.makeErrorMessage("Expected string literal terminator."));
					return;
				}
				string toInclude=line.substr(stringStart+1,endCharacterPosition-(stringStart+1));
				
				string path=formPath(codeLine.location.fileName,toInclude);
				
				Binary binary;
				try
				{
					binary.loadFromFileRaw(path);
				}
				catch(...)
				{
					errors.push_back(codeLine.makeErrorMessage("File not found."));
					return;
				}
				
				machineCode.content.insert(machineCode.content.end(),binary.content.begin(),binary.content.end());
			}
			else
			{
				errors.push_back(codeLine.makeErrorMessage("Directive not recognized."));
				return;
			}
		}
		else
		{
			size_t base=firstCharacter;
			while(true)
			{
				size_t nextBase=base+1;
				if(isNumberStartCharacter(line[base]))
				{
					size_t numberEnd=line.find_first_of(getSpaceCharacters(),base);
					string numberStr;
					if(numberEnd==string::npos) numberStr=line.substr(base,string::npos);
					else numberStr=line.substr(base,numberEnd-base);
					
					uint32_t number=0;
					if(!parseNumber(numberStr,number))
					{
						errors.push_back(codeLine.makeErrorMessage("Error parsing number."));
						return;
					}
					
					machineCode.write<uint32_t>(number);
					
					if(numberEnd==string::npos) break;
					else
					{
						nextBase=line.find_first_not_of(getSpaceCharacters(),numberEnd);
						if(nextBase==string::npos) break;
					}
				}
				else if(line[base]=='\"' || line[base]=='\'')
				{
					size_t stringPosition=base;
					string str=parseStringLiteral(line,stringPosition);
					
					for(size_t i=0;i<str.size();i++)
					{
						machineCode.write<uint8_t>(str[i]);
					}
					
					if(stringPosition>line.size()) break;
					
					size_t stringEnd=stringPosition+1;
					
					if(stringEnd>line.size()) break;
					
					nextBase=line.find_first_not_of(getSpaceCharacters(),stringEnd);
					if(nextBase==string::npos) break;
				}
				else
				{
					errors.push_back(codeLine.makeErrorMessage("Unexpected character."));
					return;
				}
				base=nextBase;
			}
		}
	}
	void assemble(const string& inputFilePath,const string& outputFilePath,vector<string>& errors)
	{
		AssemblyCode code=getAssemblyCodeFromFile(inputFilePath);
		
		Binary machineCode;
		vector<LabelDefinition> labelDefinitions;
		vector<LabelUse> labelUses;
		
		string lastMainLabel;
		for(size_t l=0;l<code.lines.size();l++)
		{
			assembleLine(machineCode,labelDefinitions,labelUses,code.lines[l],lastMainLabel,errors);
		}
		
		if(errors.size()>0) return;
		
		for(size_t i=0;i<labelUses.size();i++)
		{
			bool found=false;
			for(size_t j=0;j<labelDefinitions.size();j++)
			{
				if(labelDefinitions[j].labelName==labelUses[i].labelName)
				{
					uint32_t value=labelDefinitions[j].positionInMachineCode;
					
					size_t position=labelUses[i].positionInMachineCode;
					
					Binary binary;
					binary.write<uint32_t>(value);
					for(size_t k=0;k<binary.size();k++)
					{
						machineCode.content[position+k]=binary.content[k];
					}
					
					found=true;
					break;
				}
			}
			if(!found)
			{
				errors.push_back(labelUses[i].codeLine.makeErrorMessage("Label definition not found."));
			}
		}
		
		if(errors.size()>0) return;
		
		machineCode.saveToFileRaw(outputFilePath);
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
	
	if(args.size()!=2)
	{
		cout<<"Wrong number of parameters. Use the format: 'assemble code.asm code.bin'."<<endl;
		return 1;
	}
	else
	{
		Assembler assembler;
		
		vector<string> errors;
		assembler.assemble(args[0],args[1],errors);
		
		if(errors.size()>0)
		{
			cout<<"Errors: "<<errors.size()<<endl;
			size_t maxErrorsToShow=1000;
			if(errors.size()>maxErrorsToShow) cout<<"Showing first "<<maxErrorsToShow<<" errors."<<endl;
			for(size_t i=0;i<errors.size() && i<maxErrorsToShow;i++)
			{
				cout<<errors[i]<<endl;
			}
			return 1;
		}
	}
	}catch(const string& str)
	{
		cout<<str<<endl;
		return 1;
	}
	
	return 0;
}

