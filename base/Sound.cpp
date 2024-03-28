#define AUDIO_SAMPLE_RATE 48000

typedef struct __attribute__((packed))
{
	int ChunkID;
	int ChunkSize;
	int format;
	int Subchunk1ID;
	int Subchunk1Size;
	short int AudioFormat;
	short int NumChannels;
	int SampleRate;
	int ByteRate;
	short int BlockAlign;
	short int BitsPerSample;
}WAV_file_header;

class Sound
{
	public:
	vector<float> channel1;
	vector<float> channel2;
	float duration()
	{
		return float(channel1.size())/AUDIO_SAMPLE_RATE;
	}
	Sound(){}
	explicit Sound(const string& filepath)
	{
		if(!load(filepath)) throw string("Could not load audio file");
	}
	explicit Sound(const vector<float>& mono)
	{
		channel1=mono;
		channel2=mono;
	}
	vector<float> getMono()
	{
		vector<float> mono(channel1.size());
		for(int n=0;n<mono.size();n++)
		{
			mono[n]=(channel1[n]+channel2[n])/2;
		}
		return mono;
	}
	int save(const string& filepath)
	{
		if(channel1.size()!=channel2.size()) return 0;
		
		unsigned int size_of_data=2*2*channel1.size();
		vector<unsigned char> data(size_of_data);
		
		for(unsigned int n=0;n<channel1.size();n++)
		{
			*(short int*)&data[n*4]=channel1[n]>1 ? 1 : (channel1[n]<-1 ? -1 : (short int)(channel1[n]*0x7fff));
			*(short int*)&data[n*4+2]=channel2[n]>1 ? 1 : (channel2[n]<-1 ? -1 : (short int)(channel2[n]*0x7fff));
		}
		
		WAV_file_header wav_file_header;
		wav_file_header.ChunkID=0x46464952;
		wav_file_header.ChunkSize=size_of_data+36;
		wav_file_header.format=0x45564157;
		wav_file_header.Subchunk1ID=0x20746d66;
		wav_file_header.Subchunk1Size=16;
		wav_file_header.AudioFormat=1;
		wav_file_header.NumChannels=2;
		wav_file_header.SampleRate=AUDIO_SAMPLE_RATE;
		wav_file_header.ByteRate=AUDIO_SAMPLE_RATE*2*2;
		wav_file_header.BlockAlign=2*2;
		wav_file_header.BitsPerSample=16;
		
		ofstream file;
		file.open(filepath.c_str(),std::ios::binary);
		if(!file.is_open()) return 0;
		
		file.write((char*)&wav_file_header,sizeof(WAV_file_header));
		
		file.write("data",4);
		
		file.write((char*)&size_of_data,4);
		
		file.write((char*)data.data(),data.size());
		
		file.close();
		
		return 1;
	}
	int load(const string& filepath)
	{
		ifstream file;
		file.open(filepath.c_str(),std::ios::binary);
		if(!file.good()) return 0;
		
		WAV_file_header wav_file_header;
		file.read((char*)&wav_file_header,sizeof(WAV_file_header));
		
		if(wav_file_header.ChunkID!=0x46464952) return 0;
		if(wav_file_header.format!=0x45564157) return 0;
		if(wav_file_header.Subchunk1ID!=0x20746d66) return 0;
		if(wav_file_header.Subchunk1Size!=16) return 0;
		if(wav_file_header.AudioFormat!=1) return 0;
		
		int num_channels=wav_file_header.NumChannels;
		
		if(wav_file_header.SampleRate!=AUDIO_SAMPLE_RATE) return 0;
		int bytes_per_sample=wav_file_header.BitsPerSample/8;
		if(bytes_per_sample!=2) return 0;
		
		char str[4]={0,0,0,0};
		for(int n=0;;n++)
		{
			if(n==1000) return 0;
			char c;
			file.read(&c,1);
			for(int i=0;i<3;i++)
			{
				str[i]=str[i+1];
			}
			str[3]=c;
			if(str[0]=='d' && str[1]=='a' && str[2]=='t' && str[3]=='a') break;
		}
		int size_of_data;
		file.read((char*)&size_of_data,4);
		
		vector<unsigned char> data(size_of_data);
		file.read((char*)data.data(),data.size());
		
		file.close();
		
		if(num_channels==1)
		{
			channel1.resize(data.size()/2);
			channel2.resize(data.size()/2);
			for(int n=0;n<data.size();n+=2)
			{
				channel1[n/2]=float(int(*((short int*)&data[n])))/0x8000;
				channel2[n/2]=channel1[n/2];
			}
		}
		else
		{
			channel1.resize(data.size()/4);
			channel2.resize(data.size()/4);
			for(int n=0;n<data.size();n+=4)
			{
				channel1[n/4]=float(int(*((short int*)&data[n])))/0x8000;
				channel2[n/4]=float(int(*((short int*)&data[n+2])))/0x8000;
			}
		}
		
		return 1;
	}
};
